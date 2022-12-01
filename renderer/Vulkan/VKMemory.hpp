/**
 * @file VKMemory.hpp
 * @author yangzs
 * @brief 
 * @version 0.1
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#pragma once
#include <renderer/ShitMemory.hpp>
#include "VKPrerequisites.hpp"
#include "VKDeviceObject.hpp"

namespace Shit
{
	class VKMemory final : public Memory, public VKDeviceObject
	{
		VkDeviceMemory mHandle;

	public:
		VKMemory(VKDevice *device, const MemoryAllocateInfo &allocateInfo);
		~VKMemory() override;
	};
}
