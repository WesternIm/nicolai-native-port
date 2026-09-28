#include <windows.h>
#include <string>
int main(int argc, char** argv) {
    OutputDebugStringA("startup fixture: quote=\" slash=\\ newline=\n");
    if (argc == 2 && std::string(argv[1]) == "wait") Sleep(10000);
    return argc == 2 && std::string(argv[1]) == "fail" ? 7 : 0;
}
