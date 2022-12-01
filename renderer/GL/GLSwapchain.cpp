/**
 * @file GLSwapchain.cpp
 * @author yangzs
 * @brief
 * @version 0.1
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "GLSwapchain.hpp"
#include "GLImage.hpp"
#include "GLDevice.hpp"
#include "GLFence.hpp"
#include "GLSemaphore.hpp"
#include "GLRenderPass.hpp"
#include "GLFramebuffer.hpp"
#include "GLState.hpp"

namespace Shit
{
	GLSwapchain::GLSwapchain(GLDevice *pDevice, GLStateManager *pStateManager, const SwapchainCreateInfo &createInfo)
		: Swapchain(createInfo), mpDevice(pDevice), mpStateManager(pStateManager)
	{
		mProcessWindowEventCallable = std::bind(&GLSwapchain::ProcessWindowEvent, this, std::placeholders::_1);

#ifdef CLIP_ORIGIN_UPPER_LEFT
		mpStateManager->ClipControl(GL_UPPER_LEFT, GL_ZERO_TO_ONE);
#else
		mpStateManager->ClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
#endif
		// cubemap seamless
		mpStateManager->EnableCapability(GL_TEXTURE_CUBE_MAP_SEAMLESS);

#ifndef NDEBUG
		GL::listGLInfo();
#endif
		mpDevice->SetPresentMode(mCreateInfo.presentMode);

		CreateImages(GetSwapchainImageCount());
		// CreateRenderPass();
		// CreateFramebuffer();
	}
	GLSwapchain::~GLSwapchain()
	{
		// mpDevice->Destroy(mpRenderPass);
		// mpDevice->Destroy(mpFramebuffer);
		// for (auto &&e : mpImageViews)
		//	mpDevice->Destroy(e);
		glDeleteFramebuffers(1, &mReadFBO);
		mpStateManager->NotifyReleasedFramebuffer(mReadFBO);
	}
	void GLSwapchain::CreateImages(uint32_t count)
	{
		mImages.reserve(count);
		if (count == 1)
		{
			// default fbo
			mImages.emplace_back(new GLImage(mpStateManager, GLImageFlag::BACK_LEFT_FRAMEBUFFER));
			return;
		}

		ImageCreateInfo imageCreateInfo{
			{},
			ImageType::TYPE_2D,
			mCreateInfo.format,
			{mCreateInfo.imageExtent.width,
			 mCreateInfo.imageExtent.height,
			 1u},
			1,
			1,
			SampleCountFlagBits::BIT_1,
			ImageTiling::OPTIMAL,
			ImageUsageFlagBits::COLOR_ATTACHMENT_BIT,
			MemoryPropertyFlagBits::DEVICE_LOCAL_BIT};

		glGenFramebuffers(1, &mReadFBO);
		mpStateManager->PushDrawFramebuffer(mReadFBO);

		std::vector<GLenum> bindpoints(count);
		for (uint32_t i = 0; i < count; ++i)
		{
			auto pImage = static_cast<const GLImage *>(mImages.emplace_back(new GLImage(mpStateManager, imageCreateInfo, nullptr)));

			bindpoints[i] = GL_COLOR_ATTACHMENT0 + i;
			auto imageFlag = pImage->GetImageFlag();
			// bind fbo attachment
			if (imageFlag == GLImageFlag::TEXTURE)
				glFramebufferTexture(GL_DRAW_FRAMEBUFFER, bindpoints[i], pImage->GetHandle(), 0);
			else if (imageFlag == GLImageFlag::RENDERBUFFER)
				glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, bindpoints[i], GL_RENDERBUFFER, pImage->GetHandle());
		}
		mpStateManager->PopDrawFramebuffer();
		mpStateManager->BindReadFramebuffer(mReadFBO);
		glReadBuffer(bindpoints[0]);
	}

	/**
	 * @brief TODO: handle not ready, timeout and other cases
	 *
	 * @param info
	 * @param index
	 * @return Result
	 */
	Result GLSwapchain::GetNextImage(const GetNextImageInfo &info, uint32_t &index)
	{
		index = mAvailableImageIndex;
		if (mImages.size() > 1)
		{
			if (mOutDated)
				return Result::SHIT_ERROR_OUT_OF_DATE;
			if (info.pFence)
				static_cast<GLFence const *>(info.pFence)->Reset();
			if (info.pSemaphore)
				static_cast<GLSemaphore const *>(info.pSemaphore)->Reset();
		}
		return Result::SUCCESS;
	}
	void GLSwapchain::ProcessWindowEvent(const Event &ev)
	{
		std::visit(
			[this](auto &&arg)
			{
				using T = std::decay_t<decltype(arg)>;
				if constexpr (std::is_same_v<T, WindowResizeEvent>)
				{
					this->mOutDated = true;
				}
			},
			ev.value);
	}
	void GLSwapchain::Present() const
	{
		if (mImages.size() > 1)
		{
			auto index = mAvailableImageIndex - 1;
			//auto index = mAvailableImageIndex;
			index %= mImages.size();
			// already render to a fbo, blit to default framebuffer
			mpStateManager->BindReadFramebuffer(mReadFBO);
			glReadBuffer(GL_COLOR_ATTACHMENT0 + index);
			mpStateManager->BindDrawFramebuffer(0); // default fbo drawbuffer is leftback buffer
			// glDrawBuffer(GL_BACK_LEFT);

			glBlitFramebuffer(0, 0, mCreateInfo.imageExtent.width, mCreateInfo.imageExtent.height,
							  0, 0, mCreateInfo.imageExtent.width, mCreateInfo.imageExtent.height,
							  GL_COLOR_BUFFER_BIT,
							  GL_NEAREST);
			mAvailableImageIndex += 1;
			mAvailableImageIndex %= mImages.size();
		}
		mpDevice->SwapBuffer();
	}
} // namespace Shit
