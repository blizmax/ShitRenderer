/**
 * @file ShitSwapchain.hpp
 * @author yangzs
 * @brief 
 * @version 0.1
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#pragma once
#include "ShitRendererPrerequisites.hpp"
#include "ShitImage.hpp"

namespace Shit
{
	class Swapchain
	{
	protected:
		SwapchainCreateInfo mCreateInfo;

		std::vector<Image *> mImages;

	public:
		Swapchain(const SwapchainCreateInfo &createInfo) : mCreateInfo(createInfo) {}
		virtual ~Swapchain()
		{
			for (auto e : mImages)
				delete e;
		}
		ST_CONSTEXPR const SwapchainCreateInfo *GetCreateInfoPtr() const
		{
			return &mCreateInfo;
		}
		void GetImages(Image **images) const
		{
			memcpy(images, mImages.data(), sizeof(ptrdiff_t) * mImages.size());
		}
		Image *GetImageByIndex(uint32_t index) const
		{
			return mImages.at(index);
		}
		ST_CONSTEXPR uint32_t GetImageCount() const
		{
			return static_cast<uint32_t>(mImages.size());
		}
		virtual Result GetNextImage(const GetNextImageInfo &info, uint32_t &index) = 0;
	};
} // namespace Shit
