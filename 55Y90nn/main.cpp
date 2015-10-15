#include <Windows.h>
#include <tchar.h>

#if NDEBUG
# pragma comment(lib, "..\\x64\\Release\\MouseHook.lib")
#else
# pragma comment(lib, "..\\x64\\Debug\\MouseHook.lib")
#endif

int APIENTRY _tWinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPTSTR     lpCmdLine,
	_In_ int       nCmdShow
)
{


	return 0;
}
