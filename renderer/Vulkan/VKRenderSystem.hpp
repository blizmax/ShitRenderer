/**
 * @file VKRenderSystem.hpp
 * @author yangzs
 * @brief 
 * @version 0.1
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#pragma once
#include <renderer/ShitRenderSystem.hpp>
#include "VKPrerequisites.hpp"
#include "VKSurface.hpp"
#include "VKDevice.hpp"

namespace Shit
{
	class VKRenderSystem final : public RenderSystem
	{
		std::vector<VkLayerProperties> mInstanceLayerProperties;

		//instance extensions, value is version
		std::unordered_map<std::string, uint32_t> mExtensions;

		VkInstance mInstance;

	private:
		bool CheckLayerSupport(const char *layerName);

		void EnumeratePhysicalDevice(std::vector<PhysicalDevice> &physicalDevices) override;

		std::unique_ptr<Surface> CreateSurface(const SurfaceCreateInfo &createInfo, Window *pWindow) override;

		void LoadInstantceExtensionFunctions();

	public:
		VKRenderSystem(const RenderSystemCreateInfo &createInfo);

		~VKRenderSystem() override;

		constexpr VkInstance GetInstance() const { return mInstance; }

		Device *CreateDevice(const DeviceCreateInfo &createInfo) override;

		PFN_vkVoidFunction GetInstanceProcAddr(const char *pName);

		constexpr const std::vector<VkLayerProperties> &GetLayerProperties() const { return mInstanceLayerProperties; }

		constexpr const std::unordered_map<std::string, uint32_t> &GetExtensions() const { return mExtensions; }

		/**
		 * @brief instance extensions only
		 * 
		 * @param name 
		 * @return true 
		 * @return false 
		 */
		bool IsExtensionSupported(std::string_view name) const { return mExtensions.contains(std::string(name)); }
	};
}