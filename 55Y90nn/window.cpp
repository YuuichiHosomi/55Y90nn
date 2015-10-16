#include <Windows.h>
#include <windowsx.h>
#include <winerror.h>
#include <ShlObj.h>
#include <tchar.h>

#include <string>

#include "window.h"
#include "resource.h"

using string_type = ::std::basic_string<TCHAR>;

constexpr UINT id_tasktray = 1000;
constexpr UINT callback_msg = WM_APP;

std::wstring get_startup_path(HWND hwnd);

bool register_window(HINSTANCE hinst)
{
	WNDCLASSEX wcex = {};

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.lpfnWndProc = ::window_proc;
	wcex.hInstance = hinst;
	wcex.lpszClassName = _T("55Y90nn_DummyWindow");

	return ::RegisterClassEx(&wcex) != 0;
}

HWND create_window(HINSTANCE hinst)
{
	HWND hwnd = ::CreateWindow(
		_T("55Y90nn_DummyWindow"),
		_T("unnamed"),
		0,
		0,
		0,
		0,
		0,
		nullptr,
		nullptr,
		hinst,
		nullptr);

	return hwnd;
}

LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	auto on_create = [&](...) -> bool {
		HINSTANCE hinst = ::GetModuleHandle(nullptr);

		HICON hicon = static_cast<HICON>(LoadImage(
			hinst, MAKEINTRESOURCE(IDI_ICON32),
			IMAGE_ICON,
			0,
			0,
			LR_DEFAULTSIZE | LR_SHARED));

		if (hicon == nullptr)
			return false;

		NOTIFYICONDATA nid = {};
		nid.cbSize = sizeof(NOTIFYICONDATA);
		nid.hWnd = hwnd;
		nid.uID = id_tasktray;
		nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
		nid.uCallbackMessage = callback_msg;
		nid.hIcon = hicon;
		::_tcscpy_s(nid.szTip, _T("55Y90nn"));

		return ::Shell_NotifyIcon(NIM_ADD, &nid) != FALSE;
	};

	auto on_command = [&](HWND hwnd, int id, HWND hctl, UINT code) -> void{

	};

	switch (msg) {
		HANDLE_MSG(hwnd, WM_CREATE, on_create);
		HANDLE_MSG(hwnd, WM_COMMAND, on_command);

	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;

	case callback_msg:
		if (wParam == id_tasktray) {
			::DestroyWindow(hwnd);

			return 0;
		}

		break;
	}

	return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

string_type get_startup_path(HWND hwnd)
{
	LPITEMIDLIST pidlist = nullptr;
	HANDLE htoken;
	TCHAR startup[MAX_PATH];

	if (::OpenProcessToken(::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &htoken) == FALSE) {
		return _T("");
	}

	if(SUCCEEDED(::SHGetFolderLocation(hwnd, CSIDL_STARTUP, htoken, 0, &pidlist)))
		::SHGetPathFromIDList(pidlist, startup);

	::CloseHandle(htoken);
	CoTaskMemFree(pidlist);

	return startup;
}
