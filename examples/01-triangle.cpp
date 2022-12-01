#include "common/common.h"

uint32_t WIDTH = 800, HEIGHT = 600;

std::vector<uint16_t> indices{0, 1, 2, 3};

class Hello
{
	Shit::RenderSystem *renderSystem;
	Shit::Window *window;
	Shit::Device *device;

	Shit::Swapchain *swapchain;
	std::vector<Shit::ImageView *> swapchainImageViews;
	std::vector<Shit::Image *> swapchainImages;
	std::vector<Shit::Framebuffer *> framebuffers;

	Shit::CommandPool *commandPool;
	std::vector<Shit::CommandBuffer *> commandBuffers;
	Shit::Queue *graphicsQueue;
	Shit::Queue *presentQueue;
	Shit::Semaphore *imageAvailableSemaphore;
	Shit::Semaphore *renderFinishedSemaphore;

	Shit::Shader *vertShader;
	Shit::Shader *fragShader;

	Shit::PipelineLayout *pipelineLayout;
	Shit::RenderPass *renderPass;
	Shit::Pipeline *pipeline;

	Shit::Buffer *drawIndirectCmdBuffer;
	Shit::Buffer *drawCountBuffer;
	Shit::Buffer *drawIndexedIndirectCmdBuffer;
	Shit::Buffer *indexBuffer;

public:
	void initRenderSystem()
	{
		Shit::RenderSystemCreateInfo renderSystemCreateInfo{
			g_RendererVersion,
			Shit::RenderSystemCreateFlagBits::CONTEXT_DEBUG_BIT};

		renderSystem = LoadRenderSystem(renderSystemCreateInfo);
		g_RendererVersion = renderSystem->GetCreateInfo()->version;
		// 1. create window
		Shit::WindowCreateInfo windowCreateInfo{
			{},
			__WFILE__,
			{{DEFAULT_WINDOW_X, DEFAULT_WINDOW_Y},
			 {DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT}},
			std::bind(&Hello::ProcessEvent, this, std::placeholders::_1)};
		window = renderSystem->CreateRenderWindow(windowCreateInfo);
		// 1.5 choose phyiscal device
		// 2. create device of a physical device
		Shit::DeviceCreateInfo deviceCreateInfo{};
		if ((g_RendererVersion & Shit::RendererVersion::TypeBitmask) == Shit::RendererVersion::GL)
			deviceCreateInfo = {window};

		device = renderSystem->CreateDevice(deviceCreateInfo);

		// 3
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

		pipelineLayout = device->Create(Shit::PipelineLayoutCreateInfo{});
		createPipeline();

		createCommandBuffers();
		recordCommandBuffers();
	}
	/**
	 * @brief process window event, do not write render code here
	 *
	 * @param ev
	 */
	void ProcessEvent(const Shit::Event &ev)
	{
		std::visit(Shit::overloaded{
					   [&ev](const Shit::KeyEvent &value)
					   {
						   if (value.keyCode == Shit::KeyCode::KEY_ESCAPE)
							   ev.pWindow->Close();
					   },
					   [](auto &&) {},
					   [&ev]([[maybe_unused]] const Shit::WindowResizeEvent &value) {
					   },
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
		for (auto &&e : framebuffers)
			device->Destroy(e);
		device->Destroy(pipeline);
		device->Destroy(swapchain);
	}
	void recreateSwapchain()
	{
		cleanupSwapchain();
		createSwapchain();
		createFramebuffers();
		createPipeline();
		recordCommandBuffers();
	}
	void drawFrame()
	{
		// static uint32_t currentFrame{};
		uint32_t imageIndex{};
		Shit::GetNextImageInfo nextImageInfo{
			UINT64_MAX,
			imageAvailableSemaphore};
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

		std::vector<Shit::SubmitInfo> submitInfos{
			{1, &imageAvailableSemaphore,
			 1, &commandBuffers[imageIndex],
			 1, &renderFinishedSemaphore}};
		graphicsQueue->Submit(submitInfos, nullptr);

		Shit::PresentInfo presentInfo{
			1, &renderFinishedSemaphore,
			1, &swapchain,
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
		presentQueue->WaitIdle();
	}

	void createShaders()
	{
		g_shaderSourceLanguage = chooseShaderShourceLanguage(
			{Shit::ShadingLanguage::SPIRV, Shit::ShadingLanguage::GLSL}, device);

		std::string vertCode = "\
#version 460\n\
#ifdef VULKAN\n\
#define VertexID gl_VertexIndex\n\
#else\n\
#define VertexID gl_VertexID\n\
#endif\n\
const vec2 inPos[4] = {{-0.5, -0.5}, {0.5, -0.5},{-0.5, 0.5},{0.5,0.5}};\n\
//const vec4 color[4]={{1,0,0,1},{0,0,1,1},{0,1,0,1},{1,1,1,1}};\n\
const vec4 color[4]={{1,0,0,0.5},{0,0,1,0.5},{0,1,0,0.5},{1,1,1,0.5}};\n\
struct VS_OUT{\n\
	vec4 color;\n\
};\n\
layout(location=0) out VS_OUT vs_out;\n\
void main() { \n\
	gl_Position = vec4(inPos[VertexID], 0, 1); \n\
	vs_out.color=color[VertexID];\n\
}\n	";
		std::string fragCode = "\
#version 460\n\
layout(location = 0) out vec4 outColor;\n\
struct VS_OUT{\n\
	vec4 color;\n\
};\n\
layout(location=0)in VS_OUT fs_in;\n\
void main() { \n\
	outColor = fs_in.color;\n\
}\n";

		std::string vertCodeSpv = compileGlslToSpirv(vertCode, Shit::ShaderStageFlagBits::VERTEX_BIT, g_RendererVersion);
		std::string fragCodeSpv = compileGlslToSpirv(fragCode, Shit::ShaderStageFlagBits::FRAGMENT_BIT, g_RendererVersion);

		vertShader = device->Create(Shit::ShaderCreateInfo{g_shaderSourceLanguage, vertCodeSpv.size(), vertCodeSpv.data()});
		fragShader = device->Create(Shit::ShaderCreateInfo{g_shaderSourceLanguage, fragCodeSpv.size(), fragCodeSpv.data()});
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

		auto presentMode = choosePresentMode({Shit::PresentMode::IMMEDIATE, Shit::PresentMode::FIFO}, device, window);

		uint32_t framebufferWidth, framebufferHeight;
		window->GetFramebufferSize(framebufferWidth, framebufferHeight);
		while (framebufferWidth == 0 || framebufferHeight == 0)
		{
			window->WaitEvents();
			window->GetFramebufferSize(framebufferWidth, framebufferHeight);
		}
		// set imgui
		swapchain = device->Create(
			Shit::SwapchainCreateInfo{
				2, // min image count
				swapchainFormat.format,
				swapchainFormat.colorSpace,
				{framebufferWidth, framebufferHeight},
				1,
				Shit::ImageUsageFlagBits::COLOR_ATTACHMENT_BIT,
				presentMode,
				window});

		auto swapchainImageCount = swapchain->GetImageCount();

		swapchainImages.resize(swapchainImageCount);
		swapchain->GetImages(swapchainImages.data());
		Shit::ImageViewCreateInfo imageViewCreateInfo{
			nullptr,
			Shit::ImageViewType::TYPE_2D,
			swapchain->GetCreateInfoPtr()->format,
			{},
			{Shit::ImageAspectFlagBits::COLOR_BIT, 0, 1, 0, 1}};

		swapchainImageViews.resize(swapchainImageCount);
		while (swapchainImageCount-- > 0)
		{
			imageViewCreateInfo.pImage = swapchainImages[swapchainImageCount];
			swapchainImageViews[swapchainImageCount] = device->Create(imageViewCreateInfo);
		}
		auto presentQueueFamilyProperty = device->GetPresentQueueFamilyProperty(window);
		presentQueue = device->GetQueue(presentQueueFamilyProperty->index, 0);
	}
	void createRenderPasses()
	{
		std::vector<Shit::AttachmentDescription> attachmentDescriptions{
			{swapchain->GetCreateInfoPtr()->format,
			 Shit::SampleCountFlagBits::BIT_1,
			 Shit::AttachmentLoadOp::CLEAR,
			 Shit::AttachmentStoreOp::DONT_CARE,
			 Shit::AttachmentLoadOp::DONT_CARE,
			 Shit::AttachmentStoreOp::DONT_CARE,
			 Shit::ImageLayout::UNDEFINED, // inital layout
			 Shit::ImageLayout::PRESENT_SRC}};

		std::vector<Shit::AttachmentReference> colorAttachments{
			{0, // the index of attachment description
			 Shit::ImageLayout::COLOR_ATTACHMENT_OPTIMAL},
		};

		std::vector<Shit::SubpassDescription> subPasses{
			Shit::SubpassDescription{
				Shit::PipelineBindPoint::GRAPHICS,
				0,
				nullptr,
				static_cast<uint32_t>(colorAttachments.size()),
				colorAttachments.data(),
			},
		};
		Shit::RenderPassCreateInfo renderPassCreateInfo{
			attachmentDescriptions.size(),
			attachmentDescriptions.data(),
			subPasses.size(),
			subPasses.data(),
		};

		renderPass = device->Create(renderPassCreateInfo);
	}
	void createPipeline()
	{
		std::vector<Shit::PipelineShaderStageCreateInfo> shaderStageCreateInfos{
			Shit::PipelineShaderStageCreateInfo{
				Shit::ShaderStageFlagBits::VERTEX_BIT,
				vertShader,
				"main",
			},
			Shit::PipelineShaderStageCreateInfo{
				Shit::ShaderStageFlagBits::FRAGMENT_BIT,
				fragShader,
				"main",
			},
		};
		Shit::VertexInputStateCreateInfo vertexInputState{};
		Shit::PipelineInputAssemblyStateCreateInfo inputAssemblyState{
			Shit::PrimitiveTopology::TRIANGLE_STRIP,
		};
		Shit::Viewport viewport{0, 0,
								static_cast<float>(swapchain->GetCreateInfoPtr()->imageExtent.width),
								static_cast<float>(swapchain->GetCreateInfoPtr()->imageExtent.height),
								0, 1};
		Shit::Rect2D scissor{{0, 0}, swapchain->GetCreateInfoPtr()->imageExtent};

		Shit::PipelineViewportStateCreateInfo viewportStateCreateInfo{
			1,
			&viewport,
			1,
			&scissor};

		Shit::PipelineTessellationStateCreateInfo tessellationState{};
		Shit::PipelineRasterizationStateCreateInfo rasterizationState{
			false,
			false,
			Shit::PolygonMode::FILL,
			Shit::CullMode::BACK,
			Shit::FrontFace::COUNTER_CLOCKWISE,
			false,
		};
		rasterizationState.lineWidth = 1.f;

		Shit::PipelineMultisampleStateCreateInfo multisampleState{
			Shit::SampleCountFlagBits::BIT_1,
		};
		Shit::PipelineDepthStencilStateCreateInfo depthStencilState{};

		Shit::PipelineColorBlendAttachmentState colorBlendAttachmentstate{};
		colorBlendAttachmentstate.colorWriteMask = Shit::ColorComponentFlagBits::R_BIT | Shit::ColorComponentFlagBits::G_BIT | Shit::ColorComponentFlagBits::B_BIT | Shit::ColorComponentFlagBits::A_BIT;
		Shit::PipelineColorBlendStateCreateInfo colorBlendState{
			false,
			{},
			1,
			&colorBlendAttachmentstate,
		};

		Shit::GraphicsPipelineCreateInfo pipelineCreateInfo{
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
			0};
		pipeline = device->Create(pipelineCreateInfo);
	}
	void createFramebuffers()
	{
		Shit::FramebufferCreateInfo framebufferCreateInfo{
			renderPass,
			0,
			nullptr,
			swapchain->GetCreateInfoPtr()->imageExtent,
			1};
		auto count = swapchainImageViews.size();
		framebuffers.resize(count);
		framebufferCreateInfo.attachmentCount = 1;
		for (uint32_t i = 0; i < count; ++i)
		{
			framebufferCreateInfo.pAttachments = &swapchainImageViews[i];
			framebuffers[i] = device->Create(framebufferCreateInfo);
		}
	}
	void createCommandBuffers()
	{
		uint32_t count = swapchain->GetImageCount();
		Shit::CommandBufferCreateInfo cmdBufferCreateInfo{
			Shit::CommandBufferLevel::PRIMARY, count};
		commandBuffers.resize(count);
		commandPool->CreateCommandBuffers(cmdBufferCreateInfo, commandBuffers.data());
	}
	void recordCommandBuffers()
	{
		uint32_t count = swapchain->GetImageCount();
		Shit::CommandBufferBeginInfo cmdBufferBeginInfo{};

		std::vector<Shit::ClearValue> clearValues{std::array<float, 4>{0.2f, 0.2f, 0.2f, 1.f}};

		Shit::BeginRenderPassInfo renderPassBeginInfo{
			renderPass,
			nullptr,
			Shit::Rect2D{
				{},
				swapchain->GetCreateInfoPtr()->imageExtent},
			static_cast<uint32_t>(clearValues.size()),
			clearValues.data(),
			Shit::SubpassContents::INLINE};

		for (uint32_t i = 0; i < count; ++i)
		{
			renderPassBeginInfo.pFramebuffer = framebuffers[i];
			commandBuffers[i]->Reset({});
			commandBuffers[i]->Begin(cmdBufferBeginInfo);
			commandBuffers[i]->BeginRenderPass(renderPassBeginInfo);
			commandBuffers[i]->BindPipeline({Shit::PipelineBindPoint::GRAPHICS, pipeline});

			int drawMethod = 0;
			switch (drawMethod)
			{
			case 0:
			default:
			{
				Shit::DrawIndirectCommand drawIndirectCmd{4, 1, 0, 0};
				commandBuffers[i]->Draw(drawIndirectCmd);
			}
			break;
			case 1:
			{
				Shit::DrawIndirectInfo drawCmdInfo{
					drawIndirectCmdBuffer,
					0,
					1,
					sizeof(Shit::DrawIndirectCommand)};
				commandBuffers[i]->DrawIndirect(drawCmdInfo);
			}
			break;
			case 2:
			{
				Shit::DrawIndirectCountInfo drawCmdInfo{
					drawIndirectCmdBuffer,
					0,
					drawCountBuffer,
					0,
					1,
					sizeof(Shit::DrawIndirectCommand)};
				commandBuffers[i]->DrawIndirectCount(drawCmdInfo);
			}
			break;
			case 3:
			{
				// draw index
				commandBuffers[i]->BindIndexBuffer(Shit::BindIndexBufferInfo{
					indexBuffer,
					0,
					Shit::IndexType::UINT16});
				Shit::DrawIndexedIndirectCommand drawIndexedIndirectCmd{static_cast<uint32_t>(indices.size()), 1, 0, 0, 0};
				commandBuffers[i]->DrawIndexed(drawIndexedIndirectCmd);
			}
			break;
			}
			commandBuffers[i]->EndRenderPass();
			commandBuffers[i]->End();
		}
	}

	void createSyncObjects()
	{
		imageAvailableSemaphore = device->Create(Shit::SemaphoreCreateInfo{});
		renderFinishedSemaphore = device->Create(Shit::SemaphoreCreateInfo{});
	}
	void createDrawCommandBuffers()
	{
		// create draw indirect command buffer
		std::vector<Shit::DrawIndirectCommand> drawIndirectCmds{{4, 1, 0, 0}};
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

		// create draw indexed indirect command buffer
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
};
#include "common/entry.h"
EXAMPLE_MAIN(Hello)