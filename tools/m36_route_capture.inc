// Included inside the feature debugger's anonymous namespace. No disk patches.
constexpr std::uintptr_t kRouteEntry = 0x107c20;
struct RouteCall {
    std::uint64_t sequence;
    std::uintptr_t state, previous, current;
};

void json_runtime_state(std::ostream& out, HANDLE process, std::uintptr_t ptr) {
    out << "{\"markers\":";
    json_array(out, read_vector<std::int16_t>(process, ptr + 0x24, 6));
    out << ",\"fields\":";
    json_array(out, read_vector<std::int32_t>(process, ptr + 0x30, 5));
    out << ",\"cursors\":";
    json_array(out, read_vector<std::int32_t>(process, ptr + 0x48, 6));
    out << ",\"end_cursor\":" << read_value<std::int32_t>(process, ptr + 0x80);
    out << ",\"carry\":" << read_value<std::int16_t>(process, ptr + 0x7c);
    for (auto slot : {0x74, 0x78}) {
        out << ",\"slot_" << (slot == 0x74 ? "74" : "78") << "\":";
        const auto record = read_value<std::uint32_t>(process, ptr + slot);
        if (record) json_array(out, read_vector<std::int16_t>(process, record, 5));
        else out << "null";
    }
    out << '}';
}

void json_runtime_descriptor(std::ostream& out, HANDLE process, std::uintptr_t ptr) {
    if (!ptr) { out << "null"; return; }
    const auto nodes = read_value<std::int16_t>(process,
        read_value<std::uint32_t>(process, ptr + 0x18));
    if (nodes < 2 || nodes > kMaxNodes)
        throw std::runtime_error("runtime descriptor node count outside guarded domain");
    const auto flag = [&](int offset) {
        return read_value<BYTE>(process, read_value<std::uint32_t>(process, ptr + offset));
    };
    out << "{\"cross_flag\":" << static_cast<int>(flag(0x10))
        << ",\"descriptor_flag14\":" << static_cast<int>(flag(0x14))
        << ",\"node_count\":" << nodes << ",\"source_position\":";
    json_array(out, read_vector<std::int32_t>(process,
        read_value<std::uint32_t>(process, ptr + 0x30), nodes));
    out << ",\"duration_q11\":";
    json_array(out, read_vector<std::int16_t>(process,
        read_value<std::uint32_t>(process, ptr + 0x24), nodes - 1));
    out << ",\"pitch_q11\":";
    json_array(out, read_vector<std::int16_t>(process,
        read_value<std::uint32_t>(process, ptr + 0x28), nodes - 1));
    out << ",\"pcm_address\":" << read_value<std::uint32_t>(process, ptr + 0x4c) << '}';
}

int run_route_capture(DWORD pid, const std::string& output, const std::string& stop) {
    HANDLE process = nullptr;
    bool attached = false;
    bool active_event = false;
    bool exited = false;
    DEBUG_EVENT ev{};
    std::map<std::uintptr_t, BYTE> originals;
    std::map<DWORD, StepState> stepping;
    // Stops occur between instructions; preserve EIP/flags when detaching.
    auto restore = [&] {
        if (exited) return;
        bool failed = false;
        for (const auto& bp : originals) {
            try { write_byte(process, bp.first, bp.second); }
            catch (...) { failed = true; }
        }
        for (auto& step : stepping) {
            HANDLE thread = OpenThread(THREAD_GET_CONTEXT | THREAD_SET_CONTEXT,
                FALSE, step.first);
            if (thread) {
                CONTEXT ctx{}; ctx.ContextFlags = CONTEXT_CONTROL;
                if (GetThreadContext(thread, &ctx)) {
                    ctx.EFlags &= ~static_cast<DWORD>(0x100);
                    SetThreadContext(thread, &ctx);
                }
                CloseHandle(thread);
            }
            resume_threads(step.second.suspended_threads);
        }
        if (failed) throw std::runtime_error("could not restore every breakpoint");
    };
    try {
        if (GetFileAttributesA(output.c_str()) != INVALID_FILE_ATTRIBUTES)
            throw std::runtime_error("route capture output must be fresh");
        const auto base = find_module_base(pid, L"mtsyc32.dll");
        if (!base) throw std::runtime_error("mtsyc32.dll is not loaded");
        process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ |
            PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, pid);
        if (!process) throw std::runtime_error("OpenProcess failed");
        validate_remote_image(process, base);
        // Instruction starts verified against the pinned 2004 image. Actual
        // writer call sites, not inferred voicing, identify the observed route.
        const std::map<std::uintptr_t, std::string> points{
            {kRouteEntry, "entry"}, {0x107d54, "decision"},
            {0x107d8b, "cross"}, {0x107e28, "initial"},
            {0x107ebe, "ordinary"}, {0x108004, "interval_end"},
            {0x10801b, "post_loop"}, {0x108076, "terminal_flush"},
            {0x10808b, "failure_return"}, {0x10820d, "return"}};
        for (const auto& point : points)
            originals[base + point.first] = read_byte(process, base + point.first);
        std::ofstream out(output, std::ios::binary);
        if (!out) throw std::runtime_error("cannot open route output");
        if (!DebugActiveProcess(pid)) throw std::runtime_error("DebugActiveProcess failed");
        attached = true;
        DebugSetProcessKillOnExit(FALSE);
        for (const auto& bp : originals) write_byte(process, bp.first, 0xcc);
        SetConsoleCtrlHandler(console_handler, TRUE);
        std::map<DWORD, RouteCall> pending;
        std::uint64_t sequence = 0, records = 0, skipped = 0;
        std::cout << "M36_RUNTIME_CAPTURE_READY mode=route pid=" << pid << '\n' << std::flush;
        bool detach_requested = false;
        while (true) {
            // Restore while a debugger event stops ALL target threads. Restoring
            // into a running process can race a queued INT3 and skip an opcode.
            if (!detach_requested && (g_stop ||
                    (file_exists(stop) && pending.empty() && stepping.empty()))) {
                if (!DebugBreakProcess(process)) throw std::runtime_error("detach break failed");
                detach_requested = true;
            }
            if (!WaitForDebugEvent(&ev, 100)) {
                if (GetLastError() == ERROR_SEM_TIMEOUT) continue;
                throw std::runtime_error("WaitForDebugEvent failed");
            }
            active_event = true;
            DWORD status = DBG_CONTINUE;
            bool detach_now = false;
            if (ev.dwDebugEventCode == EXCEPTION_DEBUG_EVENT) {
                const auto code = ev.u.Exception.ExceptionRecord.ExceptionCode;
                const auto address = reinterpret_cast<std::uintptr_t>(
                    ev.u.Exception.ExceptionRecord.ExceptionAddress);
                auto bp = originals.find(address);
                if ((code == EXCEPTION_BREAKPOINT && bp != originals.end()) ||
                    (code == EXCEPTION_SINGLE_STEP && stepping.count(ev.dwThreadId))) {
                    HANDLE thread = OpenThread(THREAD_GET_CONTEXT | THREAD_SET_CONTEXT,
                        FALSE, ev.dwThreadId);
                    if (!thread) throw std::runtime_error("OpenThread failed");
                    struct ThreadGuard { HANDLE h; ~ThreadGuard() { CloseHandle(h); } } guard{thread};
                    CONTEXT ctx{}; ctx.ContextFlags = CONTEXT_FULL;
                    if (!GetThreadContext(thread, &ctx)) throw std::runtime_error("GetThreadContext failed");
                    if (code == EXCEPTION_SINGLE_STEP) {
                        auto& step = stepping.at(ev.dwThreadId);
                        write_byte(process, step.breakpoint_address, 0xcc);
                        ctx.EFlags &= ~static_cast<DWORD>(0x100);
                        if (!SetThreadContext(thread, &ctx)) throw std::runtime_error("SetThreadContext step failed");
                        resume_threads(step.suspended_threads);
                        stepping.erase(ev.dwThreadId);
                    } else {
                        // Rewind first, so even capture errors can safely detach.
                        write_byte(process, address, bp->second);
                        ctx.Eip = static_cast<DWORD>(address);
                        if (!SetThreadContext(thread, &ctx)) throw std::runtime_error("SetThreadContext rewind failed");
                        const auto rva = address - base;
                        const auto& name = points.at(rva);
                        try {
                            if (rva == kRouteEntry) {
                                if (pending.count(ev.dwThreadId)) throw std::runtime_error("nested route call");
                                RouteCall call{++sequence,
                                    read_value<std::uint32_t>(process, ctx.Esp + 16),
                                    read_value<std::uint32_t>(process, ctx.Esp + 8),
                                    read_value<std::uint32_t>(process, ctx.Esp + 12)};
                                pending[ev.dwThreadId] = call;
                            }
                            auto it = pending.find(ev.dwThreadId);
                            if (it != pending.end()) {
                                // Stage the whole row to prevent partial JSON on read failure.
                                std::ostringstream row;
                                row << "{\"schema\":\"nicolai-m36-route-v1\",\"call\":"
                                    << it->second.sequence << ",\"thread_id\":" << ev.dwThreadId
                                    << ",\"event\":\"" << name << "\",\"state\":";
                                json_runtime_state(row, process, it->second.state);
                                if (rva == kRouteEntry) {
                                    const auto context = read_value<std::uint32_t>(process, ctx.Esp + 4);
                                    row << ",\"flush_gate\":" << read_value<std::int32_t>(process, context + 0x7c)
                                        << ",\"previous\":";
                                    json_runtime_descriptor(row, process, it->second.previous);
                                    row << ",\"current\":";
                                    json_runtime_descriptor(row, process, it->second.current);
                                } else if (name == "decision") {
                                    row << ",\"interval_index\":" << ctx.Esi
                                        << ",\"cross_pending\":" << read_value<std::int16_t>(process, ctx.Esp + 0x48)
                                        << ",\"already_started\":" << read_value<std::int32_t>(process, ctx.Esp + 0x14)
                                        << ",\"flush_gate_word\":" << read_value<std::int16_t>(process, ctx.Esp + 0x1c)
                                        << ",\"step\":";
                                    auto step = read_vector<std::int16_t>(process, ctx.Esp + 0x30, 5);
                                    step[0] = static_cast<std::int16_t>(ctx.Esi);
                                    json_array(row, step);
                                } else if (name == "return" || name == "failure_return") {
                                    row << ",\"return_value\":" << ctx.Eax;
                                }
                                row << "}\n";
                                out << row.str() << std::flush;
                                if (!out) throw std::runtime_error("route write failed");
                                ++records;
                            }
                        } catch (const std::exception&) {
                            ++skipped;
                            pending.erase(ev.dwThreadId);
                        }
                        if (name == "return" || name == "failure_return") pending.erase(ev.dwThreadId);
                        StepState step;
                        step.breakpoint_address = address;
                        step.suspended_threads = suspend_other_threads(pid, ev.dwThreadId);
                        stepping[ev.dwThreadId] = std::move(step);
                        ctx.EFlags |= 0x100;
                        if (!SetThreadContext(thread, &ctx)) throw std::runtime_error("SetThreadContext trace failed");
                    }
                } else if (code == EXCEPTION_BREAKPOINT && detach_requested) {
                    restore();
                    detach_now = true;
                } else if (code != EXCEPTION_BREAKPOINT) status = DBG_EXCEPTION_NOT_HANDLED;
            } else if (ev.dwDebugEventCode == CREATE_PROCESS_DEBUG_EVENT) {
                if (ev.u.CreateProcessInfo.hFile) CloseHandle(ev.u.CreateProcessInfo.hFile);
                if (ev.u.CreateProcessInfo.hThread) CloseHandle(ev.u.CreateProcessInfo.hThread);
                if (ev.u.CreateProcessInfo.hProcess) CloseHandle(ev.u.CreateProcessInfo.hProcess);
            } else if (ev.dwDebugEventCode == CREATE_THREAD_DEBUG_EVENT) {
                if (ev.u.CreateThread.hThread) CloseHandle(ev.u.CreateThread.hThread);
            } else if (ev.dwDebugEventCode == LOAD_DLL_DEBUG_EVENT) {
                if (ev.u.LoadDll.hFile) CloseHandle(ev.u.LoadDll.hFile);
            } else if (ev.dwDebugEventCode == EXIT_THREAD_DEBUG_EVENT) {
                pending.erase(ev.dwThreadId);
                auto it = stepping.find(ev.dwThreadId);
                if (it != stepping.end()) { resume_threads(it->second.suspended_threads); stepping.erase(it); }
            } else if (ev.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT) exited = true;
            if (!ContinueDebugEvent(ev.dwProcessId, ev.dwThreadId, status))
                throw std::runtime_error("ContinueDebugEvent failed");
            active_event = false;
            if (exited || detach_now) break;
        }
        if (!exited && !DebugActiveProcessStop(pid)) throw std::runtime_error("detach failed");
        attached = false;
        CloseHandle(process);
        std::cout << "{\"records\":" << records << ",\"skipped\":" << skipped << "}\n";
        return records && !skipped && pending.empty() ? 0 : 3;
    } catch (const std::exception& e) {
        try { restore(); } catch (...) {}
        if (active_event) ContinueDebugEvent(ev.dwProcessId, ev.dwThreadId, DBG_CONTINUE);
        if (attached && !exited) DebugActiveProcessStop(pid);
        if (process) CloseHandle(process);
        std::cerr << e.what() << '\n';
        return 1;
    }
}
