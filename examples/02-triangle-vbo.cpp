#include "common/common.h"

uint32_t WIDTH = 800, HEIGHT = 600;

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;
	static Shit::VertexBindingDescription getVertexBindingDescription(uint32_t binding)
	{
		return {
			binding,
			sizeof(Vertex),
			0,
		};
	}
	static std::vector<Shit::VertexAttributeDescription> getVertexAttributeDescription(uint32_t startLocation, uint32_t binding)
	{
		return {
			{startLocation + 0,
			 binding,
			 Shit::Format::R32G32B32_SFLOAT,
			 offsetof(Vertex, pos)},
			{startLocation + 1,
			 binding,
			 Shit::Format::R32G32B32_SFLOAT,
			 offsetof(Vertex, color)},
			{startLocation + 2,
			 binding,
			 Shit::Format::R32G32_SFLOAT,
			 offsetof(Vertex, texCoord)},
		};
	}
	static uint32_t getLocationCount()
	{
		return 3;
	}
};

std::vector<Vertex> vertices{
	{{-0.5, -0.5, 0}, {1, 0, 0}, {0, 1}},
	{{0.5, -0.5, 0}, {0, 1, 0}, {1, 1}},
	{{-0.5, 0.5, 0}, {0, 0, 1}, {0, 0}},
	{{0.5, 0.5, 0}, {1, 1, 0}, {1, 0}},
};
std::vector<uint16_t> indices{0, 1, 2, 3};

class Hello
{
	Shit::RenderSystem *renderSystem;
	Shit::Window *window;
	Shit::Device *device;

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
	// std::vector<Shit::Fence  *> inFlightImages;

	Shit::Shader *vertShader;
	Shit::Shader *fragShader;

	Shit::DescriptorSetLayout *descriptorSetLayout;
	Shit::PipelineLayout *pipelineLayout;
	Shit::RenderPass *renderPass;
	Shit::Pipeline *pipeline;

	Shit::Buffer *drawIndirectCmdBuffer;
	Shit::Buffer *drawCountBuffer;
	Shit::Buffer *drawIndexedIndirectCmdBuffer;
	Shit::Buffer *indexBuffer;
	Shit::Buffer *vertexBuffer;

public:
	void initRenderSystem()
	{
		Shit::RenderSystemCreateInfo renderSystemCreateInfo{
			.version = g_RendererVersion,
			.flags = Shit::RenderSystemCreateFlagBits::CONTEXT_DEBUG_BIT,
		};

		renderSystem = LoadRenderSystem(renderSystemCreateInfo);
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
		createDescriptors();
		createDrawCommandBuffers();
		createIndexBuffer();
		createVertexBuffer();

		createSwapchain();
		createRenderPasses();
		createFramebuffers();
		pipelineLayout = device->Create(Shit::PipelineLayoutCreateInfo{});
		createPipeline();

		createCommandBuffers();
		recordCommandBuffers();

		createSyncObjects();
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
		static uint32_t currentFrame{};
		inFlightFences[currentFrame]->WaitFor(UINT64_MAX);

		uint32_t imageIndex{};
		Shit::GetNextImageInfo nextImageInfo{
			UINT64_MAX,
			imageAvailableSemaphores[currentFrame]};
		auto ret = swapchain->GetNextImage(nextImageInfo, imageIndex);
		if (ret == Shit::Result::SHIT_ERROR_OUT_OF_DATE)
		{
			presentQueue->WaitIdle();
			recreateSwapchain();
			return;
		}
		else if (ret != Shit::Result::SUCCESS && ret != Shit::Result::SUBOPTIMAL)
		{
			THROW("failed to get next image");
		}

		inFlightFences[currentFrame]->Reset();

		std::vector<Shit::SubmitInfo> submitInfos{
			{1, &imageAvailableSemaphores[currentFrame],
			 1, &commandBuffers[imageIndex],
			 1, &renderFinishedSemaphores[currentFrame]}};
		graphicsQueue->Submit(submitInfos, inFlightFences[currentFrame]);

		Shit::PresentInfo presentInfo{
			1, &renderFinishedSemaphores[currentFrame],
			1, &swapchain,
			&imageIndex};
		auto res = presentQueue->Present(presentInfo);
		if (res == Shit::Result::SHIT_ERROR_OUT_OF_DATE || res == Shit::Result::SUBOPTIMAL)
		{
			presentQueue->WaitIdle();
			recreateSwapchain();
		}
		else if (res != Shit::Result::SUCCESS)
		{
			THROW("failed to present swapchain image");
		}
		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void createShaders()
	{
		g_shaderSourceLanguage = chooseShaderShourceLanguage(
			{Shit::ShadingLanguage::SPIRV, Shit::ShadingLanguage::GLSL}, device);

		std::string vertCode = "\
#version 460\n\
layout(location = 0) in vec3 inPos;\n\
layout(location = 1) in vec3 inColor;\n\
struct VS_OUT{\n\
	vec4 color;\n\
};\n\
layout(location = 0) out VS_OUT vs_out;\n\
void main() {\n\
  gl_Position = vec4(inPos, 1);\n\
  vs_out.color = vec4(inColor,1);\n\
}\n\
";
		std::string fragCode = "\
#version 460\n\
layout(location = 0) out vec4 outColor;\n\
struct VS_OUT{\n\
	vec4 color;\n\
};\n\
layout(location=0) in VS_OUT fs_in;\n\
void main() {\n\
   outColor = fs_in.color;\n\
}\n\
		";

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

		Shit::ImageViewCreateInfo imageViewCreateInfo{
			nullptr,
			Shit::ImageViewType::TYPE_2D,
			swapchain->GetCreateInfoPtr()->format,
			{},
			{Shit::ImageAspectFlagBits::COLOR_BIT, 0, 1, 0, 1}};
		swapchainImageViews.resize(swapchainImageCount);
		while (swapchainImageCount-- > 0)
		{
			imageViewCreateInfo.pImage = swapchain->GetImageByIndex(swapchainImageCount);
			swapchainImageViews[swapchainImageCount] = device->Create(imageViewCreateInfo);
		}
		auto presentQueueFamilyProperty = device->GetPresentQueueFamilyProperty(window);
		presentQueue = device->GetQueue(presentQueueFamilyProperty->index, 0);
	}
	void createDescriptors()
	{
		descriptorSetLayout = device->Create(Shit::DescriptorSetLayoutCreateInfo{});
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
		auto vertexBindingDesc = Vertex::getVertexBindingDescription(0);
		auto vertexAttributeDesc = Vertex::getVertexAttributeDescription(0, 0);
		Shit::VertexInputStateCreateInfo vertexInputState{
			1,
			&vertexBindingDesc,
			(uint32_t)vertexAttributeDesc.size(),
			vertexAttributeDesc.data(),
		};
		Shit::PipelineInputAssemblyStateCreateInfo inputAssemblyState{
			Shit::PrimitiveTopology::TRIANGLE_STRIP,
		};
		Shit::Viewport viewport{0, 0,
								static_cast<float>(swapchain->GetCreateInfoPtr()->imageExtent.width),
								static_cast<float>(swapchain->GetCreateInfoPtr()->imageExtent.height),
								0, 1};
		Shit::Rect2D scissor{{0, 0}, swapchain->GetCreateInfoPtr()->imageExtent};

		Shit::PipelineViewportStateCreateInfo viewportStateCreateInfo{1, &viewport, 1, &scissor};

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
			shaderStageCreateInfos.size(),
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
		uint32_t count = static_cast<uint32_t>(swapchainImageViews.size());
		Shit::CommandBufferCreateInfo cmdBufferCreateInfo{
			Shit::CommandBufferLevel::PRIMARY, count};
		commandBuffers.resize(count);
		commandPool->CreateCommandBuffers(cmdBufferCreateInfo, commandBuffers.data());
	}
	void recordCommandBuffers()
	{
		uint32_t count = static_cast<uint32_t>(swapchainImageViews.size());
		Shit::CommandBufferBeginInfo cmdBufferBeginInfo{};

		std::vector<Shit::ClearValue> clearValues{Shit::ClearColorValueFloat{0.2f, 0.2f, 0.2f, 1.f}};

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
			commandBuffers[i]->Reset();
			commandBuffers[i]->Begin(cmdBufferBeginInfo);
			commandBuffers[i]->BeginRenderPass(renderPassBeginInfo);
			commandBuffers[i]->BindPipeline({Shit::PipelineBindPoint::GRAPHICS, pipeline});
			uint64_t offsets[] = {0};
			Shit::Buffer *buffers[] = {vertexBuffer};
			commandBuffers[i]->BindVertexBuffers(
				Shit::BindVertexBuffersInfo{
					0,
					1,
					buffers,
					offsets});

			int drawMethod = 4;
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
				Shit::DrawIndexedIndirectCommand drawIndexedIndirectCmd{4, 1, 0, 0, 0};
				commandBuffers[i]->DrawIndexed(drawIndexedIndirectCmd);
			}
			break;
			case 4:
			{
				// draw index
				commandBuffers[i]->BindIndexBuffer(Shit::BindIndexBufferInfo{
					indexBuffer,
					0,
					Shit::IndexType::UINT16});
				commandBuffers[i]->DrawIndexedIndirect(
					Shit::DrawIndirectInfo{
						drawIndexedIndirectCmdBuffer,
						0,
						1,
						sizeof(Shit::DrawIndexedIndirectCommand)});
			}
			break;
			case 5:
			{
				// draw index
				commandBuffers[i]->BindIndexBuffer(Shit::BindIndexBufferInfo{
					indexBuffer,
					0,
					Shit::IndexType::UINT16});
				commandBuffers[i]->DrawIndexedIndirectCount(
					Shit::DrawIndirectCountInfo{
						drawIndexedIndirectCmdBuffer,
						0,
						drawCountBuffer,
						0,
						1,
						sizeof(Shit::DrawIndexedIndirectCommand)});
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
		// inFlightImages.resize(swapchainImages.size());
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

		std::vector<Shit::DrawIndexedIndirectCommand> drawIndexedIndirectCmds{{4, 1, 0, 0, 0}};

		// create draw indexed indirect command buffer
		Shit::BufferCreateInfo drawIndexedIndirectCmdBufferCreateInfo{
			{},
			sizeof(Shit::DrawIndexedIndirectCommand) * drawIndexedIndirectCmds.size(),
			Shit::BufferUsageFlagBits::INDIRECT_BUFFER_BIT | Shit::BufferUsageFlagBits::TRANSFER_DST_BIT,
			Shit::MemoryPropertyFlagBits::DEVICE_LOCAL_BIT};
		drawIndexedIndirectCmdBuffer = device->Create(drawIndexedIndirectCmdBufferCreateInfo, drawIndexedIndirectCmds.data());
	}
	void createVertexBuffer()
	{
		Shit::BufferCreateInfo createInfo{
			{},
			sizeof(vertices[0]) * vertices.size(),
			Shit::BufferUsageFlagBits::VERTEX_BUFFER_BIT | Shit::BufferUsageFlagBits::TRANSFER_DST_BIT,
			Shit::MemoryPropertyFlagBits::DEVICE_LOCAL_BIT};

		vertexBuffer = device->Create(createInfo, vertices.data());
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