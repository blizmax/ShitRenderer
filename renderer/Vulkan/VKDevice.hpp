/**
 * @file VKDevice.hpp
 * @author yangzs
 * @brief
 * @version 0.1
 *
 * @copyright Copyright (c) 2021
 *
 */
#pragma once
#include <renderer/ShitDevice.hpp>
#include "VKPrerequisites.hpp"

namespace Shit
{
	class VKDevice final : public Device
	{
		VkDevice mDevice;

		std::vector<VkQueueFamilyProperties> mQueueFamilyProperties;

		// extension lists, value is version
		std::unordered_map<std::string, uint32_t> mExtensions;

		//CommandPool *mpTransferCommandPool;
		//Queue *mpTransferCommandQueue;
		//CommandBuffer *mpTransferCommandBuffer;

		CommandPool *mpOneTimeCommandPool;
		Queue *mpOneTimeCommandQueue;
		CommandBuffer *mpOneTimeCommandBuffer;

	public:
		VKDevice(const DeviceCreateInfo &createInfo);
		~VKDevice() override;
		constexpr VkDevice GetHandle()
		{
			return mDevice;
		}

		void _ExecuteOneTimeCommand(std::function<void(CommandBuffer *)> func);

		constexpr const std::unordered_map<std::string, uint32_t> &GetExtensions() const { return mExtensions; }

		/**
		 * @brief check device extensions and instance extesions
		 *
		 * @param name
		 * @return true
		 * @return false
		 */
		bool IsExtensionSupported(std::string_view name) const;

		PFN_vkVoidFunction GetDeviceProcAddr(const char *pName);

		void LoadDeviceExtensionFunctions();

		Result WaitIdle() const override;

		std::optional<QueueFamilyProperty> GetPresentQueueFamilyProperty(Window const *pWindow) const override;

		constexpr VkPhysicalDevice GetPhysicalDevice() const
		{
			return static_cast<VkPhysicalDevice>(std::get<PhysicalDevice>(mCreateInfo.physicalDevice));
		}

		std::optional<QueueFamilyProperty> GetQueueFamilyProperty(QueueFlagBits flag) const override;

		void GetWindowPixelFormats(const Window *pWindow, std::vector<WindowPixelFormat> &format) const override;

		void GetPresentModes(const Window *pWindow, std::vector<PresentMode> &presentModes) const override;

		Format GetSuitableImageFormat(std::span<const Format> candidates, ImageTiling tiling, FormatFeatureFlagBits flags) const override;

		void GetSupportedShaderSourceLanguages(std::vector<ShadingLanguage> &shadingLanguage) const override;

		Swapchain *Create(const SwapchainCreateInfo &createInfo) override;

		Shader *Create(const ShaderCreateInfo &createInfo) override;

		Pipeline *Create(const GraphicsPipelineCreateInfo &createInfo) override;

		Pipeline *Create(const ComputePipelineCreateInfo &createInfo) override;

		CommandPool *Create(const CommandPoolCreateInfo &createInfo) override;

		Buffer *Create(const BufferCreateInfo &createInfo, const void *pData) override;

		Buffer *Create(const BufferCreateInfo &createInfo, int val) override;

		BufferView *Create(const BufferViewCreateInfo &createInfo) override;

		Image *Create(const ImageCreateInfo &createInfo, ImageAspectFlagBits aspectMask, const void *pData) override;

		Image *Create(const ImageCreateInfo &createInfo, ImageAspectFlagBits aspectMask, int val) override;

		ImageView *Create(const ImageViewCreateInfo &createInfo) override;

		DescriptorSetLayout *Create(const DescriptorSetLayoutCreateInfo &createInfo) override;

		PipelineLayout *Create(const PipelineLayoutCreateInfo &createInfo) override;

		RenderPass *Create(const RenderPassCreateInfo &createInfo) override;

		Framebuffer *Create(const FramebufferCreateInfo &createInfo) override;

		Semaphore *Create(const SemaphoreCreateInfo &createInfo) override;

		Fence *Create(const FenceCreateInfo &createInfo) override;

		Sampler *Create(const SamplerCreateInfo &createInfo) override;

		DescriptorPool *Create(const DescriptorPoolCreateInfo &createInfo) override;

		void UpdateDescriptorSets(std::span<const WriteDescriptorSet> descriptorWrites,
								  std::span<const CopyDescriptorSet> descriptorCopies) override;

		void UpdateDescriptorSets(
			uint32_t descriptorWriteCount,
			const WriteDescriptorSet *descriptorWrites,
			uint32_t descriptorCopyCount,
			const CopyDescriptorSet *descriptorCopies) override;

		void FlushMappedMemoryRanges(std::span<const MappedMemoryRange> ranges) override;

		void InvalidateMappedMemoryRanges(std::span<const MappedMemoryRange> ranges) override;
	};

} // namespace Shit
