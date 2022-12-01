/**
 * @file VKSwapchain.hpp
 * @author yangzs
 * @brief 
 * @version 0.1
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#pragma once
#include <renderer/ShitSwapchain.hpp>
#include "VKPrerequisites.hpp"
#include "VKDeviceObject.hpp"

namespace Shit
{
	class VKSwapchain final : public Swapchain, public VKDeviceObject
	{
		VkSwapchainKHR mHandle;
		VkPresentModeKHR mPresentMode{};
		VkSurfaceFormatKHR mSurfaceFormat;

	public:
		VKSwapchain(VKDevice *pDevice, const SwapchainCreateInfo &createInfo);

		constexpr VkSurfaceFormatKHR GetSurfaceFormat() const
		{
			return mSurfaceFormat;
		}
		constexpr VkSwapchainKHR GetHandle() const
		{
			return mHandle;
		}
		constexpr VkPresentModeKHR GetPresentMode() const
		{
			return mPresentMode;
		}
		~VKSwapchain() override;

		Result GetNextImage(const GetNextImageInfo &info, uint32_t &index) override;
	};

}
