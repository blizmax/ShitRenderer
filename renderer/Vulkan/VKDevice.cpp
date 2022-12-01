/**
 * @file VKDevice.cpp
 * @author yangzs
 * @brief
 * @version 0.1
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "VKDevice.hpp"
#include <renderer/ShitWindow.hpp>
#include "VKSwapchain.hpp"
#include "VKShader.hpp"
#include "VKCommandPool.hpp"
#include "VKCommandBuffer.hpp"
#include "VKDevice.hpp"
#include "VKQueue.hpp"
#include "VKBuffer.hpp"
#include "VKBufferView.hpp"
#include "VKImage.hpp"
#include "VKDescriptor.hpp"
#include "VKSampler.hpp"
#include "VKPipeline.hpp"
#include "VKRenderPass.hpp"
#include "VKFramebuffer.hpp"
#include "VKSurface.hpp"
#include "VKSemaphore.hpp"
#include "VKDevice.hpp"
#include "VKFence.hpp"
#include "VKRenderSystem.hpp"

namespace Shit
{
	PFN_vkVoidFunction VKDevice::GetDeviceProcAddr(const char *pName)
	{
		return vkGetDeviceProcAddr(mDevice, pName);
	}
	void VKDevice::LoadDeviceExtensionFunctions()
	{
	}
	Result VKDevice::WaitIdle() const
	{
		if (vkDeviceWaitIdle(mDevice) == VK_SUCCESS)
			return Result::SUCCESS;
		return Result::SHIT_ERROR;
	}
	VKDevice::~VKDevice()
	{
		// vkDestroyDevice(mDevice, nullptr);
	}
	bool VKDevice::IsExtensionSupported(std::string_view name) const
	{
		return mExtensions.contains(std::string(name)) || g_RenderSystem->IsExtensionSupported(name);
	}
	VKDevice::VKDevice(const DeviceCreateInfo &createInfo) : Device(createInfo)
	{
		VkPhysicalDevice physicalDevice = GetPhysicalDevice();

		VK::queryQueueFamilyProperties(physicalDevice, mQueueFamilyProperties);

		std::vector<VkExtensionProperties> properties;
		VK::queryDeviceExtensionProperties(physicalDevice, properties);
		std::vector<const char *> extensionNames;
		ST_LOG("=========device extensions=================");
		for (auto &extensionProperty : properties)
		{
			ST_LOG("extension name: ", extensionProperty.extensionName, "version: ", extensionProperty.specVersion);
			if (strlen(extensionProperty.extensionName) == 0)
				continue;
			mExtensions.emplace(extensionProperty.extensionName, extensionProperty.specVersion);
			extensionNames.emplace_back(extensionProperty.extensionName);
		}

		// TODO: set physical device  features, put this outside
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

#ifndef NDEBUG
		ST_LOG("=========device features=============");
		ST_LOG("robustBufferAccess:", deviceFeatures.robustBufferAccess);
		ST_LOG("fullDrawIndexUint32:", deviceFeatures.fullDrawIndexUint32);
		ST_LOG("imageCubeArray:", deviceFeatures.imageCubeArray);
		ST_LOG("independentBlend:", deviceFeatures.independentBlend);
		ST_LOG("geometryShader:", deviceFeatures.geometryShader);
		ST_LOG("tessellationShader:", deviceFeatures.tessellationShader);
		ST_LOG("sampleRateShading:", deviceFeatures.sampleRateShading);
		ST_LOG("dualSrcBlend:", deviceFeatures.dualSrcBlend);
		ST_LOG("logicOp:", deviceFeatures.logicOp);
		ST_LOG("multiDrawIndirect:", deviceFeatures.multiDrawIndirect);
		ST_LOG("drawIndirectFirstInstance:", deviceFeatures.drawIndirectFirstInstance);
		ST_LOG("depthClamp:", deviceFeatures.depthClamp);
		ST_LOG("depthBiasClamp:", deviceFeatures.depthBiasClamp);
		ST_LOG("fillModeNonSolid:", deviceFeatures.fillModeNonSolid);
		ST_LOG("depthBounds:", deviceFeatures.depthBounds);
		ST_LOG("wideLines:", deviceFeatures.wideLines);
		ST_LOG("largePoints:", deviceFeatures.largePoints);
		ST_LOG("alphaToOne:", deviceFeatures.alphaToOne);
		ST_LOG("multiViewport:", deviceFeatures.multiViewport);
		ST_LOG("samplerAnisotropy:", deviceFeatures.samplerAnisotropy);
		ST_LOG("textureCompressionETC2:", deviceFeatures.textureCompressionETC2);
		ST_LOG("textureCompressionASTC_LDR:", deviceFeatures.textureCompressionASTC_LDR);
		ST_LOG("textureCompressionBC:", deviceFeatures.textureCompressionBC);
		ST_LOG("occlusionQueryPrecise:", deviceFeatures.occlusionQueryPrecise);
		ST_LOG("pipelineStatisticsQuery:", deviceFeatures.pipelineStatisticsQuery);
		ST_LOG("vertexPipelineStoresAndAtomics:", deviceFeatures.vertexPipelineStoresAndAtomics);
		ST_LOG("fragmentStoresAndAtomics:", deviceFeatures.fragmentStoresAndAtomics);
		ST_LOG("shaderTessellationAndGeometryPointSize:", deviceFeatures.shaderTessellationAndGeometryPointSize);
		ST_LOG("shaderImageGatherExtended:", deviceFeatures.shaderImageGatherExtended);
		ST_LOG("shaderStorageImageExtendedFormats:", deviceFeatures.shaderStorageImageExtendedFormats);
		ST_LOG("shaderStorageImageMultisample:", deviceFeatures.shaderStorageImageMultisample);
		ST_LOG("shaderStorageImageReadWithoutFormat:", deviceFeatures.shaderStorageImageReadWithoutFormat);
		ST_LOG("shaderStorageImageWriteWithoutFormat:", deviceFeatures.shaderStorageImageWriteWithoutFormat);
		ST_LOG("shaderUniformBufferArrayDynamicIndexing:", deviceFeatures.shaderUniformBufferArrayDynamicIndexing);
		ST_LOG("shaderSampledImageArrayDynamicIndexing:", deviceFeatures.shaderSampledImageArrayDynamicIndexing);
		ST_LOG("shaderStorageBufferArrayDynamicIndexing:", deviceFeatures.shaderStorageBufferArrayDynamicIndexing);
		ST_LOG("shaderStorageImageArrayDynamicIndexing:", deviceFeatures.shaderStorageImageArrayDynamicIndexing);
		ST_LOG("shaderClipDistance:", deviceFeatures.shaderClipDistance);
		ST_LOG("shaderCullDistance:", deviceFeatures.shaderCullDistance);
		ST_LOG("shaderFloat64:", deviceFeatures.shaderFloat64);
		ST_LOG("shaderInt64:", deviceFeatures.shaderInt64);
		ST_LOG("shaderInt16:", deviceFeatures.shaderInt16);
		ST_LOG("shaderResourceResidency:", deviceFeatures.shaderResourceResidency);
		ST_LOG("shaderResourceMinLod:", deviceFeatures.shaderResourceMinLod);
		ST_LOG("sparseBinding:", deviceFeatures.sparseBinding);
		ST_LOG("sparseResidencyBuffer:", deviceFeatures.sparseResidencyBuffer);
		ST_LOG("sparseResidencyImage2D:", deviceFeatures.sparseResidencyImage2D);
		ST_LOG("sparseResidencyImage3D:", deviceFeatures.sparseResidencyImage3D);
		ST_LOG("sparseResidency2Samples:", deviceFeatures.sparseResidency2Samples);
		ST_LOG("sparseResidency4Samples:", deviceFeatures.sparseResidency4Samples);
		ST_LOG("sparseResidency8Samples:", deviceFeatures.sparseResidency8Samples);
		ST_LOG("sparseResidency16Samples:", deviceFeatures.sparseResidency16Samples);
		ST_LOG("sparseResidencyAliased:", deviceFeatures.sparseResidencyAliased);
		ST_LOG("variableMultisampleRate:", deviceFeatures.variableMultisampleRate);
		ST_LOG("inheritedQueries:", deviceFeatures.inheritedQueries);
		ST_LOG("=========device features end=============");
#endif

		std::vector<VkDeviceQueueCreateInfo> queueInfos;
		std::vector<std::vector<float>> queuePriorities(mQueueFamilyProperties.size());
		for (uint32_t i = 0, len = static_cast<uint32_t>(mQueueFamilyProperties.size()); i < len; ++i)
		{
			// TODO: how to arrange queue priorities
			queuePriorities[i].resize(mQueueFamilyProperties[i].queueCount, 1.f);
			queueInfos.emplace_back(
				VkDeviceQueueCreateInfo{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
										NULL,
										0,
										i,									  // queue family index
										mQueueFamilyProperties[i].queueCount, // queue count
										queuePriorities[i].data()});
		}

		VkDeviceCreateInfo deviceInfo{
			VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			NULL,
			0,
			static_cast<uint32_t>(queueInfos.size()),
			queueInfos.data(),
			0, // deprecated
			0, // deprecated
			static_cast<uint32_t>(extensionNames.size()),
			extensionNames.data(),
			&deviceFeatures};
		CHECK_VK_RESULT(vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &mDevice))

		// create queues
		mQueues.resize(queueInfos.size());
		for (uint32_t i = 0; i < queueInfos.size(); ++i)
		{
			mQueues[i].resize(queueInfos[i].queueCount);
			for (uint32_t j = 0; j < queueInfos[i].queueCount; ++j)
			{
				mQueues[i][j] = std::make_unique<VKQueue>(this, i, j, queueInfos[i].pQueuePriorities[j]);
			}
		}

		// create default Commandpool
		auto oneTimeQueueFamilyProperty = GetQueueFamilyProperty(
			QueueFlagBits::GRAPHICS_BIT | QueueFlagBits::COMPUTE_BIT | QueueFlagBits::TRANSFER_BIT);
		mpOneTimeCommandPool = Create(
			CommandPoolCreateInfo{
				CommandPoolCreateFlagBits::RESET_COMMAND_BUFFER_BIT,
				oneTimeQueueFamilyProperty->index});

		mpOneTimeCommandQueue = GetQueue(oneTimeQueueFamilyProperty->index, 0);
		mpOneTimeCommandPool->CreateCommandBuffers(
			CommandBufferCreateInfo{CommandBufferLevel::PRIMARY, 1}, &mpOneTimeCommandBuffer);
	}
	void VKDevice::_ExecuteOneTimeCommand(std::function<void(CommandBuffer *)> func)
	{
		static auto pFence = Create(Shit::FenceCreateInfo{});
		pFence->Reset();
		mpOneTimeCommandBuffer->Begin(
			CommandBufferBeginInfo{CommandBufferUsageFlagBits::ONE_TIME_SUBMIT_BIT});
		func(mpOneTimeCommandBuffer);
		mpOneTimeCommandBuffer->End();
		SubmitInfo submitInfo[]{{0, 0, 1, &mpOneTimeCommandBuffer}};
		mpOneTimeCommandQueue->Submit(submitInfo, pFence);
		pFence->WaitFor(UINT64_MAX);
	}
	std::optional<QueueFamilyProperty> VKDevice::GetPresentQueueFamilyProperty(Window const *pWindow) const
	{
		auto index = VK::findQueueFamilyIndexPresent(
			GetPhysicalDevice(),
			static_cast<uint32_t>(mQueueFamilyProperties.size()),
			static_cast<const VKSurface *>(pWindow->GetSurfacePtr())->GetHandle());
		if (index.has_value())
			return std::optional<QueueFamilyProperty>{
				{Map(mQueueFamilyProperties[*index].queueFlags),
				 *index,
				 mQueueFamilyProperties[*index].queueCount}};
		else
			return std::nullopt;
	}

	std::optional<QueueFamilyProperty> VKDevice::GetQueueFamilyProperty(QueueFlagBits flag) const
	{
		auto index = VK::findQueueFamilyIndexByFlag(mQueueFamilyProperties, Map(flag));
		if (index.has_value())
			return std::optional<QueueFamilyProperty>{
				{Map(mQueueFamilyProperties[*index].queueFlags),
				 index.value(),
				 mQueueFamilyProperties.at(static_cast<size_t>(index.value())).queueCount}};
		return {};
	}
	void VKDevice::GetWindowPixelFormats(const Window *pWindow, std::vector<WindowPixelFormat> &formats) const
	{
		auto surface = static_cast<const VKSurface *>(pWindow->GetSurfacePtr())->GetHandle();
		std::vector<VkSurfaceFormatKHR> surfaceFormats;
		VK::querySurfaceFormats(GetPhysicalDevice(), surface, surfaceFormats);
		formats.resize(surfaceFormats.size());
		static auto v = std::views::transform(
			[](const VkSurfaceFormatKHR &e)
			{ return WindowPixelFormat{Map(e.format), Map(e.colorSpace)}; });
		std::ranges::copy(surfaceFormats | v, std::back_inserter(formats));
	}
	void VKDevice::GetPresentModes(const Window *pWindow, std::vector<PresentMode> &presentModes) const
	{
		auto surface = static_cast<const VKSurface *>(pWindow->GetSurfacePtr())->GetHandle();
		std::vector<VkPresentModeKHR> modes;
		VK::querySurfacePresentModes(GetPhysicalDevice(), surface, modes);
		static auto v = std::views::transform(
			[](const VkPresentModeKHR &e)
			{ return Map(e); });
		std::ranges::copy(modes | v, std::back_inserter(presentModes));

#ifndef NDEBUG
		static const char *presentModeNames[]{
			"IMMEDIATE",
			"MAILBOX",
			"FIFO",
			"FIFO_RELAXED",
			"SHARED_DEMAND_REFRESH",
			"SHARED_CONTINUOUS_REFRESH",
		};
		ST_LOG("supported present mode:");
		for (auto &&e : presentModes)
			ST_LOG(static_cast<size_t>(e), presentModeNames[static_cast<size_t>(e)]);
#endif
	}
	Format VKDevice::GetSuitableImageFormat(std::span<const Format> candidates, ImageTiling tiling, FormatFeatureFlagBits flags) const
	{
		auto featureFlags = Map(flags);
		VkFormatProperties formatProperties;
		for (auto &&e : candidates)
		{
			vkGetPhysicalDeviceFormatProperties(GetPhysicalDevice(), Map(e), &formatProperties);
			ST_LOG("formatProperties.linearTilingFeatures", formatProperties.linearTilingFeatures);
			ST_LOG("formatProperties.optimalTilingFeatures", formatProperties.optimalTilingFeatures);
			ST_LOG("formatProperties.bufferFeatures", formatProperties.bufferFeatures);
			if (tiling == ImageTiling::LINEAR)
			{
				if (formatProperties.linearTilingFeatures & featureFlags)
					return e;
			}
			else
			{
				if (formatProperties.optimalTilingFeatures & featureFlags)
					return e;
			}
		}
		ST_LOG("failed to find supported format");
		exit(-1);
	}
	void VKDevice::GetSupportedShaderSourceLanguages(std::vector<ShadingLanguage> &shadingLanguage) const
	{
		shadingLanguage = {ShadingLanguage::SPIRV};
	}
	Swapchain *VKDevice::Create(const SwapchainCreateInfo &createInfo)
	{
		mSwapchains.emplace_back(std::make_unique<VKSwapchain>(this, createInfo));
		createInfo.pWindow->SetSwapchain(mSwapchains.back().get());
		return mSwapchains.back().get();
	}

	Shader *VKDevice::Create(const ShaderCreateInfo &createInfo)
	{
		return mShaders.emplace_back(std::make_unique<VKShader>(static_cast<VKDevice *>(this), createInfo)).get();
	}
	Pipeline *VKDevice::Create(const GraphicsPipelineCreateInfo &createInfo)
	{
		return mPipelines.emplace_back(std::make_unique<VKGraphicsPipeline>(static_cast<VKDevice *>(this), createInfo)).get();
	}
	Pipeline *VKDevice::Create(const ComputePipelineCreateInfo &createInfo)
	{
		return mPipelines.emplace_back(std::make_unique<VKComputePipeline>(static_cast<VKDevice *>(this), createInfo)).get();
	}
	CommandPool *VKDevice::Create(const CommandPoolCreateInfo &createInfo)
	{
		return mCommandPools.emplace_back(std::make_unique<VKCommandPool>(static_cast<VKDevice *>(this), createInfo)).get();
	}
	Buffer *VKDevice::Create(const BufferCreateInfo &createInfo, const void *pData)
	{
		auto ret = mBuffers.emplace_back(std::make_unique<VKBuffer>(static_cast<VKDevice *>(this), GetPhysicalDevice(), createInfo)).get();
		if (pData)
			ret->UpdateSubData(0, createInfo.size, pData);
		return ret;
	}
	Buffer *VKDevice::Create(const BufferCreateInfo &createInfo, int val)
	{
		auto ret = mBuffers.emplace_back(std::make_unique<VKBuffer>(static_cast<VKDevice *>(this), GetPhysicalDevice(), createInfo)).get();
		ret->UpdateSubData(0, createInfo.size, val);
		return ret;
	}
	BufferView *VKDevice::Create(const BufferViewCreateInfo &createInfo)
	{
		return mBufferViews.emplace_back(std::make_unique<VKBufferView>(static_cast<VKDevice *>(this), createInfo)).get();
	}
	Image *VKDevice::Create(const ImageCreateInfo &createInfo, ImageAspectFlagBits aspectMask, const void *pData)
	{
		return mImages.emplace_back(std::make_unique<VKImage>(this, createInfo, aspectMask, pData)).get();
	}
	Image *VKDevice::Create(const ImageCreateInfo &createInfo, ImageAspectFlagBits aspectMask, int val)
	{
		return mImages.emplace_back(std::make_unique<VKImage>(this, createInfo, aspectMask, val)).get();
	}
	ImageView *VKDevice::Create(const ImageViewCreateInfo &createInfo)
	{
		return mImageViews.emplace_back(std::make_unique<VKImageView>(static_cast<VKDevice *>(this), createInfo)).get();
	}
	DescriptorSetLayout *VKDevice::Create(const DescriptorSetLayoutCreateInfo &createInfo)
	{
		return mDescriptorSetLayouts.emplace_back(std::make_unique<VKDescriptorSetLayout>(static_cast<VKDevice *>(this), createInfo)).get();
	}
	PipelineLayout *VKDevice::Create(const PipelineLayoutCreateInfo &createInfo)
	{
		return mPipelineLayouts.emplace_back(std::make_unique<VKPipelineLayout>(static_cast<VKDevice *>(this), createInfo)).get();
	}
	RenderPass *VKDevice::Create(const RenderPassCreateInfo &createInfo)
	{
		return mRenderPasses.emplace_back(std::make_unique<VKRenderPass>(static_cast<VKDevice *>(this), createInfo)).get();
	}
	Framebuffer *VKDevice::Create(const FramebufferCreateInfo &createInfo)
	{
		return mFramebuffers.emplace_back(std::make_unique<VKFramebuffer>(static_cast<VKDevice *>(this), createInfo)).get();
	}
	Semaphore *VKDevice::Create(const SemaphoreCreateInfo &createInfo)
	{
		return mSemaphores.emplace_back(std::make_unique<VKSemaphore>(static_cast<VKDevice *>(this), createInfo)).get();
	}
	Fence *VKDevice::Create(const FenceCreateInfo &createInfo)
	{
		return mFences.emplace_back(std::make_unique<VKFence>(static_cast<VKDevice *>(this), createInfo)).get();
	}
	Sampler *VKDevice::Create(const SamplerCreateInfo &createInfo)
	{
		return mSamplers.emplace_back(std::make_unique<VKSampler>(static_cast<VKDevice *>(this), createInfo)).get();
	}
	DescriptorPool *VKDevice::Create(const DescriptorPoolCreateInfo &createInfo)
	{
		return mDescriptorPools.emplace_back(std::make_unique<VKDescriptorPool>(static_cast<VKDevice *>(this), createInfo)).get();
	}
	void VKDevice::UpdateDescriptorSets(
		std::span<const WriteDescriptorSet> descriptorWrites,
		std::span<const CopyDescriptorSet> descriptorCopies)
	{
		UpdateDescriptorSets(
			static_cast<uint32_t>(descriptorWrites.size()),
			descriptorWrites.data(),
			static_cast<uint32_t>(descriptorCopies.size()),
			descriptorCopies.data());
	}
	void VKDevice::UpdateDescriptorSets(
		uint32_t descriptorWriteCount,
		const WriteDescriptorSet *descriptorWrites,
		uint32_t descriptorCopyCount,
		const CopyDescriptorSet *descriptorCopies)
	{
		if (descriptorWriteCount == 0 && descriptorCopyCount == 0)
			return;
		struct Temp
		{
			std::vector<VkDescriptorImageInfo> imagesInfo;
			std::vector<VkDescriptorBufferInfo> buffersInfo;
			std::vector<VkBufferView> texelBufferViews;
		};

		// auto count = descriptorWrites.size();
		std::vector<VkWriteDescriptorSet> writes(descriptorWriteCount);
		std::vector<Temp> temp(descriptorWriteCount);

		for (uint32_t i = 0, j; i < descriptorWriteCount; ++i)
		{
			auto &&e = descriptorWrites[i];
			writes[i] = {
				VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				nullptr,
				static_cast<VKDescriptorSet const *>(e.pDstSet)->GetHandle(),
				e.dstBinding,
				e.dstArrayElement,
				e.descriptorCount,
				Map(e.descriptorType),
			};

			switch (e.descriptorType)
			{
			case DescriptorType::SAMPLER: // sampler (vulkan)
				ST_FALLTHROUGH;
			case DescriptorType::COMBINED_IMAGE_SAMPLER: // sampler2D
				ST_FALLTHROUGH;
			case DescriptorType::SAMPLED_IMAGE: // texture2D (vulkan)
				ST_FALLTHROUGH;
			case DescriptorType::STORAGE_IMAGE: // image2D
				ST_FALLTHROUGH;
			case DescriptorType::INPUT_ATTACHMENT:
			{
				for (j = 0; j < e.descriptorCount; ++j)
				{
					temp[i].imagesInfo.emplace_back(
						e.pImageInfo[j].pSampler ? static_cast<VKSampler const *>(e.pImageInfo[j].pSampler)->GetHandle() : VK_NULL_HANDLE,
						e.pImageInfo[j].pImageView ? static_cast<VKImageView const *>(e.pImageInfo[j].pImageView)->GetHandle() : VK_NULL_HANDLE,
						Map(e.pImageInfo[j].imageLayout));
				}
				writes[i].pImageInfo = temp[i].imagesInfo.data();
				break;
			}
			case DescriptorType::UNIFORM_TEXEL_BUFFER: // samplerbuffer	(access to buffer texture,can only be accessed with texelFetch function) ,textureBuffer(vulkan)
				ST_FALLTHROUGH;
			case DescriptorType::STORAGE_TEXEL_BUFFER: // imagebuffer (access to buffer texture)
			{
				for (j = 0; j < e.descriptorCount; ++j)
				{
					temp[i].texelBufferViews.emplace_back(static_cast<const VKBufferView *>(e.pTexelBufferView[j])->GetHandle());
				}
				writes[i].pTexelBufferView = temp[i].texelBufferViews.data();
				break;
			}
			case DescriptorType::UNIFORM_BUFFER: // uniform block
			case DescriptorType::STORAGE_BUFFER: // buffer block
			case DescriptorType::UNIFORM_BUFFER_DYNAMIC:
			case DescriptorType::STORAGE_BUFFER_DYNAMIC:
			{
				for (j = 0; j < e.descriptorCount; ++j)
				{
					auto &&buffer = e.pBufferInfo[j];
					temp[i].buffersInfo.emplace_back(
						buffer.pBuffer ? static_cast<VKBuffer const *>(buffer.pBuffer)->GetHandle() : VK_NULL_HANDLE,
						buffer.offset,
						buffer.range);
				}
				writes[i].pBufferInfo = temp[i].buffersInfo.data();
				break;
			}
			}
		}
		std::vector<VkCopyDescriptorSet> copies;
		copies.reserve(descriptorCopyCount);
		for (uint32_t i = 0; i < descriptorCopyCount; ++i)
		{
			auto &&e = descriptorCopies[i];
			copies.emplace_back(
				VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET,
				nullptr,
				static_cast<VKDescriptorSet const *>(e.pSrcSet)->GetHandle(),
				e.srcBinding,
				e.srcArrayElement,
				static_cast<VKDescriptorSet const *>(e.pDstSet)->GetHandle(),
				e.dstBinding,
				e.dstArrayElement,
				e.descriptorCount);
		}
		vkUpdateDescriptorSets(mDevice,
							   static_cast<uint32_t>(writes.size()),
							   writes.data(),
							   static_cast<uint32_t>(copies.size()),
							   copies.data());
	}
	void VKDevice::FlushMappedMemoryRanges(std::span<const MappedMemoryRange> ranges)
	{
		std::vector<VkMappedMemoryRange> vkranges;
		static auto v = std::views::transform(
			[](const MappedMemoryRange &e)
			{
				if (auto p = *std::get_if<Buffer const *>(&e.memory))
				{
					return VkMappedMemoryRange{
						VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
						nullptr,
						static_cast<VKBuffer const *>(p)->GetMemory(),
						e.offset,
						e.size};
				}
				else if (auto p2 = *std::get_if<Image const *>(&e.memory))
				{
					return VkMappedMemoryRange{
						VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
						nullptr,
						static_cast<VKImage const *>(p2)->GetMemory(),
						e.offset,
						e.size};
				}
				ST_THROW("error, VKDevice FlushMappedMemoryRanges");
			});
		std::ranges::copy(ranges | v, std::back_inserter(vkranges));
		CHECK_VK_RESULT(vkFlushMappedMemoryRanges(mDevice, static_cast<uint32_t>(vkranges.size()), vkranges.data()))
	}
	void VKDevice::InvalidateMappedMemoryRanges(std::span<const MappedMemoryRange> ranges)
	{
		std::vector<VkMappedMemoryRange> vkranges;
		static auto v = std::views::transform(
			[](const MappedMemoryRange &e)
			{
				if (auto p = *std::get_if<Buffer const *>(&e.memory))
				{
					return VkMappedMemoryRange{
						VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
						nullptr,
						static_cast<VKBuffer const *>(p)->GetMemory(),
						e.offset,
						e.size};
				}
				else if (auto p2 = *std::get_if<Image const *>(&e.memory))
				{
					return VkMappedMemoryRange{
						VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
						nullptr,
						static_cast<VKImage const *>(p2)->GetMemory(),
						e.offset,
						e.size};
				}
				ST_THROW("error, VKDevice InvalidateMappedMemoryRanges");
			});
		std::ranges::copy(ranges | v, std::back_inserter(vkranges));
		CHECK_VK_RESULT(vkInvalidateMappedMemoryRanges(mDevice, static_cast<uint32_t>(vkranges.size()), vkranges.data()))
	}
} // namespace Shit
