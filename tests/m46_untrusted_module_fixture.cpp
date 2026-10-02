// Synthetic, deliberately NOT the original DLL. No voice data or original code.
#include <windows.h>
extern "C" __declspec(dllexport) int m46_untrusted_fixture() { return 46; }
BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID) { return TRUE; }
