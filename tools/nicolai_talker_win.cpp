// Standalone Windows test UI; all synthesis jobs run in an owned child process.
#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>
#include <shellapi.h>
#include <mmsystem.h>
#include <sapi.h>
#include "talker_backend.hpp"
#include "nicolai/wav_writer.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <cwctype>

namespace {
namespace fs = std::filesystem;
using nicolai::test_talker::Profile;
template<class T> struct ComPtr {
    T* p = nullptr;
    ~ComPtr() { if (p) p->Release(); }
    T* operator->() const { return p; }
};
void check(HRESULT hr, const char* stage) {
    if (FAILED(hr)) {
        std::ostringstream error;
        error << stage << " HRESULT=0x" << std::hex << static_cast<unsigned long>(hr);
        throw std::runtime_error(error.str());
    }
}
void original_stage(const char* stage) {
    std::cout << "original_sapi_stage=" << stage << '\n' << std::flush;
}
void render_original(const fs::path& text_file, const fs::path& output);
constexpr int kText = 101, kVoice = 102, kProfile = 103, kSpeak = 104,
    kStop = 105, kReplay = 106, kSave = 107, kBrowse = 108, kLogs = 109;
struct App {
    HWND window{}, text{}, voice{}, profile{}, speak{}, stop{}, replay{}, save{}, browse{}, logs{}, status{};
    HFONT font{};
    HANDLE child{};
    ULONGLONG started{};
    fs::path job, wav;
    bool smoke = false;
};

std::string utf8(const std::wstring& input) {
    if (input.empty()) return {};
    const int length = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
        input.data(), static_cast<int>(input.size()), nullptr, 0, nullptr, nullptr);
    if (!length) throw std::runtime_error("invalid UTF-16 input");
    std::string result(length, '\0');
    WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, input.data(),
        static_cast<int>(input.size()), result.data(), length, nullptr, nullptr);
    return result;
}
std::wstring wide(const std::string& input) {
    if (input.empty()) return {};
    const int length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
        input.data(), static_cast<int>(input.size()), nullptr, 0);
    if (!length) return L"Ошибка чтения UTF-8. Подробности в render.log.";
    std::wstring result(length, L'\0');
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, input.data(),
        static_cast<int>(input.size()), result.data(), length);
    return result;
}
std::wstring text_of(HWND control) {
    const int length = GetWindowTextLengthW(control);
    std::wstring result(length + 1, L'\0');
    GetWindowTextW(control, result.data(), length + 1);
    result.resize(length);
    return result;
}
fs::path executable() {
    std::vector<wchar_t> buffer(32768);
    const DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (!length || length >= buffer.size()) throw std::runtime_error("cannot resolve executable path");
    return fs::path(std::wstring(buffer.data(), length));
}
// CRT command-line quoting, including trailing slashes before a closing quote.
std::wstring quote(const std::wstring& value) {
    std::wstring result = L"\"";
    std::size_t slashes = 0;
    for (wchar_t c : value) {
        if (c == L'\\') { ++slashes; continue; }
        result.append(c == L'"' ? slashes * 2 + 1 : slashes, L'\\');
        slashes = 0;
        result += c;
    }
    result.append(slashes * 2, L'\\');
    return result + L'"';
}
std::string read_utf8(const fs::path& file) {
    std::ifstream input(file, std::ios::binary);
    if (!input) throw std::runtime_error("cannot read text/log file");
    std::string result{std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
    if (result.size() >= 3 && result.compare(0, 3, "\xef\xbb\xbf") == 0) result.erase(0, 3);
    return result;
}
fs::path local_jobs() {
    PWSTR directory = nullptr;
    const HRESULT hr = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &directory);
    if (FAILED(hr)) throw std::runtime_error("cannot resolve LocalAppData");
    fs::path result = fs::path(directory) / "NicolaiNativePort" / "TestRuns";
    CoTaskMemFree(directory);
    fs::create_directories(result);
    return result;
}
fs::path default_voice() {
    const auto parent = executable().parent_path();
    std::vector<fs::path> candidates{parent / "Elan", parent.parent_path() / "Elan"};
    wchar_t installed[32768]{};
    const DWORD n = GetEnvironmentVariableW(L"ProgramFiles(x86)", installed, 32768);
    if (n && n < 32768) candidates.push_back(fs::path(installed) / "Elan");
    for (const auto& candidate : candidates) {
        try { nicolai::test_talker::validate_voice_directory(candidate); return candidate; }
        catch (...) {}
    }
    return {};
}
void status(App& app, const std::wstring& message) { SetWindowTextW(app.status, message.c_str()); }
void controls(App& app, bool busy) {
    for (HWND control : {app.speak, app.text, app.voice, app.profile, app.browse}) EnableWindow(control, !busy);
    EnableWindow(app.stop, busy || !app.wav.empty());
    EnableWindow(app.replay, !busy && !app.wav.empty());
    EnableWindow(app.save, !busy && !app.wav.empty());
    EnableWindow(app.logs, !app.job.empty());
}
void stop_job(App& app) {
    PlaySoundW(nullptr, nullptr, 0);
    if (app.child) {
        TerminateProcess(app.child, ERROR_CANCELLED); // Only our own render job.
        WaitForSingleObject(app.child, 5000);
        CloseHandle(app.child);
        app.child = nullptr;
        KillTimer(app.window, 1);
    }
    controls(app, false);
}
void play(App& app) {
    if (!PlaySoundW(app.wav.c_str(), nullptr, SND_FILENAME | SND_ASYNC | SND_NODEFAULT))
        status(app, L"Не удалось воспроизвести WAV. Его можно сохранить и открыть вручную.");
}
void start_job(App& app) {
    const auto text = text_of(app.text);
    if (text.find_first_not_of(L" \t\r\n") == std::wstring::npos)
        throw std::runtime_error("Введите текст для произнесения.");
    const fs::path voice(text_of(app.voice));
    const LRESULT selected = SendMessageW(app.profile, CB_GETCURSEL, 0, 0);
    const wchar_t* profiles[]{L"stable", L"m36-local", L"m36-chain", L"m38-boundary", L"m40-transient", L"m41-preserve", L"m42-join-pitch", L"original-sapi"};
    if (selected < 0 || selected > 7) throw std::runtime_error("Выберите движок и акустику.");
    if (selected != 7) nicolai::test_talker::validate_voice_directory(voice);
    stop_job(app);
    app.wav.clear();
    const auto root = local_jobs();
    const auto stamp = std::to_wstring(GetCurrentProcessId()) + L"-" + std::to_wstring(GetTickCount64());
    app.job = root / stamp;
    if (!fs::create_directory(app.job)) throw std::runtime_error("cannot create fresh job directory");
    const auto input = app.job / "input.txt";
    std::ofstream file(input, std::ios::binary);
    file << utf8(text);
    file.close();
    if (!file) throw std::runtime_error("cannot write input.txt");
    const auto output = app.job / "result.wav";
    SECURITY_ATTRIBUTES security{sizeof(security), nullptr, TRUE};
    HANDLE log = CreateFileW((app.job / "render.log").c_str(), GENERIC_WRITE,
        FILE_SHARE_READ, &security, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (log == INVALID_HANDLE_VALUE) throw std::runtime_error("cannot create render.log");
    const auto exe = executable();
    std::wstring command = quote(exe.wstring()) + L" --render " + quote(voice.wstring()) +
        L" " + quote(input.wstring()) + L" " + profiles[selected] + L" " + quote(output.wstring());
    STARTUPINFOW startup{}; startup.cb = sizeof(startup);
    startup.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    startup.wShowWindow = SW_HIDE;
    startup.hStdOutput = log; startup.hStdError = log;
    HANDLE null_input = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
        &security, OPEN_EXISTING, 0, nullptr);
    startup.hStdInput = null_input;
    PROCESS_INFORMATION process{};
    const BOOL ok = CreateProcessW(exe.c_str(), command.data(), nullptr, nullptr, TRUE,
        CREATE_NO_WINDOW, nullptr, exe.parent_path().c_str(), &startup, &process);
    const DWORD create_error = ok ? ERROR_SUCCESS : GetLastError();
    CloseHandle(log);
    if (null_input != INVALID_HANDLE_VALUE) CloseHandle(null_input);
    if (!ok) throw std::runtime_error("cannot start owned render process: " + std::to_string(create_error));
    CloseHandle(process.hThread);
    app.child = process.hProcess;
    app.started = GetTickCount64();
    SetTimer(app.window, 1, 100, nullptr);
    controls(app, true);
    status(app, L"Синтезирую… Можно отменить кнопкой «Стоп».");
}
void poll(App& app) {
    if (!app.child) return;
    DWORD code = STILL_ACTIVE;
    if (!GetExitCodeProcess(app.child, &code)) throw std::runtime_error("cannot inspect render process");
    if (code == STILL_ACTIVE) {
        if (GetTickCount64() - app.started > 60000) {
            stop_job(app);
            status(app, L"Синтез отменён по тайм-ауту 60 секунд. Подробности в папке результата.");
        }
        return;
    }
    CloseHandle(app.child); app.child = nullptr;
    KillTimer(app.window, 1);
    const auto output = app.job / "result.wav";
    if (code == 0 && fs::is_regular_file(output) && fs::file_size(output) > 44) {
        app.wav = output;
        controls(app, false);
        status(app, L"Готово. Можно повторить, сохранить WAV или сменить акустику и сравнить.");
        if (!app.smoke) play(app);
    } else {
        controls(app, false);
        status(app, L"Ошибка синтеза. Подробности в render.log.");
        const auto details = read_utf8(app.job / "render.log");
        if (!app.smoke) MessageBoxW(app.window, wide(details.substr(0, 4000)).c_str(), L"Nicolai: ошибка синтеза", MB_OK | MB_ICONERROR);
    }
}
void browse(App& app) {
    BROWSEINFOW info{};
    info.hwndOwner = app.window;
    info.lpszTitle = L"Выбери папку с nicolai16.dat, exc_rus.txt и abb_rus.txt";
    info.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    PIDLIST_ABSOLUTE item = SHBrowseForFolderW(&info);
    if (!item) return;
    wchar_t directory[MAX_PATH]{};
    if (SHGetPathFromIDListW(item, directory)) SetWindowTextW(app.voice, directory);
    CoTaskMemFree(item);
}
void save(App& app) {
    wchar_t destination[32768]{};
    const wchar_t* suggested[]{L"nicolai-stable.wav", L"nicolai-m36-local.wav",
                               L"nicolai-m36-chain.wav", L"nicolai-m38-boundary.wav",
                               L"nicolai-m40-transient.wav", L"nicolai-m41-preserve.wav", L"nicolai-m42-join-pitch.wav", L"nicolai-original-sapi.wav"};
    const LRESULT selected = SendMessageW(app.profile, CB_GETCURSEL, 0, 0);
    if (selected < 0 || selected > 7) throw std::runtime_error("Choose a synthesis profile before saving.");
    const std::wstring filename(suggested[selected]);
    std::copy(filename.begin(), filename.end(), destination);
    OPENFILENAMEW dialog{}; dialog.lStructSize = sizeof(dialog);
    dialog.hwndOwner = app.window;
    dialog.lpstrFilter = L"WAV audio\0*.wav\0\0";
    dialog.lpstrFile = destination;
    dialog.nMaxFile = 32768;
    dialog.lpstrDefExt = L"wav";
    dialog.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    if (GetSaveFileNameW(&dialog) && !CopyFileW(app.wav.c_str(), destination, FALSE))
        throw std::runtime_error("cannot save WAV to selected path");
}
HWND control(App& app, const wchar_t* type, const wchar_t* label, DWORD style, int id) {
    HWND window = CreateWindowExW(type == std::wstring(L"EDIT") ? WS_EX_CLIENTEDGE : 0,
        type, label, WS_CHILD | WS_VISIBLE | style, 0, 0, 10, 10, app.window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), GetModuleHandleW(nullptr), nullptr);
    if (!window) throw std::runtime_error("cannot create GUI control");
    SendMessageW(window, WM_SETFONT, reinterpret_cast<WPARAM>(app.font), TRUE);
    return window;
}
void layout(App& app) {
    RECT rect{}; GetClientRect(app.window, &rect);
    const int width = rect.right, height = rect.bottom;
    auto move = [](HWND c, int x, int y, int w, int h) { MoveWindow(c, x, y, w, h, TRUE); };
    move(GetDlgItem(app.window, 201), 18, 15, 700, 24);
    move(app.voice, 18, 42, width - 146, 28);
    move(app.browse, width - 116, 41, 98, 30);
    move(GetDlgItem(app.window, 202), 18, 84, 120, 24);
    move(app.profile, 140, 80, width - 158, 160);
    move(GetDlgItem(app.window, 203), 18, 119, width - 36, 38);
    move(app.text, 18, 166, width - 36, height - 288);
    const int y = height - 108;
    move(app.speak, 18, y, 140, 34);
    move(app.stop, 168, y, 78, 34);
    move(app.replay, 256, y, 96, 34);
    move(app.save, 362, y, 138, 34);
    move(app.logs, 510, y, 160, 34);
    move(app.status, 18, height - 59, width - 36, 43);
}
void snapshot(HWND window, const fs::path& output) {
    if (fs::exists(output)) throw std::runtime_error("snapshot output must be fresh");
    ShowWindow(window, SW_SHOWNOACTIVATE); UpdateWindow(window);
    RECT rect{}; GetWindowRect(window, &rect);
    const int width = rect.right - rect.left, height = rect.bottom - rect.top;
    HDC screen = GetDC(window), memory = CreateCompatibleDC(screen);
    HBITMAP bitmap = CreateCompatibleBitmap(screen, width, height);
    HGDIOBJ previous = SelectObject(memory, bitmap);
    const BOOL printed = PrintWindow(window, memory, 0);
    SelectObject(memory, previous);
    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = width; info.bmiHeader.biHeight = height;
    info.bmiHeader.biPlanes = 1; info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;
    std::vector<BYTE> pixels(static_cast<std::size_t>(width) * height * 4);
    const int lines = GetDIBits(memory, bitmap, 0, height, pixels.data(), &info, DIB_RGB_COLORS);
    DeleteObject(bitmap); DeleteDC(memory); ReleaseDC(window, screen);
    if (!printed || lines != height) throw std::runtime_error("own-window snapshot failed");
    BITMAPFILEHEADER file{}; file.bfType = 0x4d42;
    file.bfOffBits = sizeof(file) + sizeof(info.bmiHeader);
    file.bfSize = file.bfOffBits + static_cast<DWORD>(pixels.size());
    std::ofstream stream(output, std::ios::binary);
    stream.write(reinterpret_cast<const char*>(&file), sizeof(file));
    stream.write(reinterpret_cast<const char*>(&info.bmiHeader), sizeof(info.bmiHeader));
    stream.write(reinterpret_cast<const char*>(pixels.data()), pixels.size());
    if (!stream) throw std::runtime_error("own-window snapshot write failed");
}
LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    auto* app = reinterpret_cast<App*>(GetWindowLongPtrW(window, GWLP_USERDATA));
    if (message == WM_NCCREATE) {
        app = static_cast<App*>(reinterpret_cast<CREATESTRUCTW*>(lparam)->lpCreateParams);
        app->window = window;
        SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
    }
    if (!app) return DefWindowProcW(window, message, wparam, lparam);
    try {
        switch (message) {
        case WM_CREATE:
            app->font = CreateFontW(-16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                DEFAULT_PITCH, L"Segoe UI");
            control(*app, L"STATIC", L"Данные голоса (папка Elan):", 0, 201);
            app->voice = control(*app, L"EDIT", default_voice().c_str(), ES_AUTOHSCROLL | WS_TABSTOP, kVoice);
            app->browse = control(*app, L"BUTTON", L"Папка…", WS_TABSTOP, kBrowse);
            control(*app, L"STATIC", L"Движок:", 0, 202);
            app->profile = control(*app, L"COMBOBOX", L"", CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP, kProfile);
            for (const auto* name : {L"Порт — обычная акустика", L"Порт — M36 local (эксперимент)", L"Порт — M36 chain (эксперимент)", L"Порт — M38 связная речь (эксперимент)", L"Порт — M40 меньше щелчков (эксперимент)", L"Порт — M41 сохранение звуков (эксперимент)", L"Порт — M42 тон на стыках (эксперимент)", L"Оригинал ПК — Nicolai через SAPI"})
                SendMessageW(app->profile, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(name));
            SendMessageW(app->profile, CB_SETCURSEL, 0, 0);
            control(*app, L"STATIC", L"Оригинал требует 32-битный SAPI-голос Elan. M36/M38/M40/M41/M42 — эксперименты.", 0, 203);
            app->text = control(*app, L"EDIT", L"Привет! Это Николай. Проверяем голос и акустику.",
                ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN | WS_VSCROLL | WS_TABSTOP, kText);
            SendMessageW(app->text, EM_SETLIMITTEXT, 16000, 0);
            app->speak = control(*app, L"BUTTON", L"Произнести", BS_DEFPUSHBUTTON | WS_TABSTOP, kSpeak);
            app->stop = control(*app, L"BUTTON", L"Стоп", WS_TABSTOP, kStop);
            app->replay = control(*app, L"BUTTON", L"Повторить", WS_TABSTOP, kReplay);
            app->save = control(*app, L"BUTTON", L"Сохранить WAV", WS_TABSTOP, kSave);
            app->logs = control(*app, L"BUTTON", L"Папка результата", WS_TABSTOP, kLogs);
            app->status = control(*app, L"STATIC", L"Введи текст и нажми «Произнести».", 0, 204);
            controls(*app, false); layout(*app);
            return 0;
        case WM_SIZE: if (app->text) layout(*app); return 0;
        case WM_GETMINMAXINFO:
            reinterpret_cast<MINMAXINFO*>(lparam)->ptMinTrackSize = {740, 450}; return 0;
        case WM_TIMER: poll(*app); return 0;
        case WM_COMMAND:
            if (LOWORD(wparam) == kProfile && HIWORD(wparam) == CBN_SELCHANGE) {
                // A selected engine is not proof that the previous WAV came
                // from it. Keep the file in TestRuns but require a fresh job.
                PlaySoundW(nullptr, nullptr, 0);
                app->wav.clear();
                controls(*app, false);
                status(*app, L"Режим изменён. Нажми «Произнести», чтобы получить новый WAV.");
                return 0;
            }
            if (HIWORD(wparam) != BN_CLICKED) break;
            switch (LOWORD(wparam)) {
            case kSpeak: if (!app->child) start_job(*app); break;
            case kStop: stop_job(*app); status(*app, L"Остановлено."); break;
            case kReplay: if (!app->wav.empty()) play(*app); break;
            case kSave: if (!app->wav.empty()) save(*app); break;
            case kBrowse: browse(*app); break;
            case kLogs: if (!app->job.empty()) ShellExecuteW(window, L"open", app->job.c_str(), nullptr, nullptr, SW_SHOWNORMAL); break;
            }
            return 0;
        case WM_CLOSE: stop_job(*app); DestroyWindow(window); return 0;
        case WM_DESTROY:
            if (app->font) DeleteObject(app->font);
            PostQuitMessage(0); return 0;
        }
    } catch (const std::exception& error) {
        if (message == WM_CREATE) return -1;
        if (message == WM_TIMER) stop_job(*app);
        controls(*app, app->child != nullptr);
        if (!app->smoke) MessageBoxW(window, wide(error.what()).c_str(), L"Nicolai: ошибка", MB_OK | MB_ICONERROR);
    }
    return DefWindowProcW(window, message, wparam, lparam);
}
int ui(HINSTANCE instance, bool smoke, const fs::path& test_voice = {}, Profile test_profile = Profile::Stable, const fs::path& screenshot = {}) {
    App app; app.smoke = smoke;
    WNDCLASSW type{}; type.lpfnWndProc = window_proc; type.hInstance = instance;
    type.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    type.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    type.lpszClassName = L"NicolaiNativePortTestTalker";
    if (!RegisterClassW(&type)) return 1;
    HWND window = CreateWindowExW(0, type.lpszClassName, L"Николай — оригинал ПК и порт",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 920, 610,
        nullptr, nullptr, instance, &app);
    if (!window) return 1;
    if (smoke) {
        if (!screenshot.empty()) snapshot(window, screenshot);
        if (!test_voice.empty()) {
            SetWindowTextW(app.voice, test_voice.c_str());
            // Exercise the actual startup text; replacing it with an easier
            // phrase previously concealed a word-initial allophone failure.
            SendMessageW(app.profile, CB_SETCURSEL, static_cast<int>(test_profile), 0);
            try { start_job(app); } catch (...) { DestroyWindow(window); return 1; }
            const ULONGLONG deadline = GetTickCount64() + 65000;
            while (app.child && GetTickCount64() < deadline) {
                MSG message{};
                while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
                    TranslateMessage(&message); DispatchMessageW(&message);
                }
                Sleep(20);
            }
            const bool rendered = !app.child && !app.wav.empty() &&
                IsWindowEnabled(app.save) && IsWindowEnabled(app.replay) && IsWindowEnabled(app.speak);
            SendMessageW(app.profile, CB_SETCURSEL, (static_cast<int>(test_profile) + 1) % 8, 0);
            SendMessageW(window, WM_COMMAND, MAKEWPARAM(kProfile, CBN_SELCHANGE),
                         reinterpret_cast<LPARAM>(app.profile));
            const bool reset = app.wav.empty() && !IsWindowEnabled(app.save) &&
                !IsWindowEnabled(app.replay) && IsWindowEnabled(app.speak);
            stop_job(app); DestroyWindow(window);
            return rendered && reset ? 0 : 1;
        }
        const std::wstring sample = L"Проверка: ёжик, кавычки «текст» и путь C:\\Голос\\";
        SetWindowTextW(app.text, sample.c_str());
        const bool ok = text_of(app.text) == sample &&
            wide(utf8(sample)) == sample && SendMessageW(app.profile, CB_GETCOUNT, 0, 0) == 8 &&
            SendMessageW(app.profile, CB_GETCURSEL, 0, 0) == 0 &&
            !IsWindowEnabled(app.save) && !IsWindowEnabled(app.stop) && IsWindowEnabled(app.speak);
        DestroyWindow(window);
        return ok ? 0 : 1;
    }
    ShowWindow(window, SW_SHOWNORMAL); UpdateWindow(window);
    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        if (!IsDialogMessageW(window, &message)) {
            TranslateMessage(&message); DispatchMessageW(&message);
        }
    }
    return static_cast<int>(message.wParam);
}
int render_job(int count, wchar_t** arguments) {
    if (count != 6) throw std::runtime_error("usage: --render <voice-directory> <UTF-8-text-file> <stable|m36-local|m36-chain|m38-boundary|m40-transient|m41-preserve|m42-join-pitch|original-sapi> <fresh-output.wav>");
    const fs::path output(arguments[5]);
    if (fs::exists(output)) throw std::runtime_error("output WAV must be fresh");
    if (std::wstring(arguments[4]) == L"original-sapi") {
        render_original(arguments[3], output);
        return 0;
    }
    const auto profile = nicolai::test_talker::parse_profile(utf8(arguments[4]));
    const auto result = nicolai::test_talker::render(arguments[2], read_utf8(arguments[3]), profile);
    nicolai::write_wav_pcm16_mono(output, result.pcm);
    if (!fs::is_regular_file(output) || fs::file_size(output) != 44 + result.pcm.samples.size() * 2)
        throw std::runtime_error("WAV write incomplete");
    std::cout << "profile=" << nicolai::test_talker::profile_name(profile)
        << "\nsamples=" << result.pcm.samples.size() << "\nrate=" << result.pcm.sample_rate
        << "\ninitial=" << result.m36_initial_paths << "\ncross=" << result.m36_cross_paths
        << "\nterminal_flushes=" << result.m36_terminal_flushes
        << "\nfallbacks=" << result.m36_fallbacks
        << "\nprotected_internal_m41=" << result.protected_internal_m41
        << "\nprotected_external_m41=" << result.protected_external_m41
        << "\nreconciled_pitch_joins_m42=" << result.reconciled_pitch_joins_m42 << '\n';
    for (const auto& join : result.joins)
        std::cout << "join=" << join.shared_phone_index << ',' << join.shared_phone
                  << ',' << join.center_sample
                  << ',' << join.overlap_samples << ',' << join.left_trim
                  << ',' << join.right_trim << ',' << join.normalized_correlation << '\n';
    return 0;
}
void render_original(const fs::path& text_file, const fs::path& output) {
    if (sizeof(void*) != 4) throw std::runtime_error("Original Nicolai requires the 32-bit test executable.");
    const auto text = read_utf8(text_file);
    if (text.empty() || text.size() > 64000) throw std::runtime_error("text must contain 1..64000 UTF-8 bytes");
    const int length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(), static_cast<int>(text.size()), nullptr, 0);
    if (!length) throw std::runtime_error("invalid UTF-8 text");
    const auto speech = wide(text);
    check(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED), "CoInitializeEx");
    struct Guard { ~Guard() { CoUninitialize(); } } guard;
    original_stage("enumerate-voices");
    ComPtr<ISpObjectTokenCategory> category;
    check(CoCreateInstance(__uuidof(SpObjectTokenCategory), nullptr, CLSCTX_INPROC_SERVER,
        __uuidof(ISpObjectTokenCategory), reinterpret_cast<void**>(&category.p)), "voice category");
    check(category->SetId(SPCAT_VOICES, FALSE), "voice category SetId");
    ComPtr<IEnumSpObjectTokens> tokens;
    check(category->EnumTokens(nullptr, nullptr, &tokens.p), "EnumTokens");
    ComPtr<ISpObjectToken> selected;
    ULONG count = 0;
    check(tokens->GetCount(&count), "voice count");
    for (ULONG index = 0; index < count; ++index) {
        ComPtr<ISpObjectToken> token;
        check(tokens->Item(index, &token.p), "voice token");
        wchar_t* description = nullptr;
        if (SUCCEEDED(token->GetStringValue(nullptr, &description))) {
            std::wstring name = description;
            CoTaskMemFree(description);
            std::transform(name.begin(), name.end(), name.begin(), [](wchar_t c) { return std::towlower(c); });
            if (name.find(L"nicolai") != std::wstring::npos || name.find(L"elan tts russian") != std::wstring::npos) {
                selected.p = token.p; token.p = nullptr; break;
            }
        }
    }
    if (!selected.p) throw std::runtime_error("Original Nicolai was not found in 32-bit SAPI. Install the original Elan voice; the port modes do not need it.");
    ComPtr<ISpVoice> voice;
    check(CoCreateInstance(__uuidof(SpVoice), nullptr, CLSCTX_INPROC_SERVER,
        __uuidof(ISpVoice), reinterpret_cast<void**>(&voice.p)), "SpVoice");
    original_stage("select-voice");
    check(voice->SetVoice(selected.p), "SetVoice");
    original_stage("set-rate");
    check(voice->SetRate(0), "SetRate");
    check(voice->SetVolume(100), "SetVolume");
    ComPtr<ISpStream> stream;
    check(CoCreateInstance(__uuidof(SpStream), nullptr, CLSCTX_INPROC_SERVER,
        __uuidof(ISpStream), reinterpret_cast<void**>(&stream.p)), "SpStream");
    const WAVEFORMATEX format{WAVE_FORMAT_PCM, 1, 16000, 32000, 2, 16, 0};
    check(stream->BindToFile(output.c_str(), SPFM_CREATE_ALWAYS, &SPDFID_WaveFormatEx, &format, 0), "BindToFile");
    check(voice->SetOutput(stream.p, TRUE), "SetOutput");
    original_stage("speak");
    check(voice->Speak(speech.c_str(), SPF_IS_NOT_XML, nullptr), "Speak");
    check(stream->Close(), "Close stream");
    if (!fs::exists(output) || fs::file_size(output) <= 44) throw std::runtime_error("Original SAPI produced no audio");
    original_stage("complete");
}
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int) {
    int count = 0;
    wchar_t** arguments = CommandLineToArgvW(GetCommandLineW(), &count);
    if (!arguments) return 1;
    struct ArgGuard { wchar_t** p; ~ArgGuard() { LocalFree(p); } } guard{arguments};
    try {
        if (count > 1 && std::wstring(arguments[1]) == L"--render") return render_job(count, arguments);
        const bool smoke = (count == 2 || count == 3) && std::wstring(arguments[1]) == L"--ui-smoke";
        const bool job_test = count == 4 && std::wstring(arguments[1]) == L"--ui-job-test";
        if (count != 1 && !smoke && !job_test) return 2;
        const HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        struct ComGuard { HRESULT hr; ~ComGuard() { if (SUCCEEDED(hr)) CoUninitialize(); } } com{hr};
        if (job_test) return ui(instance, true, fs::path(arguments[2]), nicolai::test_talker::parse_profile(utf8(arguments[3])));
        return ui(instance, smoke, {}, Profile::Stable, smoke && count == 3 ? fs::path(arguments[2]) : fs::path{});
    } catch (const std::exception& error) {
        std::cerr << "error=" << error.what() << '\n';
        return 1;
    }
}
