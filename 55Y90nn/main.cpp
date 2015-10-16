#include <Windows.h>
#include <tchar.h>

#include "window.h"

#if NDEBUG
# pragma comment(lib, "..\\x64\\Release\\MouseHook.lib")
#else
# pragma comment(lib, "..\\x64\\Debug\\MouseHook.lib")
#endif

__declspec(dllimport)
bool start_hook();

__declspec(dllimport)
void end_hook();

int APIENTRY _tWinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPTSTR     lpCmdLine,
	_In_ int       nCmdShow
)
{
	if(!::register_window(hInstance))
		return 0;

	HWND hwnd = ::create_window(hInstance);

	if (hwnd == nullptr)
		return 0;

	if (!::start_hook()) {
		::DestroyWindow(hwnd);
		return 0;
	}

	MSG msg;
	BOOL ret;

	while ((ret = ::GetMessage(&msg, hwnd, 0, 0)) && ret != -1) {
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}

	::end_hook();

	return static_cast<int>(msg.wParam);
}
