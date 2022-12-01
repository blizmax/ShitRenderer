/**
 * @file VKSurface.cpp
 * @author yangzs
 * @brief 
 * @version 0.1
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "VKSurface.hpp"
#include "VKRenderSystem.hpp"

#ifdef _WIN32
#include <renderer/ShitWindowWin32.hpp>
#endif

namespace Shit
{
	VKSurface::VKSurface(const SurfaceCreateInfo &createInfo, Window *pWindow) : Surface(createInfo)
	{
#ifdef _WIN32
		VkWin32SurfaceCreateInfoKHR info{
			VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
			nullptr,
			0,
			static_cast<WindowWin32 *>(pWindow)->GetInstance(),
			static_cast<WindowWin32 *>(pWindow)->GetHWND(),
		};
#else
		static_assert(0, "there is no VK surface implementation");
#endif
		if (vkCreateWin32SurfaceKHR(g_RenderSystem->GetInstance(), &info, nullptr, &mHandle) != VK_SUCCESS)
			ST_THROW("failed to create VK surface");
	}
	VKSurface::~VKSurface()
	{
		vkDestroySurfaceKHR(g_RenderSystem->GetInstance(), mHandle, nullptr);
	}
}