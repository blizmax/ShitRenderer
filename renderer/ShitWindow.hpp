/**
 * @file Window.hpp
 * @author yangzs
 * @brief TODO: remove this module out of renderer
 * @version 0.1
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#pragma once
#include "ShitRendererPrerequisites.hpp"
#include "ShitInput.hpp"
#include "ShitListener.hpp"

namespace Shit
{
	class Window
	{
	protected:
		friend class RenderSystem;

		WindowCreateInfo mCreateInfo;
		//Listener<void(const Event &)> mListener;
		Signal<void(const Event &)> mSignal{};

		mutable Surface *mpSurface{};
		mutable Swapchain *mpSwapchain{};
		RenderSystem *mpRenderSystem;

		void SetSurface(Surface *pSurface)
		{
			mpSurface = pSurface;
		}

	public:
		Window(const WindowCreateInfo &createInfo, RenderSystem *pRenderSystem) : mCreateInfo(createInfo), mpRenderSystem(pRenderSystem)
		{
			if (createInfo.callBackFunction)
				AddEventListener(createInfo.callBackFunction);
		}
		const WindowCreateInfo *GetCreateInfo()
		{
			return &mCreateInfo;
		}
		virtual ~Window() {}

		//virtual bool RegisterHotKey(int id, InputModifierBits modifiers, KeyCode keycode);
		//virtual bool UnRegisterHotKey(int id);

		virtual void SetSize(uint32_t width, uint32_t height) = 0;
		virtual void SetTitle(const wchar_t *title) = 0;
		virtual void SetPos(int x, int y) = 0;
		virtual void SetCurorPos(int x, int y) = 0;

		virtual bool PollEvents() const = 0;
		virtual void WaitEvents() const = 0;

		virtual void Close() const = 0;
		virtual bool GetWindowSize(uint32_t &width, uint32_t &height) const = 0;
		virtual bool GetFramebufferSize(uint32_t &width, uint32_t &height) const = 0;
		virtual void ShowCursor(bool flag) const = 0;
		virtual bool GetCursorPos(int32_t &x, int32_t &y) const = 0;

		Shit::Connection AddEventListener(Slot<void(const Event &)> eventListener)
		{
			return mSignal.Connect(eventListener);
		}

		void SetSwapchain(Swapchain *pSwapchain) const
		{
			mpSwapchain = pSwapchain;
		}
		constexpr const Surface *GetSurfacePtr() const
		{
			return mpSurface;
		}
		constexpr const Swapchain *GetSwapchainPtr() const
		{
			return mpSwapchain;
		}
	};
}