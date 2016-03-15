#include <Windows.h>
#include <windowsx.h>

#include <string>

#define FORWARD_WM_MOUSEHWHEEL(hwnd, xPos, yPos, zDelta, fwKeys, fn) \
    (void)(fn)((hwnd), WM_MOUSEHWHEEL, MAKEWPARAM((fwKeys),(zDelta)), MAKELPARAM((xPos),(yPos)))


HINSTANCE hinst;
HHOOK hhook;

LRESULT CALLBACK mouse_proc(
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
	hhook = ::SetWindowsHookEx(WH_MOUSE_LL, ::mouse_proc, ::hinst, 0);
	return hhook != nullptr;
}

__declspec(dllexport)
void end_hook()
{
	if (hhook != nullptr)
		::UnhookWindowsHookEx(hhook);
}

HWND htarget;
POINT presspos;
bool pressing = false;
bool moved;
bool toward, right;
bool sended = false;

LRESULT CALLBACK mouse_proc(
	_In_ int    nCode,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
)
{
	if (nCode == HC_ACTION) {
		if (wParam == WM_MBUTTONDOWN) {
			if (!sended) {
				if (::GetCursorPos(&presspos) != FALSE)
				{
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
				}
				
				return 1;
			}
		}
		else if (wParam == WM_MBUTTONUP) {
			pressing = false;

			if (sended) {
				sended = false;
				return ::CallNextHookEx(hhook, nCode, wParam, lParam);
			}
			else if (!moved) {
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
				sended = true;
			}
			
			return 1;
		}
		else if (pressing && wParam == WM_MOUSEMOVE) {
			moved = true;

			auto lphook = reinterpret_cast<LPMSLLHOOKSTRUCT>(lParam);

			auto dx = lphook->pt.x - presspos.x;
			auto dy = lphook->pt.y - presspos.y;

			INPUT i = {};

			if (right == (dx > 0) && dx != 0) {
				DWORD delta = WHEEL_DELTA * dx / 5;

				if (dx < 0)
					delta--;
				else
					delta++;

				POINT pt = presspos;
				::ScreenToClient(htarget, &pt);

				//FORWARD_WM_MOUSEHWHEEL(htarget, pt.x, pt.y, delta, 0, ::PostMessage);

				//htarget = ::GetAncestor(htarget, GA_PARENT);
				//HWND hparent = ::GetAncestor(htarget, GA_PARENT);

				//FORWARD_WM_HSCROLL(hparent, htarget, (dx > 0 ? SB_LINERIGHT : SB_LINELEFT), 0, ::PostMessage);
				//FORWARD_WM_HSCROLL(hparent, htarget, SB_ENDSCROLL, 0, ::PostMessage);
			}

			if (toward == (dy < 0) && dy != 0) {
				DWORD delta = -WHEEL_DELTA * dy / 5;

				if (dy < 0)
					delta++;
				else
					delta--;

				POINT pt = presspos;
				::ScreenToClient(htarget, &pt);

				FORWARD_WM_MOUSEWHEEL(htarget, pt.x, pt.y, delta, 0, ::PostMessage);
			}

			right = dx > 0;
			toward = dy < 0;
			
			return 1;
		}
	}

	return ::CallNextHookEx(hhook, nCode, wParam, lParam);
}
