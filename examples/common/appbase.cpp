#include "appbase.h"

AppBase *g_App;

static Shit::WindowPixelFormat chooseSwapchainFormat(
	const std::vector<Shit::WindowPixelFormat> &candidates,
	Shit::Device *pDevice,
	Shit::Window *pWindow)
{
	std::vector<Shit::WindowPixelFormat> supportedFormats;
	pDevice->GetWindowPixelFormats(pWindow, supportedFormats);
	LOG("supported formats:");
	for (auto &&e : supportedFormats)
	{
		LOG_VAR(static_cast<int>(e.format));
		LOG_VAR(static_cast<int>(e.colorSpace));
		for (auto &&a : candidates)
		{
			if (e.format == a.format && e.colorSpace == a.colorSpace)
				return e;
		}
	}
	return supportedFormats[0];
}
static Shit::PresentMode choosePresentMode(
	const std::vector<Shit::PresentMode> &candidates,
	Shit::Device *pDevice,
	Shit::Window *window)
{
	std::vector<Shit::PresentMode> modes;
	pDevice->GetPresentModes(window, modes);
	for (auto &&a : candidates)
	{
		for (auto &&e : modes)
		{
			if (static_cast<int>(e) == static_cast<int>(a))
				return e;
		}
	}
	return modes[0];
}

//============================================================
AppBase::AppBase()
{
	g_App = this;
}
AppBase::~AppBase()
{
}
void AppBase::init(Shit::RendererVersion rendererVersion, const wchar_t *windowTitle, Shit::SampleCountFlagBits sampleCount)
{
	_sampleCount = sampleCount;
	_windowTitle = windowTitle;
	_rendererVersion = rendererVersion;

	initRenderSystem();
	createSwapchain();
	createQueues();
	createCommandPool();
	createSyncObjects();
	createDescriptorPool();
	createDescriptorSetLayouts();
	initManagers();

	createDefaultRenderPass();
	createDefaultRenderTarget();
	initDefaultCommandBuffer();
}
void AppBase::initManagers()
{
	_imageManager = std::make_unique<ImageManager>();
	_shaderManager = std::make_unique<ShaderManager>();
	_textureManager = std::make_unique<TextureManager>();
	_materialDataBlockManager = std::make_unique<MaterialDataBlockManager>();
	_modelManager = std::make_unique<ModelManager>();

	_shaderManager->registerResourceLoader<ShaderSpvLoader>(ResourceManager::DEFAULT_LOADER_NAME, _device);
}
void AppBase::initRenderSystem()
{
	_renderSystem = LoadRenderSystem(
		Shit::RenderSystemCreateInfo{
			_rendererVersion,
#ifndef NDEBUG
			Shit::RenderSystemCreateFlagBits::CONTEXT_DEBUG_BIT,
#endif
		});
	// 1. create window
	//		std::make_shared<std::function<void(const Shit::Event &)>>(std::bind(&AppBase::processBaseEvent, this, std::placeholders::_1))};
	_window = _renderSystem->CreateRenderWindow(
		Shit::WindowCreateInfo{
			Shit::WindowCreateFlagBits::FIXED_SIZE,
			__WFILE__,
			{{80, 40},
			 {1280, 720}},
		});

	// 1.5 choose phyiscal device
	// 2. create device of a physical device
	Shit::DeviceCreateInfo deviceCreateInfo{};
	if ((_rendererVersion & Shit::RendererVersion::TypeBitmask) == Shit::RendererVersion::GL)
		deviceCreateInfo = {_window};
	_device = _renderSystem->CreateDevice(deviceCreateInfo);
}
void AppBase::createSwapchain()
{
	auto swapchainFormat = chooseSwapchainFormat(
		{
			//{Shit::Format::B8G8R8A8_UNORM, Shit::ColorSpace::SRGB_NONLINEAR},
			{Shit::Format::B8G8R8A8_SRGB, Shit::ColorSpace::SRGB_NONLINEAR},
		},
		_device, _window);
	LOG_VAR(static_cast<int>(swapchainFormat.format));
	LOG_VAR(static_cast<int>(swapchainFormat.colorSpace));

	auto presentMode = choosePresentMode(
		{Shit::PresentMode::MAILBOX, Shit::PresentMode::FIFO},
		//{Shit::PresentMode::FIFO},
		_device, _window);
	LOG("selected present mode:", static_cast<int>(presentMode));

	uint32_t framebufferWidth, framebufferHeight;
	_window->GetFramebufferSize(framebufferWidth, framebufferHeight);
	while (framebufferWidth == 0 || framebufferHeight == 0)
	{
		_window->WaitEvents();
		_window->GetFramebufferSize(framebufferWidth, framebufferHeight);
	}
	_window->AddEventListener(std::bind(&AppBase::windowEventListener, this, std::placeholders::_1));

	_swapchain = _device->Create(
		Shit::SwapchainCreateInfo{
			MAX_FRAMES_IN_FLIGHT, // min image count
			swapchainFormat.format,
			swapchainFormat.colorSpace,
			{framebufferWidth, framebufferHeight},
			1,
			Shit::ImageUsageFlagBits::COLOR_ATTACHMENT_BIT |
				Shit::ImageUsageFlagBits::TRANSFER_SRC_BIT |
				Shit::ImageUsageFlagBits::TRANSFER_DST_BIT,
			presentMode,
			_window});
}
void AppBase::recreateSwapchain()
{
	for (auto e : _defaultRenderTarget.imageViews)
		_device->Destroy(e);
	_defaultRenderTarget.imageViews.clear();
	for (auto e : _defaultRenderTarget.images)
		_device->Destroy(e);
	_defaultRenderTarget.images.clear();
	for (auto &&e : _defaultRenderTarget.frameBuffers)
		_device->Destroy(e);
	_defaultRenderTarget.frameBuffers.clear();
	_device->Destroy(_swapchain);
	createSwapchain();
	createDefaultRenderTarget();
	_swapchainRecreateSignal();
	_needRecreateSwapchain = false;
	_needUpdateDefaultCommandBuffer = true;
}
void AppBase::createDefaultRenderPass()
{
	Shit::Format depthFormatCandidates[]{
		Shit::Format::D24_UNORM_S8_UINT,
		Shit::Format::D32_SFLOAT_S8_UINT};
	auto depthFormat = _device->GetSuitableImageFormat(
		depthFormatCandidates,
		Shit::ImageTiling::OPTIMAL,
		Shit::FormatFeatureFlagBits::DEPTH_STENCIL_ATTACHMENT_BIT);

	// create renderpass
	std::vector<Shit::AttachmentDescription> attachmentDesc{
		// color
		{_swapchain->GetCreateInfoPtr()->format,
		 _sampleCount,
		 Shit::AttachmentLoadOp::CLEAR,
		 Shit::AttachmentStoreOp::DONT_CARE,
		 Shit::AttachmentLoadOp::DONT_CARE,
		 Shit::AttachmentStoreOp::DONT_CARE,
		 Shit::ImageLayout::UNDEFINED,
		 Shit::ImageLayout::COLOR_ATTACHMENT_OPTIMAL},
		// depth
		{depthFormat,
		 _sampleCount,
		 Shit::AttachmentLoadOp::CLEAR,
		 Shit::AttachmentStoreOp::DONT_CARE,
		 Shit::AttachmentLoadOp::DONT_CARE,
		 Shit::AttachmentStoreOp::DONT_CARE,
		 Shit::ImageLayout::UNDEFINED,
		 Shit::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL},
	};
	if (_sampleCount != Shit::SampleCountFlagBits::BIT_1)
	{
		attachmentDesc[0].finalLayout = Shit::ImageLayout::COLOR_ATTACHMENT_OPTIMAL;
		// resolve attachment
		attachmentDesc.emplace_back(
			_swapchain->GetCreateInfoPtr()->format,
			Shit::SampleCountFlagBits::BIT_1,
			Shit::AttachmentLoadOp::CLEAR,
			Shit::AttachmentStoreOp::DONT_CARE,
			Shit::AttachmentLoadOp::DONT_CARE,
			Shit::AttachmentStoreOp::DONT_CARE,
			Shit::ImageLayout::UNDEFINED,
			Shit::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
	}
	Shit::AttachmentReference colorAttachment{0, Shit::ImageLayout::COLOR_ATTACHMENT_OPTIMAL};
	Shit::AttachmentReference depthAttachment{1, Shit::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
	Shit::AttachmentReference resolveAttachment{2, Shit::ImageLayout::COLOR_ATTACHMENT_OPTIMAL};

	Shit::SubpassDescription subpassDesc[]{
		Shit::PipelineBindPoint::GRAPHICS,
		0,
		0,
		1,
		&colorAttachment,
		_sampleCount == Shit::SampleCountFlagBits::BIT_1 ? 0 : &resolveAttachment,
		&depthAttachment};

	_defaultRenderTarget.renderPass = _device->Create(
		Shit::RenderPassCreateInfo{
			(uint32_t)attachmentDesc.size(),
			attachmentDesc.data(),
			1,
			subpassDesc});

	setDefaultRenderPassClearValue();
}
void AppBase::setDefaultRenderPassClearValue()
{
	Shit::ClearColorValueFloat clearColor = {0.2, 0.2, 0.2, 1};
	Shit::ClearDepthStencilValue clearDepthStencil = {1., 0};
	_defaultRenderTarget.clearValues = {
		clearColor, clearDepthStencil};
	if (_sampleCount != Shit::SampleCountFlagBits::BIT_1)
	{
		_defaultRenderTarget.clearValues.emplace_back(clearColor);
	}
}
void AppBase::createDefaultRenderTarget()
{
	// create framebuffer
	// color
	Shit::ImageCreateInfo colorImageCI{
		{}, // create flags
		Shit::ImageType::TYPE_2D,
		_swapchain->GetCreateInfoPtr()->format,
		{_swapchain->GetCreateInfoPtr()->imageExtent.width,
		 _swapchain->GetCreateInfoPtr()->imageExtent.height,
		 1},
		1, // mipmap levels,
		1, // array layers
		_sampleCount,
		Shit::ImageTiling::OPTIMAL,
		Shit::ImageUsageFlagBits::COLOR_ATTACHMENT_BIT | Shit::ImageUsageFlagBits::TRANSFER_SRC_BIT,
		Shit::MemoryPropertyFlagBits::DEVICE_LOCAL_BIT};
	Shit::ImageViewCreateInfo colorImageViewCI{
		0,
		Shit::ImageViewType::TYPE_2D,
		_swapchain->GetCreateInfoPtr()->format,
		{},
		{Shit::ImageAspectFlagBits::COLOR_BIT, 0, 1, 0, 1}};

	//================
	Shit::ImageCreateInfo depthImageCI{
		{}, // create flags
		Shit::ImageType::TYPE_2D,
		_defaultRenderTarget.renderPass->GetCreateInfoPtr()->pAttachments[1].format,
		{_swapchain->GetCreateInfoPtr()->imageExtent.width,
		 _swapchain->GetCreateInfoPtr()->imageExtent.height,
		 1},
		1, // mipmap levels,
		1, // array layers
		_sampleCount,
		Shit::ImageTiling::OPTIMAL,
		Shit::ImageUsageFlagBits::DEPTH_STENCIL_ATTACHMENT_BIT,
		Shit::MemoryPropertyFlagBits::DEVICE_LOCAL_BIT};

	Shit::ImageViewCreateInfo depthImageViewCI{
		0,
		Shit::ImageViewType::TYPE_2D,
		_defaultRenderTarget.renderPass->GetCreateInfoPtr()->pAttachments[1].format,
		{},
		{Shit::ImageAspectFlagBits::DEPTH_BIT | Shit::ImageAspectFlagBits::STENCIL_BIT, 0, 1, 0, 1}};

	Shit::FramebufferCreateInfo frameBufferCI{
		_defaultRenderTarget.renderPass,
		_defaultRenderTarget.renderPass->GetCreateInfoPtr()->attachmentCount,
		0,
		Shit::Extent2D{
			_swapchain->GetCreateInfoPtr()->imageExtent.width,
			_swapchain->GetCreateInfoPtr()->imageExtent.height},
		1};

	std::vector<Shit::ImageView *> attachments;
	auto frameCount = getFramesInFlight();
	for (int i = 0; i < frameCount; ++i)
	{
		if (_sampleCount == Shit::SampleCountFlagBits::BIT_1)
		{
			colorImageViewCI.pImage = _defaultRenderTarget.images.emplace_back(_swapchain->GetImageByIndex(i));
			_defaultRenderTarget.imageViews.emplace_back(_device->Create(colorImageViewCI));

			depthImageViewCI.pImage = _defaultRenderTarget.images.emplace_back(_device->Create(depthImageCI));
			_defaultRenderTarget.imageViews.emplace_back(_device->Create(depthImageViewCI));
		}
		else
		{
			// color image
			colorImageViewCI.pImage = _defaultRenderTarget.images.emplace_back(_device->Create(colorImageCI));
			_defaultRenderTarget.imageViews.emplace_back(_device->Create(colorImageViewCI));

			//
			depthImageViewCI.pImage = _defaultRenderTarget.images.emplace_back(_device->Create(depthImageCI));
			_defaultRenderTarget.imageViews.emplace_back(_device->Create(depthImageViewCI));

			// resolve image
			colorImageViewCI.pImage = _defaultRenderTarget.images.emplace_back(_swapchain->GetImageByIndex(i));
			_defaultRenderTarget.imageViews.emplace_back(_device->Create(colorImageViewCI));
		}
		frameBufferCI.pAttachments = &_defaultRenderTarget.imageViews[i * frameBufferCI.attachmentCount];
		_defaultRenderTarget.frameBuffers.emplace_back(_device->Create(frameBufferCI));
	}
}

void AppBase::createCommandPool()
{
	_shortLiveCommandPool = _device->Create(
		Shit::CommandPoolCreateInfo{
			Shit::CommandPoolCreateFlagBits::TRANSIENT_BIT |
				Shit::CommandPoolCreateFlagBits::RESET_COMMAND_BUFFER_BIT,
			_graphicsQueue->GetFamilyIndex()});
	_longLiveCommandPool = _device->Create(
		Shit::CommandPoolCreateInfo{Shit::CommandPoolCreateFlagBits::RESET_COMMAND_BUFFER_BIT,
									_graphicsQueue->GetFamilyIndex()});
}
void AppBase::createSyncObjects()
{
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		_imageAvailableSemaphores[i] = _device->Create(Shit::SemaphoreCreateInfo{});
		_renderFinishedSemaphores[i] = _device->Create(Shit::SemaphoreCreateInfo{});
		_inFlightFences[i] = _device->Create(Shit::FenceCreateInfo{Shit::FenceCreateFlagBits::SIGNALED_BIT});
	}
}
void AppBase::createDescriptorPool()
{
	Shit::DescriptorPoolSize poolsize[]{
		{Shit::DescriptorType::COMBINED_IMAGE_SAMPLER, 500},
		{Shit::DescriptorType::STORAGE_IMAGE, 500},
		{Shit::DescriptorType::UNIFORM_BUFFER, 500},
		{Shit::DescriptorType::UNIFORM_BUFFER_DYNAMIC, 500},
		{Shit::DescriptorType::STORAGE_BUFFER, 500},
		{Shit::DescriptorType::STORAGE_BUFFER_DYNAMIC, 500},
	};
	_descriptorPool = _device->Create(Shit::DescriptorPoolCreateInfo{
		1000, std::size(poolsize), poolsize});
}
void AppBase::initDefaultCommandBuffer()
{
	_longLiveCommandPool->CreateCommandBuffers(
		Shit::CommandBufferCreateInfo{
			Shit::CommandBufferLevel::PRIMARY,
			MAX_FRAMES_IN_FLIGHT},
		_defaultCommandBuffers);

	// Shit::ImageSubresourceRange ranges[]{
	//	{Shit::ImageAspectFlagBits::COLOR_BIT, 0, 1, 0, 1}};
}
void AppBase::recordDefaultCommandBuffer()
{
	// Shit::ClearColorValueInt32 clearValue{0.2, 0.2, 0.2, 1.};
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		_defaultCommandBuffers[i]->Reset();
		_defaultCommandBuffers[i]->Begin({});
		//_defaultCommandBuffers[i]->ClearColorImage(Shit::ClearColorImageInfo{
		//	_swapchain->GetImageByIndex(i),
		//	Shit::ImageLayout::TRANSFER_DST_OPTIMAL,
		//	clearValue,
		//	1,
		//	ranges});
		_defaultCommandBuffers[i]->BeginRenderPass(
			Shit::BeginRenderPassInfo{
				_defaultRenderTarget.renderPass,
				_defaultRenderTarget.frameBuffers[i],
				{{}, _swapchain->GetCreateInfoPtr()->imageExtent},
				(uint32_t)_defaultRenderTarget.clearValues.size(),
				_defaultRenderTarget.clearValues.data(),
				Shit::SubpassContents::INLINE});
		_defaultCommandBuffers[i]->EndRenderPass();
		_defaultCommandBuffers[i]->End();
	}
}
void AppBase::createQueues()
{
	// present queue
	auto presentQueueFamilyProprety = _device->GetPresentQueueFamilyProperty(_window);
	_presentQueue = _device->GetQueue(presentQueueFamilyProprety->index, 0);

	// graphics queue
	// 5
	auto graphicsQueueFamilyProperty = _device->GetQueueFamilyProperty(Shit::QueueFlagBits::GRAPHICS_BIT);
	if (!graphicsQueueFamilyProperty.has_value())
		THROW("failed to find a graphic queue");
	_graphicsQueue = _device->GetQueue(graphicsQueueFamilyProperty->index, 0);

	// transfer queue
	auto transferQueueFamilyProperty = _device->GetQueueFamilyProperty(Shit::QueueFlagBits::TRANSFER_BIT);
	if (!transferQueueFamilyProperty.has_value())
		THROW("failed to find a transfer queue");
	_transferQueue = _device->GetQueue(transferQueueFamilyProperty->index, 0);

	// compute queue
	auto computeQueueFamilyProperty = _device->GetQueueFamilyProperty(Shit::QueueFlagBits::COMPUTE_BIT);
	if (!computeQueueFamilyProperty.has_value())
		THROW("failed to find a compute queue");
	_computeQueue = _device->GetQueue(computeQueueFamilyProperty->index, 0);
}
void AppBase::updateScene()
{
	for (auto e : _behaviours)
	{
		e->updatePerFrame(_frameDeltaTimeMs);
	}

	// updae scene
	for (auto &&e : _gameObjects)
	{
		e->update();
	}
}
void AppBase::mainLoop()
{
	_timer.Restart();
	while (_window->PollEvents())
	{
		//_device->WaitIdle();
		_frameDeltaTimeMs = _timer.GetDt();

		_inFlightFences[_currentFrame]->WaitFor(UINT64_MAX);

		uint32_t imageIndex{};
		Shit::GetNextImageInfo nextImageInfo{
			UINT64_MAX,
			_imageAvailableSemaphores[_currentFrame]};
		auto ret = _swapchain->GetNextImage(nextImageInfo, imageIndex);
		if (ret == Shit::Result::SHIT_ERROR_OUT_OF_DATE || _needRecreateSwapchain)
		{
			_device->WaitIdle();
			recreateSwapchain();
			continue;
		}
		else if (ret != Shit::Result::SUCCESS && ret != Shit::Result::SUBOPTIMAL)
		{
			THROW("failed to get next image");
		}
		_inFlightFences[_currentFrame]->Reset();

		updateScene();
		updateFrame(imageIndex);
		_editorCamera->updateUBOBuffer(imageIndex);

		renderImGui(imageIndex);
		if (_needUpdateDefaultCommandBuffer)
		{
			_device->WaitIdle();
			recordDefaultCommandBuffer();
			_needUpdateDefaultCommandBuffer = false;
		}

		setupSubmitCommandBuffers(imageIndex);

		std::vector<Shit::SubmitInfo> submitInfos{
			{1, &_imageAvailableSemaphores[_currentFrame],
			 (uint32_t)_submitCommandBuffers.size(),
			 _submitCommandBuffers.data(),
			 1, &_renderFinishedSemaphores[_currentFrame]}};
		_graphicsQueue->Submit(submitInfos, _inFlightFences[_currentFrame]);

		Shit::PresentInfo presentInfo{
			1, &_renderFinishedSemaphores[_currentFrame],
			1, &_swapchain,
			&imageIndex};
		auto res = _presentQueue->Present(presentInfo);
		if (res == Shit::Result::SHIT_ERROR_OUT_OF_DATE || res == Shit::Result::SUBOPTIMAL)
		{
			_needRecreateSwapchain = true;
			//_device->WaitIdle();
			//recreateSwapchain();
		}
		else if (res != Shit::Result::SUCCESS)
		{
			THROW("failed to present swapchain image");
		}
		if (_takeScreenShot)
		{
			takeScreenshot(_device, _swapchain->GetImageByIndex(imageIndex), Shit::ImageLayout::PRESENT_SRC);
			_takeScreenShot = false;
		}
		_currentFrame = (_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}
	_device->WaitIdle();
}
void AppBase::setupSubmitCommandBuffers(uint32_t frameIndex)
{
	_submitCommandBuffers = {_defaultCommandBuffers[frameIndex], _imguiCommandBuffers[frameIndex]};
}
void AppBase::createEditorCamera()
{
	// create camera
	uint32_t width, height;
	_window->GetFramebufferSize(width, height);
	auto cameraNode = _gameObjects.emplace_back(std::make_unique<GameObject>()).get();
	_editorCamera = cameraNode->addComponent<Camera>(
		PerspectiveDescription{glm::radians(60.f), float(width) / height}, 0.01, 500.f);
	_editorCamera->getParentTransform()->translate({0, 0, 3});
	cameraNode->addComponent<EditCameraController>(_window);
}
void AppBase::windowEventListener(Shit::Event const &event)
{
	ImGuiIO &io = ImGui::GetIO();
	std::visit(Shit::overloaded{
				   [&](Shit::KeyEvent const &value)
				   {
					   io.KeysDown[Shit::KeyCode::KEY_ESCAPE] = value.keyCode == Shit::KeyCode::KEY_ESCAPE && value.state;
					   if (value.keyCode == Shit::KeyCode::KEY_ESCAPE && !value.state)
						   event.pWindow->Close();
					   _takeScreenShot = io.KeysDown[Shit::KeyCode::KEY_P] = value.keyCode == Shit::KeyCode::KEY_P && value.state;
					   io.KeysDown[Shit::KeyCode::KEY_BACK] = value.keyCode == Shit::KeyCode::KEY_BACK && value.state;
					   io.KeysDown[Shit::KeyCode::KEY_PERIOD] = value.keyCode == Shit::KeyCode::KEY_PERIOD && value.state;
					   io.KeysDown[Shit::KeyCode::KEY_RETURN] = value.keyCode == Shit::KeyCode::KEY_RETURN && value.state;
				   },
				   [&](const Shit::CharEvent &value)
				   {
					   //   LOG("char", value.codepoint);
					   io.AddInputCharacter(value.codepoint);
				   },
				   [&](const Shit::MouseButtonEvent &value)
				   {
					   io.MouseDown[0] = value.button == Shit::MouseButton::MOUSE_L && value.state;
					   io.MouseDown[1] = value.button == Shit::MouseButton::MOUSE_R && value.state;
					   io.MouseDown[2] = value.button == Shit::MouseButton::MOUSE_M && value.state;
				   },
				   [&](const Shit::MouseMoveEvent &value)
				   {
					   io.MousePos.x = value.xpos;
					   io.MousePos.y = value.ypos;
				   },
				   [&](const Shit::MouseWheelEvent &value)
				   {
					   if (io.WantCaptureMouse)
					   {
						   io.MouseWheel += value.yoffset;
						   io.MouseWheelH += value.xoffset;
					   }
				   },
				   [&](const Shit::WindowResizeEvent &value)
				   {
					   uint32_t width, height;
					   event.pWindow->GetFramebufferSize(width, height);
					   io.DisplaySize.x = width;
					   io.DisplaySize.y = height;
				   },
				   [](auto &&) {},
			   },
			   event.value);
}
Shit::Sampler *AppBase::createSampler(std::string_view name, Shit::SamplerCreateInfo const &createInfo)
{
	return _samplers.emplace(std::string(name), _device->Create(createInfo)).first->second;
}
void AppBase::createSamplers()
{
	createSampler("linear",
				  Shit::SamplerCreateInfo{
					  Shit::Filter::LINEAR,
					  Shit::Filter::LINEAR,
					  Shit::SamplerMipmapMode::NEAREST,
					  Shit::SamplerWrapMode::REPEAT,
					  Shit::SamplerWrapMode::REPEAT,
					  Shit::SamplerWrapMode::REPEAT,
					  0,
					  false,
					  0,
					  false,
					  {},
					  -1000,
					  1000,
					  Shit::BorderColor::FLOAT_OPAQUE_BLACK});
	createSampler("nearest",
				  Shit::SamplerCreateInfo{
					  Shit::Filter::NEAREST,
					  Shit::Filter::NEAREST,
					  Shit::SamplerMipmapMode::NEAREST,
					  Shit::SamplerWrapMode::REPEAT,
					  Shit::SamplerWrapMode::REPEAT,
					  Shit::SamplerWrapMode::REPEAT,
					  0,
					  false,
					  0,
					  false,
					  {},
					  -1000,
					  1000,
					  Shit::BorderColor::FLOAT_OPAQUE_BLACK});
	createSampler("trilinear",
				  Shit::SamplerCreateInfo{
					  Shit::Filter::LINEAR,
					  Shit::Filter::LINEAR,
					  Shit::SamplerMipmapMode::LINEAR,
					  Shit::SamplerWrapMode::REPEAT,
					  Shit::SamplerWrapMode::REPEAT,
					  Shit::SamplerWrapMode::REPEAT,
					  0,
					  false,
					  0,
					  false,
					  {},
					  -1000,
					  1000,
					  Shit::BorderColor::FLOAT_OPAQUE_BLACK});
}
void AppBase::createDescriptorSetLayouts()
{
	//
	Shit::DescriptorSetLayoutBinding bindings[]{
		// transform
		{0, Shit::DescriptorType::UNIFORM_BUFFER, 1, Shit::ShaderStageFlagBits::VERTEX_BIT},
		// camera
		{0, Shit::DescriptorType::STORAGE_BUFFER, 1,
		 Shit::ShaderStageFlagBits::VERTEX_BIT |
			 Shit::ShaderStageFlagBits::FRAGMENT_BIT |
			 Shit::ShaderStageFlagBits::GEOMETRY_BIT |
			 Shit::ShaderStageFlagBits::COMPUTE_BIT},
		// material
		{0, Shit::DescriptorType::COMBINED_IMAGE_SAMPLER, 8, Shit::ShaderStageFlagBits::FRAGMENT_BIT},
		{1, Shit::DescriptorType::UNIFORM_BUFFER, 1, Shit::ShaderStageFlagBits::FRAGMENT_BIT},
		// light
		{1, Shit::DescriptorType::STORAGE_BUFFER, 1, Shit::ShaderStageFlagBits::FRAGMENT_BIT | Shit::ShaderStageFlagBits::GEOMETRY_BIT},
		// env
		//{3, Shit::DescriptorType::UNIFORM_BUFFER, 1, Shit::ShaderStageFlagBits::FRAGMENT_BIT},
		//{8, Shit::DescriptorType::COMBINED_IMAGE_SAMPLER, 1, Shit::ShaderStageFlagBits::FRAGMENT_BIT},
		//{9, Shit::DescriptorType::COMBINED_IMAGE_SAMPLER, 1, Shit::ShaderStageFlagBits::FRAGMENT_BIT},
		//{10, Shit::DescriptorType::COMBINED_IMAGE_SAMPLER, 1, Shit::ShaderStageFlagBits::FRAGMENT_BIT},
	};

	_transformDescriptorSetLayout = _device->Create(Shit::DescriptorSetLayoutCreateInfo{1, &bindings[0]});
	_cameraDescriptorSetLayout = _device->Create(Shit::DescriptorSetLayoutCreateInfo{1, &bindings[1]});
	_materialDescriptorSetLayout = _device->Create(Shit::DescriptorSetLayoutCreateInfo{2, &bindings[2]});
	_lightDescriptorSetLayout = _device->Create(Shit::DescriptorSetLayoutCreateInfo{1, &bindings[4]});
	//_envDescriptorSetLayout = _device->Create(Shit::DescriptorSetLayoutCreateInfo{4, &bindings[5]});
}
void AppBase::initImGui()
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	uint32_t frameWidth, frameHeight;
	_window->GetFramebufferSize(frameWidth, frameHeight);
	io.DisplaySize = ImVec2{float(frameWidth), float(frameHeight)};
	auto imageCount = _swapchain->GetImageCount();

	_imguiCommandBuffers.resize(imageCount);
	_longLiveCommandPool->CreateCommandBuffers(
		Shit::CommandBufferCreateInfo{
			Shit::CommandBufferLevel::PRIMARY,
			imageCount},
		_imguiCommandBuffers.data());

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	// ImGui::StyleColorsClassic();

	std::vector<Shit::Image *> images(imageCount);
	_swapchain->GetImages(images.data());

	ImGui_ImplShitRenderer_InitInfo imguiInitInfo{
		_rendererVersion,
		_device,
		images,
		_imguiCommandBuffers,
		Shit::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,
		Shit::ImageLayout::PRESENT_SRC};

	ImGui_ImplShitRenderer_Init(&imguiInitInfo);
	executeOneTimeCommands(
		_device,
		[](Shit::CommandBuffer *commandBuffer)
		{ ImGui_ImplShitRenderer_CreateFontsTexture(commandBuffer); });
	ImGui_ImplShitRenderer_DestroyFontUploadObjects();
	addRecreateSwapchainListener(std::bind(&AppBase::ImguiRecreateSwapchain, this));
}
void AppBase::ImguiRecreateSwapchain()
{
	std::vector<Shit::Image *> images(_swapchain->GetImageCount());
	_swapchain->GetImages(images.data());
	ImGui_ImplShitRenderer_RecreateFrameBuffers(images);
}
void AppBase::renderImGui(uint32_t frameIndex)
{
	ImGui_ImplShitRenderer_NewFrame();
	ImGui::NewFrame();
	//=======================================
	//ImGui::Begin("info");
	//ImGui::Text("FPS:%0.f", 1000.f / _frameDeltaTimeMs);
	//ImGui::End();
	setupImGui();
	//=======================================
	ImGui::Render();
	ImGui_ImplShitRenderer_RecordCommandBuffer(frameIndex);
}
void AppBase::setupImGui()
{
	static bool isOpen = true;
	ImGui::ShowDemoWindow(&isOpen);
}
void AppBase::run(
	Shit::RendererVersion rendererVersion,
	const wchar_t *windowTitle,
	Shit::SampleCountFlagBits sampleCount)
{
	init(rendererVersion, windowTitle, sampleCount);
	initImGui();

	createEditorCamera();
	createSamplers();

	prepare();

	for (auto &&e : _gameObjects)
	{
		e->getComponents(_behaviours);
		e->prepare();
	}

	updateScene();

	recordDefaultCommandBuffer();

	mainLoop();

	cleanUp();
}