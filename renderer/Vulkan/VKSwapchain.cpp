/**
 * @file VKSwapchain.cpp
 * @author yangzs
 * @brief
 * @version 0.1
 *
 * @copyright Copyright (c) 2021
 *
 */
#include <renderer/ShitWindow.hpp>
#include "VKSwapchain.hpp"
#include "VKSemaphore.hpp"
#include "VKFence.hpp"
#include "VKDevice.hpp"
#include "VKSurface.hpp"
#include "VKImage.hpp"

namespace Shit
{
	VKSwapchain::~VKSwapchain()
	{
		vkDestroySwapchainKHR(mpDevice->GetHandle(), mHandle, nullptr);
	}
	VKSwapchain::VKSwapchain(VKDevice *pDevice, const SwapchainCreateInfo &createInfo)
		: Swapchain(createInfo), VKDeviceObject(pDevice)
	{
		VkSurfaceKHR surface = static_cast<const VKSurface *>(mCreateInfo.pWindow->GetSurfacePtr())->GetHandle();

		auto presentQueueFamilyProperty = pDevice->GetPresentQueueFamilyProperty(mCreateInfo.pWindow);

		if (!presentQueueFamilyProperty.has_value())
			ST_THROW("current device do not support present to surface");

		// set format
		std::vector<VkSurfaceFormatKHR> surfaceFormats;
		VK::querySurfaceFormats(mpDevice->GetPhysicalDevice(), surface, surfaceFormats);

		VkSurfaceFormatKHR surfaceFormat = surfaceFormats[0];

		VkFormat format = Map(createInfo.format);
		VkColorSpaceKHR colorSpace = Map(createInfo.colorSpace);
		// change to srgba8
		for (auto &&e : surfaceFormats)
		{
			if (e.format == format && e.colorSpace == colorSpace)
			{
				surfaceFormat = e;
				break;
			}
			ST_LOG("swapchain format do not support:", format);
		}
		ST_LOG_VAR(surfaceFormat.format);
		ST_LOG_VAR(surfaceFormat.colorSpace);

		std::vector<VkPresentModeKHR> presentModes;
		VK::querySurfacePresentModes(mpDevice->GetPhysicalDevice(), surface, presentModes);
		VkPresentModeKHR presentMode{VK_PRESENT_MODE_IMMEDIATE_KHR};
		auto dstmode = Map(createInfo.presentMode);
		for (auto &&e : presentModes)
		{
			if (dstmode == e)
			{
				presentMode = e;
				break;
			}
		}

		VkSwapchainCreateInfoKHR swapchainInfo{
			VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			NULL,
			0,
			surface,
			createInfo.minImageCount,
			surfaceFormat.format,
			surfaceFormat.colorSpace,
			{createInfo.imageExtent.width,
			 createInfo.imageExtent.height},
			1,
			Map(createInfo.imageUsage),
			VK_SHARING_MODE_EXCLUSIVE,
			0,
			nullptr,
			VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			presentMode,
			VK_TRUE,
			VK_NULL_HANDLE};

		CHECK_VK_RESULT(vkCreateSwapchainKHR(mpDevice->GetHandle(), &swapchainInfo, nullptr, &mHandle));

		uint32_t swapchainImageCount;
		vkGetSwapchainImagesKHR(mpDevice->GetHandle(), mHandle, &swapchainImageCount, nullptr);
		ST_LOG_VAR(swapchainImageCount);
		std::vector<VkImage> swapchainImages;
		swapchainImages.resize(swapchainImageCount);
		vkGetSwapchainImagesKHR(mpDevice->GetHandle(), mHandle, &swapchainImageCount, swapchainImages.data());

		ImageCreateInfo imageCreateInfo{
			{},
			ImageType::TYPE_2D,
			createInfo.format,
			{createInfo.imageExtent.width,
			 createInfo.imageExtent.height, 1},
			1,
			1,
			SampleCountFlagBits::BIT_1,
			ImageTiling::OPTIMAL,
			createInfo.imageUsage,
			MemoryPropertyFlagBits::DEVICE_LOCAL_BIT,
			ImageLayout::PRESENT_SRC};
		for (auto e : swapchainImages)
		{
			mImages.emplace_back(new VKImage(mpDevice, imageCreateInfo, e, true));
		}
	}
	Result VKSwapchain::GetNextImage(const GetNextImageInfo &info, uint32_t &index)
	{
		return Map(vkAcquireNextImageKHR(
			mpDevice->GetHandle(), mHandle, info.timeout,
			info.pSemaphore ? static_cast<VKSemaphore const *>(info.pSemaphore)->GetHandle() : VK_NULL_HANDLE,
			info.pFence ? static_cast<VKFence const *>(info.pFence)->GetHandle() : VK_NULL_HANDLE,
			&index));
	}
} // namespace Shit
