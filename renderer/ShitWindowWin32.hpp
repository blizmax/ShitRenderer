/**
 * @file ShitWin32Window.hpp
 * @author yangzs
 * @brief
 * @version 0.1
 *
 * @copyright Copyright (c) 2021
 *
 */
#pragma once
#ifdef _WIN32
#include "ShitWindow.hpp"

#ifndef UNICODE
#define UNICODE
#endif
#include <Windows.h>

namespace Shit
{

	UINT Map(InputModifierBits modifier);
	void ErrorExit(LPTSTR lpszFunction);

	class WindowWin32 final : public Shit::Window
	{
		HWND mHwnd;
		HINSTANCE mInstance;

	public:
		static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

		WindowWin32(const WindowCreateInfo &createInfo, RenderSystem *pRenderSystem);

		~WindowWin32() override {}

		BOOL Create(
			PCWSTR lpWindowName,
			DWORD dwStyle,
			DWORD dwExStyle = 0,
			int x = CW_USEDEFAULT,
			int y = CW_USEDEFAULT,
			int nWidth = CW_USEDEFAULT,
			int nHeight = CW_USEDEFAULT,
			HWND hWndParent = 0,
			HMENU hMenu = 0);

		virtual PCWSTR ClassName() const { return L"Win32Windwo"; }

		void SetSize(uint32_t width, uint32_t height) override;
		void SetTitle(const wchar_t *title) override;
		void SetPos(int x, int y) override;
		void SetCurorPos(int x, int y) override;
		void ShowCursor(bool flag) const override;

		void Close() const override;
		bool GetWindowSize(uint32_t &width, uint32_t &height) const override;
		bool GetFramebufferSize(uint32_t &width, uint32_t &height) const override;
		bool GetCursorPos(int32_t &x, int32_t &y) const override;

		bool PollEvents() const override;
		void WaitEvents() const override;

		constexpr HWND GetHWND() const { return mHwnd; }
		constexpr HINSTANCE GetInstance() const { return mInstance; }

		// bool RegisterHotKey(int id, InputModifierBits modifiers, KeyCode keycode) override;
		// bool UnRegisterHotKey(int id) override;
	};
}
#endif