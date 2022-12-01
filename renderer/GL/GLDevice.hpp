/**
 * @file GLDevice.hpp
 * @author yangzs
 * @brief 
 * @version 0.1
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#pragma once

#include <renderer/ShitDevice.hpp>
#include "GLState.hpp"

#ifdef _WIN32
typedef struct HDC__ *HDC;
typedef struct HGLRC__ *HGLRC;
#endif

namespace Shit
{
	/**
	 * @brief in opengl, device is created based on windows, one window has its own device
	 * 
	 */
	class GLDevice : public Device
	{
	protected:
		std::unique_ptr<GLStateManager> mpStateManager{};

		RenderSystemCreateInfo mRenderSystemCreateInfo;

		Window *mpWindow{};

		void EnableDebugOutput(const void *userParam);

		virtual void CreateRenderContext() = 0;

	public:
		GLDevice(const DeviceCreateInfo &createInfo, const RenderSystemCreateInfo &renderSystemCreateInfo);

		~GLDevice() override;

		Result WaitIdle() const override;

		GLStateManager *GetStateManager()
		{
			return mpStateManager.get();
		}
		virtual void MakeCurrent() const = 0;

		virtual void SetPresentMode(PresentMode mode) const = 0;

		virtual void SwapBuffer() const = 0;

		Swapchain *Create(const SwapchainCreateInfo &createInfo) override;

		CommandPool *Create(const CommandPoolCreateInfo &createInfo) override;

		Shader *Create(const ShaderCreateInfo &createInfo) override;

		Pipeline *Create(const GraphicsPipelineCreateInfo &createInfo) override;

		Pipeline *Create(const ComputePipelineCreateInfo &createInfo) override;

		Buffer *Create(const BufferCreateInfo &createInfo, const void *pData) override;

		Buffer *Create(const BufferCreateInfo &createInfo, int val) override;

		BufferView *Create(const BufferViewCreateInfo &createInfo) override;

		Image *Create(const ImageCreateInfo &createInfo, ImageAspectFlagBits aspectMask, const void *pData) override;

		Image *Create(const ImageCreateInfo &createInfo, ImageAspectFlagBits aspectMask, int val) override;

		DescriptorSetLayout *Create(const DescriptorSetLayoutCreateInfo &createInfo) override;

		ImageView *Create(const ImageViewCreateInfo &createInfo) override;
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

		void GetSupportedShaderSourceLanguages(std::vector<ShadingLanguage> &shadingLanguage) const override;
	};

#ifdef _WIN32

	class GLDeviceWin32 final : public GLDevice
	{
		HDC mHDC;
		HGLRC mHRenderContext; //false context

		void CreateRenderContext() override;

		void SetPresentMode(PresentMode mode) const override;

		/**
		 * @brief create a false rendercontext and init wgl extensions
		 * 
		 */
		void InitWglExtentions();

	public:
		GLDeviceWin32(const DeviceCreateInfo &createInfo, const RenderSystemCreateInfo &renderSystemCreateInfo);
		~GLDeviceWin32() override;

		void MakeCurrent() const override;

		void SwapBuffer() const override;

		constexpr HDC GetHDC() const
		{
			return mHDC;
		}
		constexpr HGLRC GetRenderContext() const
		{
			return mHRenderContext;
		}

		void GetWindowPixelFormats(const Window *pWindow, std::vector<WindowPixelFormat> &formats) const override;

		void GetPresentModes(const Window *pWindow, std::vector<PresentMode> &presentModes) const override;
	};
#endif

}