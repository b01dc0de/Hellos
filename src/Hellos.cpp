#include "Hellos.h"
#include "Graphics_DX11.h"

#if UNICODE
#define APPNAME() (L"Hellos")
#else
#define APPNAME() ("Hellos")
#endif

// Globals
bool bRunning = false;
HWND hWindow;
UINT WinResX = 1600U;
UINT WinResY = 900U;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND InitWindow(HINSTANCE hInstance)
{
	WNDCLASSEX WndClass = {};
	WndClass.cbSize = sizeof(WNDCLASSEX);
	WndClass.style = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = WindowProc;
	WndClass.hInstance = hInstance;
	WndClass.lpszClassName = APPNAME();

	RegisterClassEx(&WndClass);

	RECT WndRect = { 0, 0, (LONG)WinResX, (LONG)WinResY };
	UINT WndStyle = WS_CAPTION | WS_OVERLAPPEDWINDOW;
	UINT WndExStyle = WS_EX_OVERLAPPEDWINDOW;
	AdjustWindowRectEx(&WndRect, WndStyle, FALSE, WndExStyle);

	HWND NewWindow = CreateWindowEx(
		WndExStyle,
		APPNAME(),
		APPNAME(),
		WndStyle,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		WndRect.right - WndRect.left,
		WndRect.bottom - WndRect.top,
		nullptr,
		nullptr,
		hInstance,
		nullptr
	);

	return NewWindow;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT Result = 0;
	switch (uMsg)
	{
		case WM_KEYUP:
		{
			if (VK_ESCAPE == wParam)
			{
				bRunning = false;
			}
		} break;
		case WM_CLOSE:
		{
			bRunning = false;
		} break;
		default:
		{
			Result = DefWindowProc(hwnd, uMsg, wParam, lParam);
		} break;
	}
	
	return Result;
}

int WINAPI WinMain_EmptyWindow(HINSTANCE hInst, HINSTANCE hPrevInst, PSTR CmdLine, int WndShow)
{
	(void)hPrevInst;
	(void)CmdLine;
	if (HWND hWnd = InitWindow(hInst))
	{
		hWindow = hWnd;

		ShowWindow(hWindow, WndShow);

		bRunning = true;
		while (bRunning)
		{
			MSG msg;
			while (PeekMessage(&msg, hWindow, 0, 0, PM_REMOVE) > 0)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
	return 0;
}
int WINAPI WinMain_DX11_Triangle(HINSTANCE hInst, HINSTANCE hPrevInst, PSTR CmdLine, int WndShow)
{
	(void)hPrevInst;
	(void)CmdLine;
	if (HWND hWnd = InitWindow(hInst))
	{
		hWindow = hWnd;

		HRESULT Result = Graphics_DX11::InitGraphics();
		if (Result != S_OK)
		{
			DebugBreak();
		}

		ShowWindow(hWindow, WndShow);

		bRunning = true;
		while (bRunning)
		{
			// Get input
			MSG msg;
			BOOL MsgResult;
			while ((MsgResult = PeekMessage(&msg, hWindow, 0, 0, PM_REMOVE)) > 0)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			if (MsgResult == -1)
			{
				bRunning = false;
				break;
			}

			// Update
			UpdateWindow(hWindow);

			Graphics_DX11::Draw();
		}
	}

	return 0;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, PSTR CmdLine, int WndShow)
{
	int Result = 0;

	static const int Project_Win32_EmptyWindow= 0;
	static const int Project_Win32_DX11_Triangle = 1;

	static int BuildProject = Project_Win32_DX11_Triangle;
	switch (BuildProject)
	{
		case Project_Win32_EmptyWindow:
		{
			Result = WinMain_EmptyWindow(hInst, hPrevInst, CmdLine, WndShow);
		} break;
		case Project_Win32_DX11_Triangle:
		{
			Result = WinMain_DX11_Triangle(hInst, hPrevInst, CmdLine, WndShow);
		} break;
		default:
		{
			Result = 1;
			DebugBreak();
		} break;
	}

	return Result;
}

