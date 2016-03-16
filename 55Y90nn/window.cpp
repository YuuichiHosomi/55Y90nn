
#include <Windows.h>
#include <windowsx.h>
#include <winerror.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#include <tchar.h>
#include <comdef.h>

#include <string>

#include "window.h"
#include "resource.h"

using string_type = ::std::basic_string<TCHAR>;

constexpr UINT id_tasktray = 1000;
constexpr UINT callback_msg = WM_APP;

string_type get_startup_dir(HWND hwnd);

_COM_SMARTPTR_TYPEDEF(IShellLink, __uuidof(IShellLink));

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

	auto on_command = [&](HWND hwnd, int id, HWND hctl, UINT code) -> void {
		switch (id) {
		case ID_MENU_ADDTOSTARTUP:
		{
			bool succeeded = false;

			string_type startup_dir = ::get_startup_dir(hwnd);
			string_type shortcut_path;
			string_type process_path;
			string_type working_dir;

			shortcut_path.reserve(MAX_PATH);
			process_path.reserve(MAX_PATH);
			working_dir.reserve(_MAX_DRIVE + _MAX_DIR);

			if (startup_dir.size() != 0) {
				TCHAR shortcut_path_str[MAX_PATH];

				if (::PathCombine(shortcut_path_str, startup_dir.data(), _T("55Y90nn.lnk")) != nullptr) {
					shortcut_path = shortcut_path_str;

					TCHAR process_path_str[MAX_PATH];

					if (::GetModuleFileName(nullptr, process_path_str, MAX_PATH) < MAX_PATH) {
						process_path = process_path_str;

						TCHAR drive[_MAX_DRIVE], dir[_MAX_DIR];

						if (::_tsplitpath_s(process_path_str, drive, _MAX_DRIVE, dir, _MAX_DIR, nullptr, 0, nullptr, 0) != EINVAL) {
							working_dir = drive;
							working_dir += dir;
						}
					}
				}

				if (working_dir.size() != 0) {
					IShellLinkPtr psl;
					IPersistFilePtr ppf;

					psl.CreateInstance(CLSID_ShellLink);

					if (SUCCEEDED(psl->SetPath(process_path.data()))
							&& SUCCEEDED(psl->SetDescription(_T("wheel emulator for trackpoint")))
							&& SUCCEEDED(psl->SetWorkingDirectory(working_dir.data()))
							&& SUCCEEDED(psl->QueryInterface(&ppf))) {
						succeeded = SUCCEEDED(ppf->Save(shortcut_path.data(), TRUE));
					}
				}
			}

			if (succeeded) {
				::MessageBox(hwnd, _T("succeeded to add to startup"), _T("message"), MB_OK);
			} else {
				::MessageBox(hwnd, _T("failed to add to startup"), _T("error"), MB_OK | MB_ICONWARNING);
			}

			break;
		}

		case ID_MENU_EXIT:
			::DestroyWindow(hwnd);
			break;
		}
	};

	switch (msg) {
		HANDLE_MSG(hwnd, WM_CREATE, on_create);
		HANDLE_MSG(hwnd, WM_COMMAND, on_command);

	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;

	case callback_msg:
		if (wParam == id_tasktray) {
			if (lParam == WM_LBUTTONDOWN || lParam == WM_RBUTTONDOWN) {
				auto hinst = ::GetModuleHandle(nullptr);

				HMENU hmenu = ::LoadMenu(hinst, MAKEINTRESOURCE(IDR_MENU));
				HMENU hsubmenu = nullptr;

				if (hmenu != nullptr) {
					hsubmenu = ::GetSubMenu(hmenu, 0);
				}

				if (hsubmenu != nullptr) {
					POINT pt;
					::GetCursorPos(&pt);
					::SetForegroundWindow(hwnd);
					::TrackPopupMenu(hsubmenu, TPM_BOTTOMALIGN, pt.x, pt.y, 0, hwnd, nullptr);
					::DestroyMenu(hmenu);
				}
			}

			return 0;
		}

		break;
	}

	return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

string_type get_startup_dir(HWND hwnd)
{
	HANDLE htoken;

	if (::OpenProcessToken(::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &htoken)) {
		LPITEMIDLIST piil;

		if (SUCCEEDED(::SHGetFolderLocation(hwnd, CSIDL_STARTUP, htoken, 0, &piil))) {
			TCHAR startup_dir[MAX_PATH];

			if (::SHGetPathFromIDList(piil, startup_dir)) {
				return startup_dir;
			}

			::CoTaskMemFree(piil);
		}

		::CloseHandle(htoken);
	}

	return _T("");
}
