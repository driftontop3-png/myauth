#pragma once
#include <Windows.h>

namespace input
{
	static BYTE ntusersendinput_bytes[30];
	HMODULE user32 = 0;
	HMODULE win32u = 0;
	static BOOLEAN WINAPI init()
	{
		HMODULE user32_lib = LoadLibraryA("user32.dll");
		if (!user32_lib) return false;
		HMODULE win32u_lib = LoadLibraryA("win32u.dll");
		if (!win32u_lib) return false;
		user32 = GetModuleHandleA("user32.dll");
		if (!user32) return false;
		win32u = GetModuleHandleA("win32u.dll");
		if (!win32u) return false;

		LPVOID ntusersendinput_addr = GetProcAddress(user32, "NtUserSendInput");
		if (!ntusersendinput_addr)
		{
			ntusersendinput_addr = GetProcAddress(win32u, "NtUserSendInput");
			if (!ntusersendinput_addr) return FALSE;
		}
		memcpy(ntusersendinput_bytes, ntusersendinput_addr, 30);
		return TRUE;
	}
	static BOOLEAN WINAPI ntusersendinput(UINT cinputs, LPINPUT pinputs, int cbsize)
	{
		LPVOID ntusersendinput_spoof = VirtualAlloc(0, 0x1000, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if (!ntusersendinput_spoof) return FALSE;
		memcpy(ntusersendinput_spoof, ntusersendinput_bytes, 30);
		NTSTATUS result = reinterpret_cast<NTSTATUS(NTAPI*)(UINT, LPINPUT, int)>(ntusersendinput_spoof)(cinputs, pinputs, cbsize);
		ZeroMemory(ntusersendinput_spoof, 0x1000);
		VirtualFree(ntusersendinput_spoof, 0, MEM_RELEASE);
		return (result > 0);
	}
	static BOOLEAN WINAPI move_mouse(int x, int y)
	{
		INPUT input = { 0 };
		input.type = INPUT_MOUSE;
		input.mi.mouseData = 0;
		input.mi.time = 0;
		input.mi.dx = x;
		input.mi.dy = y;
		input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_VIRTUALDESK;
		return ntusersendinput(1, &input, sizeof(input));
	}
	static void left_down()
	{
		INPUT input = {};
		input.type = INPUT_MOUSE;
		input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
		if (!ntusersendinput(1, &input, sizeof(INPUT)))
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
	}

	static void left_up()
	{
		INPUT input = {};
		input.type = INPUT_MOUSE;
		input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
		if (!ntusersendinput(1, &input, sizeof(INPUT)))
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	}

	static void left_click()
	{
		left_down();
		left_up();
	}

	static void right_down()
	{
		INPUT input = {};
		input.type = INPUT_MOUSE;
		input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
		if (!ntusersendinput(1, &input, sizeof(INPUT)))
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
	}

	static void right_up()
	{
		INPUT input = {};
		input.type = INPUT_MOUSE;
		input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
		if (!ntusersendinput(1, &input, sizeof(INPUT)))
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
	}

	static void right_click()
	{
		right_down();
		right_up();
	}

	static void right_click_game(HWND game_window)
	{
		if (game_window)
		{
			const int cx = GetSystemMetrics(SM_CXSCREEN) / 2;
			const int cy = GetSystemMetrics(SM_CYSCREEN) / 2;
			const LPARAM lp = MAKELPARAM(cx, cy);
			PostMessageA(game_window, WM_RBUTTONDOWN, MK_RBUTTON, lp);
			PostMessageA(game_window, WM_RBUTTONUP, 0, lp);
		}

		right_click();
	}

	static void left_click_game(HWND game_window)
	{
		if (game_window)
		{
			const int cx = GetSystemMetrics(SM_CXSCREEN) / 2;
			const int cy = GetSystemMetrics(SM_CYSCREEN) / 2;
			const LPARAM lp = MAKELPARAM(cx, cy);
			PostMessageA(game_window, WM_LBUTTONDOWN, MK_LBUTTON, lp);
			PostMessageA(game_window, WM_LBUTTONUP, 0, lp);
		}

		left_click();
	}

	static void hold_left_game(HWND game_window, bool down)
	{
		if (game_window)
		{
			const int cx = GetSystemMetrics(SM_CXSCREEN) / 2;
			const int cy = GetSystemMetrics(SM_CYSCREEN) / 2;
			const LPARAM lp = MAKELPARAM(cx, cy);
			PostMessageA(game_window, down ? WM_LBUTTONDOWN : WM_LBUTTONUP, down ? MK_LBUTTON : 0, lp);
		}

		if (down)
			left_down();
		else
			left_up();
	}
}