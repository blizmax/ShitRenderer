/**
 * @file ShitRenderer.hpp
 * @author yangzs
 * @brief 
 * @version 0.1
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#pragma once
#include "ShitRendererPrerequisites.hpp"
#include "ShitNonCopyable.hpp"
#include "ShitWindow.hpp"
#include "ShitDevice.hpp"

namespace Shit
{
	class RenderSystem : public NonCopyable
	{
	protected:
		RenderSystemCreateInfo mCreateInfo;

		std::vector<std::unique_ptr<Window>> mWindows;
		std::vector<std::unique_ptr<Device>> mDevices;
		std::vector<std::unique_ptr<Surface>> mSurfaces;

	protected:
		RenderSystem() :mCreateInfo{ RendererVersion::VULKAN_110 } {}

		void DestroyDevice(const Device *pDevice);

		virtual std::unique_ptr<Surface> CreateSurface([[maybe_unused]] const SurfaceCreateInfo &createInfo, [[maybe_unused]] Window *pWindow)
		{
			return std::move(nullptr);
		};

	public:
		RenderSystem(const RenderSystemCreateInfo &createInfo)
			: mCreateInfo(createInfo)
		{
		}
		virtual ~RenderSystem()
		{
		}

		const RenderSystemCreateInfo *GetCreateInfo() const
		{
			return &mCreateInfo;
		}

		Window *CreateRenderWindow(const WindowCreateInfo &createInfo);

		/**
		 * @brief Create a Device object, 
		 * create a device include all queue families supported by the physical device
		 * currently physical device selection is not supported, the method will use the current gpu(opengl) or the best gpu(Vulkan)
		 * 
		 * @param createInfo 
		 * @return Device* 
		 */
		virtual Device *CreateDevice(const DeviceCreateInfo &createInfo) = 0;

		/**
		 * @brief TODO: physical device not finished
		 * 
		 * @param physicalDevices 
		 */
		virtual void EnumeratePhysicalDevice(std::vector<PhysicalDevice> &physicalDevices) = 0;
	};

	RenderSystem *LoadRenderSystem(const RenderSystemCreateInfo &createInfo);
	void DeleteRenderSystem(RenderSystem *pRenderSystem);
} // namespace Shit
