#pragma once

#include <Windows.h>

bool register_window(HINSTANCE hinst);
HWND create_window(HINSTANCE hinst);
LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
