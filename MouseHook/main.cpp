#include <Windows.h>


BOOL WINAPI DllMain(
	_In_ HINSTANCE hinstDLL,
	_In_ DWORD     fdwReason,
	_In_ LPVOID    lpvReserved
)
{
	if (fdwReason == DLL_PROCESS_ATTACH) {
		// load
	}
	else if (fdwReason == DLL_PROCESS_DETACH) {
		// unload
	}

	return TRUE;
}

__declspec(dllexport)
void func()
{
}
