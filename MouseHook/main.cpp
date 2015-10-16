#include <Windows.h>
#include <windowsx.h>

#include <string>

HINSTANCE hinst;
HHOOK hhook;

LRESULT CALLBACK keyboard_proc(
	_In_ int    nCode,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
	);

BOOL WINAPI DllMain(
	_In_ HINSTANCE hinstDLL,
	_In_ DWORD     fdwReason,
	_In_ LPVOID    lpvReserved
)
{
	if (fdwReason == DLL_PROCESS_ATTACH) {
		::hinst = hinstDLL;
	}

	return TRUE;
}

__declspec(dllexport)
bool start_hook()
{
	hhook = ::SetWindowsHookEx(WH_MOUSE_LL, ::keyboard_proc, ::hinst, 0);
	return hhook != nullptr;
}

__declspec(dllexport)
void end_hook()
{
	if (hhook != nullptr)
		::UnhookWindowsHookEx(hhook);
}

MSLLHOOKSTRUCT s;
HWND htarget;
POINT presspos;
bool pressing = false;
bool moved;
bool toward, right;

LRESULT CALLBACK keyboard_proc(
	_In_ int    nCode,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
)
{
	if (wParam == WM_MBUTTONDOWN) {
		if (::GetCursorPos(&presspos) != FALSE) {
			HWND hwnd = htarget;
			POINT pt = presspos;

			hwnd = ::WindowFromPoint(pt);

			do {
				htarget = hwnd;

				::ScreenToClient(htarget, &pt);
				hwnd = ::ChildWindowFromPointEx(htarget, pt, CWP_ALL);
			} while (hwnd != nullptr && hwnd != htarget);

			pressing = true;
			moved = false;

			s = *(MSLLHOOKSTRUCT*)lParam;

			wchar_t a[100];
			wsprintf(a, L"%x\n", htarget);
			::OutputDebugString(a);
		}

		if (nCode >= 0)
			return 1;
	}
	else if (wParam == WM_MBUTTONUP) {
		pressing = false;

		if (!moved) {
			INPUT i[2] = {};
			i[0].type = INPUT_MOUSE;
			i[0].mi.dwFlags = MOUSEEVENTF_MIDDLEDOWN | MOUSEEVENTF_ABSOLUTE;
			i[0].mi.dx = presspos.x;
			i[0].mi.dy = presspos.y;

			i[1].type = INPUT_MOUSE;
			i[1].mi.dwFlags = MOUSEEVENTF_MIDDLEUP | MOUSEEVENTF_ABSOLUTE;
			i[1].mi.dx = presspos.x;
			i[1].mi.dy = presspos.y;

			::SendInput(2, i, sizeof(INPUT));
		}

		if (nCode >= 0)
			return 1;
	}
	else if (pressing && wParam == WM_MOUSEMOVE) {
		moved = true;

		auto lphook = reinterpret_cast<LPMSLLHOOKSTRUCT>(lParam);

		auto dx = lphook->pt.x - presspos.x;
		auto dy = lphook->pt.y - presspos.y;

		INPUT i = {};

		if (right == (dx > 0) &&dx != 0) {
			DWORD delta = WHEEL_DELTA * dx / 4;

			if (dx < 0)
				delta--;
			else
				delta++;

			i.type = INPUT_MOUSE;
			i.mi.dwFlags = MOUSEEVENTF_HWHEEL;
			i.mi.mouseData = delta;
			::SendInput(1, &i, sizeof(INPUT));
		}

		if(toward == (dy < 0) && dy != 0){
			DWORD delta = - WHEEL_DELTA * dy / 4;

			if (dy < 0)
				delta++;
			else
				delta--;

			i.type = INPUT_MOUSE;
			i.mi.dwFlags = MOUSEEVENTF_WHEEL;
			i.mi.mouseData = delta;
			::SendInput(1, &i, sizeof(INPUT));
		}

		right = dx > 0;
		toward = dy < 0;

		if(nCode >= 0)
			return 1;
	}

	return ::CallNextHookEx(hhook, nCode, wParam, lParam);
}
