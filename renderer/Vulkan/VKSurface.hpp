/**
 * @file VKSurface.hpp
 * @author yangzs
 * @brief 
 * @version 0.1
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#pragma once
#include <renderer/ShitSurface.hpp>
#include "VKPrerequisites.hpp"

namespace Shit
{
	class VKSurface final : public Surface
	{
		VkSurfaceKHR mHandle;

	public:
		VKSurface(const SurfaceCreateInfo &createInfo, Window *pWindow);
		~VKSurface() override;
		constexpr VkSurfaceKHR GetHandle() const
		{
			return mHandle;
		}
	};
}