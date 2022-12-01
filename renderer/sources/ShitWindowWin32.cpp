/**
 * @file ShitWindowWin32.cpp
 * @author yangzs
 * @brief
 * @version 0.1
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifdef _WIN32
#include "ShitWindowWin32.hpp"

#include <windowsx.h>
#include <strsafe.h>

namespace Shit
{
	UINT Map(InputModifierBits modifier)
	{
		static UINT modeArray[]{
			MOD_ALT,
			MOD_CONTROL,
			MOD_NOREPEAT,
			MOD_SHIFT,
			MOD_WIN,
		};
		return static_cast<UINT>(modifier);
	}
	void ErrorExit(LPTSTR lpszFunction)
	{
		// Retrieve the system error message for the last-error code

		LPVOID lpMsgBuf;
		LPVOID lpDisplayBuf;
		DWORD dw = GetLastError();

		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf,
			0, NULL);

		// Display the error message and exit the process

		lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
										  (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
		StringCchPrintf((LPTSTR)lpDisplayBuf,
						LocalSize(lpDisplayBuf) / sizeof(TCHAR),
						TEXT("%s failed with error %d: %s"),
						lpszFunction, dw, lpMsgBuf);
		MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

		LocalFree(lpMsgBuf);
		LocalFree(lpDisplayBuf);
		ExitProcess(dw);
	}
	//==========================================

	LRESULT CALLBACK WindowWin32::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		WindowWin32 *pThis = NULL;

		if (uMsg == WM_NCCREATE)
		{
			CREATESTRUCT *pCreate = (CREATESTRUCT *)lParam;
			pThis = (WindowWin32 *)pCreate->lpCreateParams;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
			pThis->mHwnd = hwnd;
		}
		else
		{
			pThis = (WindowWin32 *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		}
		if (pThis)
		{
			return pThis->HandleMessage(uMsg, wParam, lParam);
		}
		else
		{
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
	}
	BOOL WindowWin32::Create(
		PCWSTR lpWindowName,
		DWORD dwStyle,
		DWORD dwExStyle,
		int x,
		int y,
		int nWidth,
		int nHeight,
		HWND hWndParent,
		HMENU hMenu)
	{
		mInstance = GetModuleHandle(NULL);

		WNDCLASS wc = {0};

		wc.lpfnWndProc = WindowProc;
		wc.hInstance = mInstance;
		wc.lpszClassName = ClassName();
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);

		RegisterClass(&wc);

		mHwnd = CreateWindowEx(
			dwExStyle,	  // Optional window styles.
			ClassName(),  // Window class
			lpWindowName, // Window text
			dwStyle,	  // Window style

			// Size and position
			x, y, nWidth, nHeight,

			hWndParent, // Parent window
			hMenu,		// Menu
			mInstance,	// Instance handle
			this		// Additional application data
		);
		if (!mHwnd)
		{
			WCHAR str[] = L"CreateWindowEx";
			ErrorExit(str);
		}

		return (mHwnd ? TRUE : FALSE);
	}

	WindowWin32::WindowWin32(const WindowCreateInfo &createInfo, RenderSystem *pRenderSystem)
		: Shit::Window(createInfo, pRenderSystem)
	{
		DWORD dwStyle{WS_OVERLAPPEDWINDOW};
		if (!static_cast<bool>(mCreateInfo.flags & WindowCreateFlagBits::INVISIBLE))
		{
			dwStyle |= WS_VISIBLE;
		}
		if (!static_cast<bool>(mCreateInfo.flags & WindowCreateFlagBits::FIXED_SIZE))
		{
			dwStyle |= WS_MINIMIZEBOX;
			dwStyle |= WS_MAXIMIZEBOX;
			dwStyle |= WS_THICKFRAME;
		}

		RECT rect{
			(LONG)createInfo.rect.offset.x,
			(LONG)createInfo.rect.offset.y,
			(LONG)(createInfo.rect.offset.x + createInfo.rect.extent.width),
			(LONG)(createInfo.rect.offset.y + createInfo.rect.extent.height)};
		AdjustWindowRect(&rect, dwStyle, false);
		Create(createInfo.name, dwStyle, 0,
			   rect.left,
			   rect.top,
			   rect.right - rect.left,
			   rect.bottom - rect.top);
	}
	LRESULT WindowWin32::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		Event ev{};
		ev.pWindow = this;
		switch (uMsg)
		{
		case WM_DESTROY:
			ev.value = WindowCloseEvent{};
			PostQuitMessage(0);
			break;
		case WM_SIZE:
			mCreateInfo.rect.extent.width = LOWORD(lParam);
			mCreateInfo.rect.extent.height = HIWORD(lParam);
			ev.value = WindowResizeEvent{mCreateInfo.rect.extent.width, mCreateInfo.rect.extent.height};
			break;
		case WM_MOUSEMOVE:
			ev.modifier = MapKeyModifier(static_cast<uint32_t>(wParam));
			ev.value = MouseMoveEvent{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
			break;
		case WM_MOUSEWHEEL:
			ev.modifier = MapKeyModifier(LOWORD(wParam));
			ev.value = MouseWheelEvent{0, int((wParam >> 31) ? ((wParam >> 16) & 0x7fff) - 0x8000 : HIWORD(wParam)) / WHEEL_DELTA};
			break;
		//=========================
		// mouse button
		case WM_LBUTTONDOWN:
			ev.modifier = MapKeyModifier(static_cast<uint32_t>(wParam));
			ev.value = MouseButtonEvent{MouseButton::MOUSE_L, true};
			break;
		case WM_LBUTTONUP:
			ev.modifier = MapKeyModifier(static_cast<uint32_t>(wParam));
			ev.value = MouseButtonEvent{MouseButton::MOUSE_L, false};
			break;
		case WM_RBUTTONDOWN:
			ev.value = MouseButtonEvent{MouseButton::MOUSE_R, true};
			ev.modifier = MapKeyModifier(static_cast<uint32_t>(wParam));
			break;
		case WM_RBUTTONUP:
			ev.value = MouseButtonEvent{MouseButton::MOUSE_R, false};
			ev.modifier = MapKeyModifier(static_cast<uint32_t>(wParam));
			break;
		case WM_MBUTTONDOWN:
			ev.value = MouseButtonEvent{MouseButton::MOUSE_M, true};
			ev.modifier = MapKeyModifier(static_cast<uint32_t>(wParam));
			break;
		case WM_MBUTTONUP:
			ev.value = MouseButtonEvent{MouseButton::MOUSE_M, false};
			ev.modifier = MapKeyModifier(static_cast<uint32_t>(wParam));
			break;
		//==================================================
		case WM_KEYDOWN:
		{
			ev.value = KeyEvent{
				MapKey(static_cast<uint32_t>(wParam)),
				true,
				bool((lParam >> 30) & 1),
				bool((lParam >> 29) & 1),
			};
			break;
		}
		case WM_KEYUP:
		{
			ev.value = KeyEvent{
				MapKey(static_cast<uint32_t>(wParam)),
				false,
				bool((lParam >> 30) & 1),
				bool((lParam >> 29) & 1),
			};
			break;
		}
		case WM_SYSKEYDOWN:
			ev.value = KeyEvent{
				MapKey(static_cast<uint32_t>(wParam)),
				true,
				bool((lParam >> 30) & 1),
				bool((lParam >> 29) & 1),
			};
			break;
		case WM_SYSKEYUP:
			ev.value = KeyEvent{
				MapKey(static_cast<uint32_t>(wParam)),
				false,
				bool((lParam >> 30) & 1),
				bool((lParam >> 29) & 1),
			};
			break;
		//=================================================
		case WM_CHAR:
			ev.value = Event::EventType{CharEvent{static_cast<uint32_t>(wParam)}};
			break;
		// case WM_DROPFILES:
		//{
		//	char filename[256];
		//	DragQueryFileA((HDROP)wParam, 0, filename, 255);
		//	char *p = filename;
		//	ev.value = Event::EventType{DropEvent{1, &p}};
		//	DragFinish((HDROP)wParam);
		// }
		// break;
		default:
			break;
		}
		mSignal(ev);
		return DefWindowProc(mHwnd, uMsg, wParam, lParam);
	}

	void WindowWin32::SetSize(uint32_t width, uint32_t height)
	{
		mCreateInfo.rect.extent.width = width;
		mCreateInfo.rect.extent.height = height;
		SetWindowPos(mHwnd, HWND_NOTOPMOST, 0, 0, static_cast<int>(width), static_cast<int>(height), static_cast<int>(SWP_NOMOVE));
	}
	void WindowWin32::SetTitle(const wchar_t *title)
	{
		SetWindowText(mHwnd, title);
	}
	void WindowWin32::SetPos(int x, int y)
	{
		mCreateInfo.rect.offset.x = x;
		mCreateInfo.rect.offset.y = y;
		SetWindowPos(mHwnd, HWND_NOTOPMOST, x, y, 0, 0, SWP_NOSIZE);
	}
	void WindowWin32::Close() const
	{
		// PostQuitMessage(0);
		PostMessage(mHwnd, WM_CLOSE, 0, 0);
	}
	bool WindowWin32::PollEvents() const
	{
		MSG msg{};
		// while (GetMessage(&msg, NULL, 0, 0))
		//{
		//	TranslateMessage(&msg);
		//	DispatchMessage(&msg);
		// }
		// while (PeekMessage(&msg, mHwnd, 0, 0, PM_REMOVE))
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
				return false;
		}
		return true;
	}
	void WindowWin32::WaitEvents() const
	{
		WaitMessage();
		PollEvents();
		//		MSG msg{};
		//		BOOL bRet;
		//
		//		if ((bRet = GetMessage(&msg, mHwnd, 0, 0)) != 0)
		//		{
		//			if (bRet == -1)
		//			{
		//				// handle the error and possibly exit
		//				ST_THROW("get meesage error");
		//			}
		//			else
		//			{
		//				TranslateMessage(&msg);
		//				DispatchMessage(&msg);
		//			}
		//		}
	}
	bool WindowWin32::GetWindowSize(uint32_t &width, uint32_t &height) const
	{
		RECT rect{};
		if (::GetWindowRect(mHwnd, &rect))
		{
			width = rect.right - rect.left;
			height = rect.bottom - rect.top;
			return true;
		}
		return false;
	}
	bool WindowWin32::GetFramebufferSize(uint32_t &width, uint32_t &height) const
	{
		RECT rect{};
		if (GetClientRect(mHwnd, &rect))
		{
			width = rect.right - rect.left;
			height = rect.bottom - rect.top;
			return true;
		}
		return false;
	}
	bool WindowWin32::GetCursorPos(int32_t &x, int32_t &y) const
	{
		POINT point;
		if (::GetCursorPos(&point))
		{
			::ScreenToClient(mHwnd, &point);
			x = static_cast<int32_t>(point.x);
			y = static_cast<int32_t>(point.y);
			return true;
		}
		return false;
	}
	void WindowWin32::ShowCursor(bool flag) const
	{
		::ShowCursor(flag);
	}
	void WindowWin32::SetCurorPos(int x, int y)
	{
		POINT pt{x, y};
		::ClientToScreen(mHwnd, &pt);
		::SetCursorPos(pt.x, pt.y);
	}
	// bool WindowWin32::RegisterHotKey(int id, InputModifierBits modifiers, KeyCode keycode)
	//{
	//	return ::RegisterHotKey(mHwnd, id, Map(modifiers), MapKey(keycode));
	// }
	// bool WindowWin32::UnRegisterHotKey(int id)
	//{
	//	return ::UnregisterHotKey(mHwnd, id);
	// }
}

#endif