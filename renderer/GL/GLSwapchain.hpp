/**
 * @file GLSwapchain.hpp
 * @author yangzs
 * @brief 
 * @version 0.1
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#pragma once
#include <renderer/ShitSwapchain.hpp>
#include "GLPrerequisites.hpp"

#ifdef _WIN32
#include <renderer/ShitWindowWin32.hpp>
#endif

namespace Shit
{

	/**
	 * @brief images count is in [8,16]
	 * 
	 */
	class GLSwapchain : public Swapchain
	{
		GLDevice *mpDevice;
		GLStateManager *mpStateManager;
		mutable uint32_t mAvailableImageIndex{};
		bool mOutDated{};

		std::function<void(const Event &)> mProcessWindowEventCallable;

		GLuint mReadFBO; //attach image to a read fbo

	protected:
		void CreateImages(uint32_t count);

		//void CreateRenderPass();

		/**
		 * @brief Create a Framebuffer object used to blit
		 * 
		 */
		//void CreateFramebuffer();

		constexpr uint32_t GetSwapchainImageCount() const
		{
			return mCreateInfo.minImageCount;
			//return (std::min)((std::max)(mCreateInfo.minImageCount, 2U), 8U);
		}

		void EnableDebugOutput(const void *userParam);

		void ProcessWindowEvent(const Event &ev);

	public:
		GLSwapchain(GLDevice *pDevice, GLStateManager *pStateManager, const SwapchainCreateInfo &createInfo);

		~GLSwapchain() override;

		Result GetNextImage(const GetNextImageInfo &info, uint32_t &index) override;

		/**
		 * @brief if swapchain use default framebuffer, then do nothing,
		 *  otherwise blit current image to backleft framebuffer
		 * 
		 */
		void Present() const;

		decltype(auto) GetProcessWindowEventCallable() const
		{
			return mProcessWindowEventCallable;
		}
	};
} // namespace Shit
