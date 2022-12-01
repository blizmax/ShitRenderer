#include "common/appbase.h"
#include <stb_image.h>

#define MAX_FRAMES_IN_FLIGHT 2
#define SAMPLE_COUNT Shit::SampleCountFlagBits::BIT_1

#define INPUT_IMAGE_COUNT 4

uint32_t WIDTH = 800, HEIGHT = 600;

static const char *vertShaderName = "raytracing/render_in_triangle.vert";

static const char *fragShaderNames[]{
	"raytracing/raytracing_sphere.frag",
	"raytracing/raymarching_sphere.frag",
};
static const char *inputImageNames[INPUT_IMAGE_COUNT]{
	"Lenna_test.jpg",
	//"Mt-Washington-Gold-Room_Ref.hdr",
	"Ridgecrest_Road_Ref.hdr",
	"envBRDF.hdr",
};

std::vector<uint16_t> indices{0, 1, 2};

struct UBO
{
	float viewportSize[2];
	float mousePos[2]; //[0,1]
	float mouseWheel[2];
	alignas(16) int mouseButtonAction[3]; //L M R, action: 0: none, 1: down, 2:up, 3:repeat, 4 unknow
};

static int curPipelineIndex = 0;

class Hello
{
	Shit::RenderSystem *renderSystem;
	Shit::Window *window;
	Shit::Device *device;

	Shit::SwapchainCreateInfo swapchainCreateInfo;
	Shit::Swapchain *swapchain;
	std::vector<Shit::ImageView *> swapchainImageViews;
	std::vector<Shit::Framebuffer *> framebuffers;

	Shit::CommandPool *commandPool;
	std::vector<Shit::CommandBuffer *> commandBuffers;
	Shit::Queue *graphicsQueue;
	Shit::Queue *presentQueue;
	Shit::Semaphore *imageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT];
	Shit::Semaphore *renderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT];
	Shit::Fence *inFlightFences[MAX_FRAMES_IN_FLIGHT];

	Shit::Shader *vertShader;
	std::vector<Shit::Shader *> fragShaders;

	Shit::PipelineLayout *pipelineLayout;
	Shit::RenderPass *renderPass;
	std::vector<Shit::Pipeline *> pipelines;

	Shit::Buffer *drawIndirectCmdBuffer;
	Shit::Buffer *drawCountBuffer;
	Shit::Buffer *drawIndexedIndirectCmdBuffer;
	Shit::Buffer *indexBuffer;

	Shit::DescriptorPool *descriptorPool;
	Shit::DescriptorSetLayout *descriptorSetLayout;
	std::vector<Shit::DescriptorSet *> descriptorSets;
	std::vector<Shit::Buffer *> uboBuffers;

	bool uboFlags[MAX_FRAMES_IN_FLIGHT]{};
	UBO ubo{};

	Shit::Sampler *linearSampler;
	Shit::Image *inputImages[INPUT_IMAGE_COUNT];
	Shit::ImageView *inputImageViews[INPUT_IMAGE_COUNT];

	///=========================
	//GUI
	Shit::CommandPool *guiCommandPool;
	std::vector<Shit::CommandBuffer *> guiCommandBuffers; //secondary commandbuffers
	Shit::RenderPass *guiRenderPass;

public:
	void initRenderSystem()
	{
		Shit::RenderSystemCreateInfo renderSystemCreateInfo{
			g_RendererVersion,
			Shit::RenderSystemCreateFlagBits::CONTEXT_DEBUG_BIT};

		renderSystem = LoadRenderSystem(renderSystemCreateInfo);
		//1. create window
		Shit::WindowCreateInfo windowCreateInfo{
			{},
			__WFILE__,
			{{DEFAULT_WINDOW_X, DEFAULT_WINDOW_Y},
			 {DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT}}};
		window = renderSystem->CreateRenderWindow(windowCreateInfo);
		//1.5 choose phyiscal device
		//2. create device of a physical device
		Shit::DeviceCreateInfo deviceCreateInfo{};
		if ((g_RendererVersion & Shit::RendererVersion::TypeBitmask) == Shit::RendererVersion::GL)
			deviceCreateInfo = {window};

		device = renderSystem->CreateDevice(deviceCreateInfo);

		//3
		auto graphicsQueueFamilyProperty = device->GetQueueFamilyProperty(Shit::QueueFlagBits::GRAPHICS_BIT);
		if (!graphicsQueueFamilyProperty.has_value())
			THROW("failed to find a graphic queue");

		LOG_VAR(graphicsQueueFamilyProperty->index);
		LOG_VAR(graphicsQueueFamilyProperty->count);

		// 4. command pool
		Shit::CommandPoolCreateInfo commandPoolCreateInfo{
			Shit::CommandPoolCreateFlagBits::RESET_COMMAND_BUFFER_BIT,
			graphicsQueueFamilyProperty->index};
		commandPool = device->Create(commandPoolCreateInfo);

		// 5
		graphicsQueue = device->GetQueue(graphicsQueueFamilyProperty->index, 0);

		createShaders();
		createDrawCommandBuffers();
		createSyncObjects();
		createIndexBuffer();

		createSwapchain();
		createRenderPasses();
		createFramebuffers();

		createDescriptorSets();
		createInputImages();
		createUBOBuffer();

		createPipelines();

		commandBuffers.resize(swapchain->GetImageCount());
		commandPool->CreateCommandBuffers(
			Shit::CommandBufferCreateInfo{
				Shit::CommandBufferLevel::PRIMARY,
				swapchain->GetImageCount()},
			commandBuffers.data());
		updateCommandBuffer(pipelines[curPipelineIndex]);

		//prepare GUI
		initGUI();
		window->AddEventListener(std::bind(&Hello::ProcessEvent, this, std::placeholders::_1));
	}
	/**
	 * @brief process window event, do not write render code here
	 * 
	 * @param ev 
	 */
	void ProcessEvent(const Shit::Event &ev)
	{
		ImGuiIO &io = ImGui::GetIO();
		std::visit(Shit::overloaded{
					   [&](const Shit::KeyEvent &value)
					   {
						   if (value.keyCode == Shit::KeyCode::KEY_ESCAPE)
							   ev.pWindow->Close();
						   io.KeysDown[Shit::KeyCode::KEY_BACK] = value.keyCode == Shit::KeyCode::KEY_BACK && value.state;
						   if (value.keyCode == Shit::KeyCode::KEY_W)
						   {
							   io.KeysDown[Shit::KeyCode::KEY_W] = value.state;
						   }
						   if (value.keyCode == Shit::KeyCode::KEY_S)
						   {
							   io.KeysDown[Shit::KeyCode::KEY_S] = value.state;
						   }
						   if (value.keyCode == Shit::KeyCode::KEY_A)
						   {
							   io.KeysDown[Shit::KeyCode::KEY_A] = value.state;
						   }
						   if (value.keyCode == Shit::KeyCode::KEY_D)
						   {
							   io.KeysDown[Shit::KeyCode::KEY_D] = value.state;
						   }
					   },
					   [&](const Shit::CharEvent &value)
					   {
						   io.AddInputCharacter(value.codepoint);
					   },
					   [&](const Shit::MouseButtonEvent &value)
					   {
						   io.MouseDown[0] = value.button == Shit::MouseButton::MOUSE_L && value.state;
						   io.MouseDown[1] = value.button == Shit::MouseButton::MOUSE_R && value.state;
						   io.MouseDown[2] = value.button == Shit::MouseButton::MOUSE_M && value.state;
						   if (!io.WantCaptureMouse)
						   {
							   for (auto &&e : uboFlags)
								   e = true;
							   //   ubo.mouseButtonAction[static_cast<int>(value.button) - 1] = static_cast<int>(value.action);
						   }
					   },
					   [&](const Shit::MouseMoveEvent &value)
					   {
						   io.MousePos.x = value.xpos;
						   io.MousePos.y = value.ypos;
						   if (!io.WantCaptureMouse)
						   {
							   for (auto &&e : uboFlags)
								   e = true;
							   ubo.mousePos[0] = float(value.xpos) / swapchain->GetCreateInfoPtr()->imageExtent.width;
							   ubo.mousePos[1] = float(value.ypos) / swapchain->GetCreateInfoPtr()->imageExtent.height;
						   }
					   },
					   [&](const Shit::MouseWheelEvent &value)
					   {
						   if (io.WantCaptureMouse)
						   {
							   io.MouseWheel += value.yoffset;
							   io.MouseWheelH += value.xoffset;
						   }
						   for (auto &&e : uboFlags)
							   e = true;
						   ubo.mouseWheel[0] += value.xoffset;
						   ubo.mouseWheel[1] += value.yoffset;
					   },
					   [&](const Shit::WindowResizeEvent &value)
					   {
						   uint32_t width, height;
						   ev.pWindow->GetFramebufferSize(width, height);
						   io.DisplaySize.x = width;
						   io.DisplaySize.y = height;
						   for (auto &&e : uboFlags)
							   e = true;
					   },
					   [](auto &&) {},
				   },
				   ev.value);
	}

	void mainLoop()
	{
		while (window->PollEvents())
		{
			drawFrame();
		}
		presentQueue->WaitIdle();
	}
	void cleanUp()
	{
	}
	void run()
	{
		initRenderSystem();
		mainLoop();
		cleanUp();
	}
	void cleanupSwapchain()
	{
		for (auto &&e : swapchainImageViews)
			device->Destroy(e);
		device->Destroy(renderPass);
		for (auto &&e : framebuffers)
			device->Destroy(e);
		//device->Destroy(pipelineLayout);
		for (auto e : pipelines)
			device->Destroy(e);
		pipelines.clear();
		device->Destroy(swapchain);
	}
	void recreateSwapchain()
	{
		cleanupSwapchain();
		createSwapchain();
		createRenderPasses();
		createFramebuffers();
		createPipelines();
		guiRecreateSwapchain();
		updateCommandBuffer(pipelines[curPipelineIndex]);
	}
	void drawFrame()
	{
		static uint32_t currentFrame{};
		inFlightFences[currentFrame]->WaitFor(UINT64_MAX);

		uint32_t imageIndex{};
		Shit::GetNextImageInfo nextImageInfo{
			UINT64_MAX,
			imageAvailableSemaphores[currentFrame]};
		auto res = swapchain->GetNextImage(nextImageInfo, imageIndex);
		if (res == Shit::Result::SHIT_ERROR_OUT_OF_DATE)
		{
			presentQueue->WaitIdle();
			recreateSwapchain();
			return;
		}
		else if (res != Shit::Result::SUCCESS && res != Shit::Result::SUBOPTIMAL)
		{
			THROW("failed to get next image");
		}
		//==================================
		inFlightFences[currentFrame]->Reset();

		updateUBOBuffer(imageIndex);
		drawGUI(imageIndex);
		//=========================
		Shit::CommandBuffer *cmdBuffers[] = {commandBuffers[imageIndex], guiCommandBuffers[imageIndex]};
		std::vector<Shit::SubmitInfo> submitInfos{
			{1,
			 &imageAvailableSemaphores[currentFrame],
			 2,
			 cmdBuffers,
			 1,
			 &renderFinishedSemaphores[currentFrame]}};
		graphicsQueue->Submit(submitInfos, inFlightFences[currentFrame]);

		Shit::PresentInfo presentInfo{
			1,
			&renderFinishedSemaphores[currentFrame],
			1,
			&swapchain,
			&imageIndex};
		res = presentQueue->Present(presentInfo);
		if (res == Shit::Result::SHIT_ERROR_OUT_OF_DATE || res == Shit::Result::SUBOPTIMAL)
		{
			presentQueue->WaitIdle();
			recreateSwapchain();
		}
		else if (res != Shit::Result::SUCCESS)
		{
			THROW("failed to present swapchain image");
		}
		//presentQueue->WaitIdle();
		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void createShaders()
	{
		auto vertShaderPath = buildShaderPath(device, vertShaderName, g_RendererVersion);
		auto vertCode = readFile(vertShaderPath.c_str());
		vertShader = device->Create(Shit::ShaderCreateInfo{g_shaderSourceLanguage, vertCode.size(), vertCode.data()});

		for (auto e : fragShaderNames)
		{
			auto fragShaderPath = buildShaderPath(device, e, g_RendererVersion);
			auto fragCode = readFile(fragShaderPath.c_str());
			fragShaders.emplace_back(device->Create(Shit::ShaderCreateInfo{g_shaderSourceLanguage, fragCode.size(), fragCode.data()}));
		}
	}

	void createSwapchain()
	{
		auto swapchainFormat = chooseSwapchainFormat(
			{
				{Shit::Format::B8G8R8A8_SRGB, Shit::ColorSpace::SRGB_NONLINEAR},
				{Shit::Format::R8G8B8A8_SRGB, Shit::ColorSpace::SRGB_NONLINEAR},
			},
			device,
			window);
		LOG_VAR(static_cast<int>(swapchainFormat.format));
		LOG_VAR(static_cast<int>(swapchainFormat.colorSpace));

		auto presentMode = choosePresentMode({Shit::PresentMode::IMMEDIATE, Shit::PresentMode::MAILBOX, Shit::PresentMode::FIFO}, device, window);

		swapchainCreateInfo = Shit::SwapchainCreateInfo{
			2,
			swapchainFormat.format,
			swapchainFormat.colorSpace,
			{800, 600},
			1,
			Shit::ImageUsageFlagBits::COLOR_ATTACHMENT_BIT |
				Shit::ImageUsageFlagBits::TRANSFER_SRC_BIT,
			presentMode,
			window};
		window->GetFramebufferSize(swapchainCreateInfo.imageExtent.width, swapchainCreateInfo.imageExtent.height);
		while (swapchainCreateInfo.imageExtent.width == 0 && swapchainCreateInfo.imageExtent.height == 0)
		{
			window->WaitEvents();
			window->GetFramebufferSize(swapchainCreateInfo.imageExtent.width, swapchainCreateInfo.imageExtent.height);
		}
		swapchain = device->Create(swapchainCreateInfo);
		Shit::ImageViewCreateInfo imageViewCreateInfo{
			nullptr,
			Shit::ImageViewType::TYPE_2D,
			swapchainCreateInfo.format,
			{},
			{Shit::ImageAspectFlagBits::COLOR_BIT, 0, 1, 0, 1}};
		auto swapchainImageCount = swapchain->GetImageCount();
		swapchainImageViews.resize(swapchainImageCount);
		while (swapchainImageCount-- > 0)
		{
			imageViewCreateInfo.pImage = swapchain->GetImageByIndex(swapchainImageCount);
			swapchainImageViews[swapchainImageCount] = device->Create(imageViewCreateInfo);
		}
		auto presentQueueFamilyProperty = device->GetPresentQueueFamilyProperty(window);
		presentQueue = device->GetQueue(presentQueueFamilyProperty->index, 0);
	}
	void createRenderPasses()
	{
		std::vector<Shit::AttachmentDescription> attachmentDescriptions{
			{swapchain->GetCreateInfoPtr()->format,
			 SAMPLE_COUNT,
			 Shit::AttachmentLoadOp::CLEAR,
			 Shit::AttachmentStoreOp::DONT_CARE,
			 Shit::AttachmentLoadOp::DONT_CARE,
			 Shit::AttachmentStoreOp::DONT_CARE,
			 Shit::ImageLayout::UNDEFINED, //inital layout
			 Shit::ImageLayout::PRESENT_SRC}};

		std::vector<Shit::AttachmentReference> colorAttachments{
			{0, //the index of attachment description
			 Shit::ImageLayout::COLOR_ATTACHMENT_OPTIMAL},
		};

		std::vector<Shit::SubpassDescription> subPasses{
			Shit::SubpassDescription{
				Shit::PipelineBindPoint::GRAPHICS,
				0,
				0,
				(uint32_t)colorAttachments.size(),
				colorAttachments.data(),
			},
		};
		Shit::RenderPassCreateInfo renderPassCreateInfo{
			(uint32_t)attachmentDescriptions.size(),
			attachmentDescriptions.data(),
			(uint32_t)subPasses.size(),
			subPasses.data(),
		};

		renderPass = device->Create(renderPassCreateInfo);
	}
	void createPipelines()
	{
		static Shit::VertexInputStateCreateInfo vertexInputState{};
		static Shit::PipelineInputAssemblyStateCreateInfo inputAssemblyState{
			Shit::PrimitiveTopology::TRIANGLE_LIST,
		};
		//Shit::PipelineViewportStateCreateInfo viewportStateCreateInfo{
		//	{{0, 0, 800, 600, 0, 1}},
		//	{{{0, 0}, {500, 400}}}};
		Shit::Viewport viewports[]{{0, 0, static_cast<float>(swapchain->GetCreateInfoPtr()->imageExtent.width), static_cast<float>(swapchain->GetCreateInfoPtr()->imageExtent.height), 0, 1}};
		Shit::Rect2D scissors[]{{{0, 0}, swapchain->GetCreateInfoPtr()->imageExtent}};
		Shit::PipelineViewportStateCreateInfo viewportStateCreateInfo{
			(uint32_t)std::size(viewports),
			viewports,
			(uint32_t)std::size(scissors),
			scissors};
		static Shit::PipelineTessellationStateCreateInfo tessellationState{};
		static Shit::PipelineRasterizationStateCreateInfo rasterizationState{
			false,
			false,
			Shit::PolygonMode::FILL,
			Shit::CullMode::BACK,
			Shit::FrontFace::COUNTER_CLOCKWISE,
			false,
			0, 0, 0,
			1.f};

		static Shit::PipelineMultisampleStateCreateInfo multisampleState{
			SAMPLE_COUNT,
		};
		static Shit::PipelineDepthStencilStateCreateInfo depthStencilState{};

		static Shit::PipelineColorBlendAttachmentState colorBlendAttachmentstate{
			false,
			Shit::BlendFactor::SRC_ALPHA,
			Shit::BlendFactor::ONE_MINUS_SRC_ALPHA,
			Shit::BlendOp::ADD,
			{},
			{},
			{},
			Shit::ColorComponentFlagBits::R_BIT | Shit::ColorComponentFlagBits::G_BIT | Shit::ColorComponentFlagBits::B_BIT | Shit::ColorComponentFlagBits::A_BIT};
		static Shit::PipelineColorBlendStateCreateInfo colorBlendState{
			false,
			Shit::LogicOp::COPY,
			1,
			&colorBlendAttachmentstate,
		};
		for (auto e : fragShaders)
		{
			std::vector<Shit::PipelineShaderStageCreateInfo> shaderStageCreateInfos{
				Shit::PipelineShaderStageCreateInfo{
					Shit::ShaderStageFlagBits::VERTEX_BIT,
					vertShader,
					"main",
				},
				Shit::PipelineShaderStageCreateInfo{
					Shit::ShaderStageFlagBits::FRAGMENT_BIT,
					e,
					"main",
				},
			};
			pipelines.emplace_back(device->Create(
				Shit::GraphicsPipelineCreateInfo{
					(uint32_t)shaderStageCreateInfos.size(),
					shaderStageCreateInfos.data(),
					vertexInputState,
					inputAssemblyState,
					viewportStateCreateInfo,
					tessellationState,
					rasterizationState,
					multisampleState,
					depthStencilState,
					colorBlendState,
					{},
					pipelineLayout,
					renderPass,
					0}));
		}
	}
	void createFramebuffers()
	{
		Shit::FramebufferCreateInfo framebufferCreateInfo{
			renderPass,
			0,
			0,
			swapchain->GetCreateInfoPtr()->imageExtent,
			1};
		auto count = swapchainImageViews.size();
		framebuffers.resize(count);
		framebufferCreateInfo.attachmentCount = 1;
		while (count-- > 0)
		{
			framebufferCreateInfo.pAttachments = &swapchainImageViews[count];
			framebuffers[count] = device->Create(framebufferCreateInfo);
		}
	}
	void updateCommandBuffer(Shit::Pipeline *pipeline)
	{
		static std::vector<Shit::ClearValue> clearValues{std::array<float, 4>{0.2f, 0.2f, 0.2f, 1.f}};

		Shit::CommandBufferBeginInfo cmdBufferBeginInfo{};

		Shit::BeginRenderPassInfo renderPassBeginInfo{
			renderPass,
			nullptr,
			Shit::Rect2D{
				{},
				swapchain->GetCreateInfoPtr()->imageExtent},
			static_cast<uint32_t>(clearValues.size()),
			clearValues.data(),
			Shit::SubpassContents::INLINE};

		device->WaitIdle();

		for (uint32_t i = 0; i < swapchain->GetImageCount(); ++i)
		{
			renderPassBeginInfo.pFramebuffer = framebuffers[i];
			commandBuffers[i]->Reset({});
			commandBuffers[i]->Begin(cmdBufferBeginInfo);
			commandBuffers[i]->BeginRenderPass(renderPassBeginInfo);
			commandBuffers[i]->BindPipeline({Shit::PipelineBindPoint::GRAPHICS, pipeline});
			commandBuffers[i]->BindDescriptorSets(Shit::BindDescriptorSetsInfo{
				Shit::PipelineBindPoint::GRAPHICS,
				pipelineLayout,
				0,
				1,
				&descriptorSets[i]});
			int drawMethod = 1;
			switch (drawMethod)
			{
			case 0:
			default:
			{
				commandBuffers[i]->Draw(Shit::DrawIndirectCommand{3, 1, 0, 0});
			}
			break;
			case 1:
			{
				commandBuffers[i]->DrawIndirect(
					Shit::DrawIndirectInfo{
						drawIndirectCmdBuffer,
						0,
						1,
						sizeof(Shit::DrawIndirectCommand)});
			}
			break;
			}
			commandBuffers[i]->EndRenderPass();
			commandBuffers[i]->End();
		}
	}

	void createSyncObjects()
	{
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			imageAvailableSemaphores[i] = device->Create(Shit::SemaphoreCreateInfo{});
			renderFinishedSemaphores[i] = device->Create(Shit::SemaphoreCreateInfo{});
			inFlightFences[i] = device->Create(Shit::FenceCreateInfo{Shit::FenceCreateFlagBits::SIGNALED_BIT});
		}
	}
	void createDrawCommandBuffers()
	{
		//create draw indirect command buffer
		std::vector<Shit::DrawIndirectCommand> drawIndirectCmds{{3, 1, 0, 0}};
		Shit::BufferCreateInfo bufferCreateInfo{
			{},
			sizeof(Shit::DrawIndirectCommand) * drawIndirectCmds.size(),
			Shit::BufferUsageFlagBits::INDIRECT_BUFFER_BIT | Shit::BufferUsageFlagBits::TRANSFER_DST_BIT,
			Shit::MemoryPropertyFlagBits::DEVICE_LOCAL_BIT};
		drawIndirectCmdBuffer = device->Create(bufferCreateInfo, drawIndirectCmds.data());
		bufferCreateInfo.size = sizeof(uint32_t);
		uint32_t drawCount = 1;
		drawCountBuffer = device->Create(bufferCreateInfo, &drawCount);

		std::vector<Shit::DrawIndexedIndirectCommand> drawIndexedIndirectCmds{{static_cast<uint32_t>(indices.size()), 1, 0, 0, 0}};

		//create draw indexed indirect command buffer
		Shit::BufferCreateInfo drawIndexedIndirectCmdBufferCreateInfo{
			{},
			sizeof(Shit::DrawIndexedIndirectCommand) * drawIndexedIndirectCmds.size(),
			Shit::BufferUsageFlagBits::INDIRECT_BUFFER_BIT | Shit::BufferUsageFlagBits::TRANSFER_DST_BIT,
			Shit::MemoryPropertyFlagBits::DEVICE_LOCAL_BIT};
		drawIndexedIndirectCmdBuffer = device->Create(drawIndexedIndirectCmdBufferCreateInfo, drawIndexedIndirectCmds.data());
	}
	void createIndexBuffer()
	{
		Shit::BufferCreateInfo indexBufferCreateInfo{
			{},
			sizeof(indices[0]) * indices.size(),
			Shit::BufferUsageFlagBits::INDEX_BUFFER_BIT | Shit::BufferUsageFlagBits::TRANSFER_DST_BIT,
			Shit::MemoryPropertyFlagBits::DEVICE_LOCAL_BIT};
		indexBuffer = device->Create(indexBufferCreateInfo, indices.data());
	}
	void createDescriptorSets()
	{
		std::vector<Shit::DescriptorSetLayoutBinding> bindings{
			{0, Shit::DescriptorType::UNIFORM_BUFFER, 1, Shit::ShaderStageFlagBits::FRAGMENT_BIT},
			{1, Shit::DescriptorType::COMBINED_IMAGE_SAMPLER, 4, Shit::ShaderStageFlagBits::FRAGMENT_BIT},
		};
		descriptorSetLayout = device->Create(Shit::DescriptorSetLayoutCreateInfo{(uint32_t)bindings.size(), bindings.data()});

		std::vector<Shit::DescriptorPoolSize> poolSizes{
			{Shit::DescriptorType::UNIFORM_BUFFER, swapchain->GetImageCount()},
		};
		descriptorPool = device->Create(Shit::DescriptorPoolCreateInfo{swapchain->GetImageCount(), (uint32_t)poolSizes.size(), poolSizes.data()});
		std::vector<Shit::DescriptorSetLayout *> setLayouts(swapchain->GetImageCount(), descriptorSetLayout);
		descriptorPool->Allocate(Shit::DescriptorSetAllocateInfo{(uint32_t)setLayouts.size(), setLayouts.data()}, descriptorSets);
		pipelineLayout = device->Create(Shit::PipelineLayoutCreateInfo{1, &descriptorSetLayout});
	}
	void createInputImages()
	{
		linearSampler = device->Create(Shit::SamplerCreateInfo{
			Shit::Filter::LINEAR,
			Shit::Filter::LINEAR,
			Shit::SamplerMipmapMode::LINEAR,
			Shit::SamplerWrapMode::REPEAT,
			Shit::SamplerWrapMode::REPEAT,
			Shit::SamplerWrapMode::REPEAT,
			0,
			false,
			1.,
			false,
			{},
			-1000.f,
			1000.f,
		});
		//==============
		int width{1}, height{1}, components{4}, componentSize{};
		Shit::Format format = Shit::Format::R8G8B8A8_SRGB;
		for (int i = 0; i < INPUT_IMAGE_COUNT; ++i)
		{
			void *pixels{};
			if (inputImageNames[i])
			{
				auto imagePath = std::string(SHIT_SOURCE_DIR) + "/assets/images/" + inputImageNames[i];
				pixels = loadImage(imagePath.data(), width, height, components, 4, componentSize); //force load an alpha channel,even not exist
			}
			if (componentSize == 4)
				format = Shit::Format::R32G32B32A32_SFLOAT;
			inputImages[i] = device->Create(
				Shit::ImageCreateInfo{
					{},
					Shit::ImageType::TYPE_2D,
					format,
					{(uint32_t)width, (uint32_t)height, 1},
					0,
					1,
					Shit::SampleCountFlagBits::BIT_1,
					Shit::ImageTiling::OPTIMAL,
					Shit::ImageUsageFlagBits::SAMPLED_BIT,
					Shit::MemoryPropertyFlagBits::DEVICE_LOCAL_BIT,
					Shit::ImageLayout::SHADER_READ_ONLY_OPTIMAL},
				Shit::ImageAspectFlagBits::COLOR_BIT,
				pixels);
			inputImages[i]->GenerateMipmap(
				Shit::Filter::LINEAR,
				Shit::ImageLayout::SHADER_READ_ONLY_OPTIMAL,
				Shit::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
			inputImageViews[i] = device->Create(
				Shit::ImageViewCreateInfo{
					inputImages[i],
					Shit::ImageViewType::TYPE_2D,
					format,
					{},
					{Shit::ImageAspectFlagBits::COLOR_BIT, 0, inputImages[i]->GetCreateInfoPtr()->mipLevels, 0, 1}});
		}
	}
	void createUBOBuffer()
	{
		for (int i = 0; i < swapchain->GetImageCount(); ++i)
		{
			uboBuffers.emplace_back(device->Create(Shit::BufferCreateInfo{
													   {},
													   sizeof(UBO),
													   Shit::BufferUsageFlagBits::UNIFORM_BUFFER_BIT,
													   Shit::MemoryPropertyFlagBits::HOST_VISIBLE_BIT | Shit::MemoryPropertyFlagBits::HOST_COHERENT_BIT},
												   &ubo));
		}

		std::vector<Shit::DescriptorImageInfo> imagesInfo;
		for (int j = 0; j < INPUT_IMAGE_COUNT; ++j)
			imagesInfo.emplace_back(
				linearSampler, inputImageViews[j], Shit::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
		std::vector<Shit::WriteDescriptorSet> writes;

		Shit::DescriptorBufferInfo bufferInfo{uboBuffers[0], 0ul, sizeof(UBO)};
		for (int i = 0; i < swapchain->GetImageCount(); ++i)
		{
			writes.emplace_back(Shit::WriteDescriptorSet{descriptorSets[i],
														 0,
														 0,
														 1,
														 Shit::DescriptorType::UNIFORM_BUFFER,
														 0,
														 &bufferInfo});
			writes.emplace_back(descriptorSets[i],
								1,
								0,
								(uint32_t)imagesInfo.size(),
								Shit::DescriptorType::COMBINED_IMAGE_SAMPLER,
								imagesInfo.data());
		}
		device->UpdateDescriptorSets(writes, {});
	}
	void updateUBOBuffer(int index)
	{
		if (uboFlags[index])
		{
			uboFlags[index] = false;
			void *data;
			uboBuffers[index]->MapMemory(0, sizeof(UBO), &data);
			ubo.viewportSize[0] = (float)dynamic_cast<Shit::GraphicsPipeline *>(pipelines[index])->GetCreateInfoPtr()->viewportState.viewports[0].width;
			ubo.viewportSize[1] = (float)dynamic_cast<Shit::GraphicsPipeline *>(pipelines[index])->GetCreateInfoPtr()->viewportState.viewports[0].height;
			memcpy(data, &ubo, sizeof(UBO));
			uboBuffers[index]->UnMapMemory();
		}
	}
	void initGUI()
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO &io = ImGui::GetIO();
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		uint32_t windowWidth, windowHeight;
		window->GetFramebufferSize(windowWidth, windowHeight);
		io.DisplaySize = ImVec2{float(windowWidth), float(windowHeight)};

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsClassic();

		guiCommandPool = device->Create(
			Shit::CommandPoolCreateInfo{
				Shit::CommandPoolCreateFlagBits::RESET_COMMAND_BUFFER_BIT,
				graphicsQueue->GetFamilyIndex()});
		guiCommandBuffers.resize(swapchain->GetImageCount());
		guiCommandPool->CreateCommandBuffers(
			Shit::CommandBufferCreateInfo{
				Shit::CommandBufferLevel::PRIMARY,
				swapchain->GetImageCount()},
			guiCommandBuffers.data());

		std::vector<Shit::Image *> images(swapchain->GetImageCount());
		swapchain->GetImages(images.data());

		ImGui_ImplShitRenderer_InitInfo imguiInitInfo{
			g_RendererVersion,
			device,
			images,
			guiCommandBuffers,
			Shit::ImageLayout::PRESENT_SRC,
			Shit::ImageLayout::PRESENT_SRC,
		};
		ImGui_ImplShitRenderer_Init(&imguiInitInfo);

		executeOneTimeCommands(device, [](Shit::CommandBuffer *commandBuffer)
							   { ImGui_ImplShitRenderer_CreateFontsTexture(commandBuffer); });
		ImGui_ImplShitRenderer_DestroyFontUploadObjects();
	}
	void guiRecreateSwapchain()
	{
		std::vector<Shit::Image *> images(swapchain->GetImageCount());
		swapchain->GetImages(images.data());
		ImGui_ImplShitRenderer_RecreateFrameBuffers(images);
	}
	void drawGUI(uint32_t imageIndex)
	{
		ImGui_ImplShitRenderer_NewFrame();
		ImGui::NewFrame();
		//=======================================
		ImGui::Begin("pipeline selector");
		if (ImGui::Button("screen shot"))
			takeScreenshot(device, swapchain->GetImageByIndex(imageIndex), Shit::ImageLayout::PRESENT_SRC);

		static bool flag = false;
		for (int i = 0, len = IM_ARRAYSIZE(fragShaderNames); i < len; ++i)
			flag |= ImGui::RadioButton(fragShaderNames[i], &curPipelineIndex, i);
		if (flag)
		{
			updateCommandBuffer(pipelines[curPipelineIndex]);
			flag = false;
		}
		ImGui::End();
		//=======================================
		ImGui::Render();
		ImGui_ImplShitRenderer_RecordCommandBuffer(imageIndex);
	}
};
#include "common/entry.h"
EXAMPLE_MAIN(Hello)
