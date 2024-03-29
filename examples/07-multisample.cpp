#include "common/common.h"
#include <stb_image.h>

uint32_t WIDTH = 800, HEIGHT = 600;

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

const char *testImagePath = SHIT_SOURCE_DIR "/assets/images/00000010.jpg";

const Shit::SampleCountFlagBits SAMPLE_COUNT = Shit::SampleCountFlagBits::BIT_4;

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
	{{-0.5, -0.5, 0}, {1, 0, 0}, {0, 0}},
	{{0.5, -0.5, 0}, {0, 1, 0}, {1, 0}},
	{{-0.5, 0.5, 0}, {0, 0, 1}, {0, 1}},
	{{0.5, 0.5, 0}, {1, 1, 0}, {1, 1}},
};
std::vector<uint16_t> indices{0, 1, 2, 3};

struct alignas(16) UBO_MVP
{
	glm::mat4 M;
	glm::mat4 PV;
};

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
	// std::vector<Shit::Fence  *> inFlightImages;

	Shit::Shader *vertShader;
	Shit::Shader *fragShader;

	Shit::DescriptorPool *descriptorPool;
	std::vector<Shit::DescriptorSetLayout *> descriptorSetLayouts;
	std::vector<Shit::DescriptorSet *> descriptorSets;

	Shit::PipelineLayout *pipelineLayout;
	Shit::RenderPass *renderPass;
	Shit::Pipeline *pipeline;

	Shit::Buffer *drawIndirectCmdBuffer;
	Shit::Buffer *drawCountBuffer;
	Shit::Buffer *drawIndexedIndirectCmdBuffer;
	Shit::Buffer *indexBuffer;
	Shit::Buffer *vertexBuffer;

	Shit::Image *testImage;
	Shit::ImageView *testImageView;
	Shit::Sampler *sampler;

	std::vector<Shit::Buffer *> uboMVPBuffers;

	Shit::Image *colorImage;
	Shit::ImageView *colorImageView;

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
		// window->AddEventListener(std::bind(&Hello::ProcessEvent, this,std::placeholders::_1));
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

		createSwapchain();

		createShaders();
		createDescriptorSets();
		createRenderPasses();
		createColorResources();

		pipelineLayout = device->Create(Shit::PipelineLayoutCreateInfo{(uint32_t)descriptorSetLayouts.size(), descriptorSetLayouts.data()});
		createPipeline();
		createFramebuffers();
		createSyncObjects();

		createDrawCommandBuffers();
		createIndexBuffer();
		createVertexBuffer();
		createImages();
		createSamplers();
		createUBOMVPBuffer();

		updateDescriptorSets();

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
		device->Destroy(colorImage);
		device->Destroy(colorImageView);
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
		createColorResources();
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
		ret = presentQueue->Present(presentInfo);
		if (ret == Shit::Result::SHIT_ERROR_OUT_OF_DATE || ret == Shit::Result::SUBOPTIMAL)
		{
			presentQueue->WaitIdle();
			recreateSwapchain();
		}
		else if (ret != Shit::Result::SUCCESS)
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
layout(location = 2) in vec2 inTexCoord;\n\
struct VS_OUT { \n\
	vec3 color;\n\
	vec2 texCoord;\n\
};\n\
layout(location = 0) out VS_OUT vs_out;\n\
layout(binding=1) uniform MVP{\n\
	mat4 M;\n\
	mat4 PV;\n\
};\n\
void main() \n\
{\n\
  gl_Position = PV*M*vec4(inPos, 1);\n\
  vs_out.color = inColor;\n\
  #ifdef VULKAN\n\
  vs_out.texCoord= vec2(inTexCoord.x,1.-inTexCoord.y);\n\
  #else\n\
  vs_out.texCoord= inTexCoord;\n\
  #endif\n\
}\n\
";
		std::string fragCode = "\
#version 460\n\
layout(location = 0) out vec4 outColor;\n\
struct VS_OUT { \n\
	vec3 color;\n\
	vec2 texCoord;\n\
};\n\
layout(location = 0) in VS_OUT fs_in;\n\
layout(binding=0) uniform sampler2D tex;\n\
void main() \n\
{\n\
  outColor = texture(tex,fs_in.texCoord);\n\
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

		swapchainCreateInfo = Shit::SwapchainCreateInfo{
			2,
			swapchainFormat.format,
			swapchainFormat.colorSpace,
			{800, 600},
			1,
			Shit::ImageUsageFlagBits::COLOR_ATTACHMENT_BIT,
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
	void createDescriptorSets()
	{
		std::vector<Shit::DescriptorSetLayoutBinding> bindings{
			Shit::DescriptorSetLayoutBinding{
				.binding = 0,
				.descriptorType = Shit::DescriptorType::COMBINED_IMAGE_SAMPLER,
				.descriptorCount = 1,
				.stageFlags = Shit::ShaderStageFlagBits::FRAGMENT_BIT,
			},
			Shit::DescriptorSetLayoutBinding{
				.binding = 1,
				.descriptorType = Shit::DescriptorType::UNIFORM_BUFFER,
				.descriptorCount = 1,
				.stageFlags = Shit::ShaderStageFlagBits::VERTEX_BIT,
			},
		};
		auto imageCount = swapchain->GetImageCount();
		descriptorSetLayouts.resize(
			imageCount,
			device->Create(Shit::DescriptorSetLayoutCreateInfo{(uint32_t)bindings.size(), bindings.data()}));

		std::vector<Shit::DescriptorPoolSize> poolSizes{
			{Shit::DescriptorType::COMBINED_IMAGE_SAMPLER, imageCount},
			{Shit::DescriptorType::UNIFORM_BUFFER, imageCount}};
		Shit::DescriptorPoolCreateInfo descriptorPoolCreateInfo{
			static_cast<uint32_t>(descriptorSetLayouts.size()),
			static_cast<uint32_t>(poolSizes.size()),
			poolSizes.data()};
		descriptorPool = device->Create(descriptorPoolCreateInfo);

		Shit::DescriptorSetAllocateInfo allocInfo{(uint32_t)descriptorSetLayouts.size(), descriptorSetLayouts.data()};
		descriptorPool->Allocate(allocInfo, descriptorSets);
	}
	void updateDescriptorSets()
	{
		Shit::DescriptorImageInfo imagesInfo[]{
			Shit::DescriptorImageInfo{sampler, testImageView, Shit::ImageLayout::SHADER_READ_ONLY_OPTIMAL},
		};
		Shit::DescriptorBufferInfo buffersInfo[]{
			{nullptr, 0, sizeof(UBO_MVP)},
		};
		std::vector<Shit::WriteDescriptorSet> writes{
			Shit::WriteDescriptorSet{
				nullptr, // pDstset
				0,		 // dst binding
				0,		 // dstArrayElement
				1,
				Shit::DescriptorType::COMBINED_IMAGE_SAMPLER,
				&imagesInfo[0],
			},
			Shit::WriteDescriptorSet{
				nullptr, // pDstset
				1,		 // dst binding
				0,		 // dstArrayElement
				1,
				Shit::DescriptorType::UNIFORM_BUFFER,
				nullptr,
				&buffersInfo[0],
			},
		};
		for (size_t i = 0, n = descriptorSets.size(); i < n; ++i)
		{
			for (auto &&a : writes)
				a.pDstSet = descriptorSets[i];
			buffersInfo[0].pBuffer = uboMVPBuffers[i];
			writes[1].pBufferInfo = &buffersInfo[0];
			device->UpdateDescriptorSets(writes, {});
		}
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
			 Shit::ImageLayout::UNDEFINED, // inital layout
			 Shit::ImageLayout::COLOR_ATTACHMENT_OPTIMAL},
			{swapchain->GetCreateInfoPtr()->format,
			 Shit::SampleCountFlagBits::BIT_1,
			 Shit::AttachmentLoadOp::CLEAR,
			 Shit::AttachmentStoreOp::DONT_CARE,
			 Shit::AttachmentLoadOp::DONT_CARE,
			 Shit::AttachmentStoreOp::DONT_CARE,
			 Shit::ImageLayout::UNDEFINED, // inital layout
			 Shit::ImageLayout::PRESENT_SRC},
		};

		std::vector<Shit::AttachmentReference> colorAttachments{
			{0, // the index of attachment description
			 Shit::ImageLayout::COLOR_ATTACHMENT_OPTIMAL},
		};
		std::vector<Shit::AttachmentReference> resolveAttachments{
			{1,
			 Shit::ImageLayout::COLOR_ATTACHMENT_OPTIMAL}};

		std::vector<Shit::SubpassDescription> subPasses{
			Shit::SubpassDescription{
				Shit::PipelineBindPoint::GRAPHICS,
				0,
				nullptr,
				static_cast<uint32_t>(colorAttachments.size()),
				colorAttachments.data(),
				resolveAttachments.data(),
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
			vertexAttributeDesc.size(),
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
			SAMPLE_COUNT,
			true,	 // sample shading
			1.f,	 // min sample shading
			nullptr, // mask
			false,	 // alpha to coverage
			false	 // alpha to one
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
		framebufferCreateInfo.attachmentCount = renderPass->GetCreateInfoPtr()->attachmentCount;
		for (uint32_t i = 0; i < count; ++i)
		{
			Shit::ImageView *imageViews[] = {colorImageView, swapchainImageViews[i]};
			framebufferCreateInfo.pAttachments = imageViews;
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

		std::vector<Shit::ClearValue> clearValues{
			std::array<float, 4>{0.2f, 0.2f, 0.2f, 1.f},
			std::array<float, 4>{0.2f, 0.2f, 0.2f, 1.f},
		};

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
			uint64_t offsets[] = {0};
			Shit::Buffer *buffers[] = {vertexBuffer};
			commandBuffers[i]->BindVertexBuffers(
				Shit::BindVertexBuffersInfo{
					0,
					1,
					buffers,
					offsets});

			Shit::BindDescriptorSetsInfo info{
				Shit::PipelineBindPoint::GRAPHICS,
				pipelineLayout,
				0,
				1,
				&descriptorSets[i]};
			commandBuffers[i]->BindDescriptorSets(info);

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
	void createImages()
	{
		int width, height, components;
		stbi_set_flip_vertically_on_load((renderSystem->GetCreateInfo()->version & Shit::RendererVersion::TypeBitmask) == Shit::RendererVersion::GL);
		auto pixels = stbi_load(testImagePath, &width, &height, &components, 4); // force load an alpha channel,even not exist
		if (!pixels)
			throw std::runtime_error("failed to load texture image!");

		Shit::ImageCreateInfo imageCreateInfo{
			//.flags=Shit::ImageCreateFlagBits::MUTABLE_FORMAT_BIT,
			.imageType = Shit::ImageType::TYPE_2D,
			.format = Shit::Format::R8G8B8A8_SRGB,
			.extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1},
			.arrayLayers = 1,
			.samples = Shit::SampleCountFlagBits::BIT_1,
			.tiling = Shit::ImageTiling::OPTIMAL,
			.usageFlags = Shit::ImageUsageFlagBits::SAMPLED_BIT,
			.memoryPropertyFlags = Shit::MemoryPropertyFlagBits::DEVICE_LOCAL_BIT,
			.initialLayout = Shit::ImageLayout::SHADER_READ_ONLY_OPTIMAL};

		testImage = device->Create(imageCreateInfo, Shit::ImageAspectFlagBits::COLOR_BIT, pixels);
		testImage->GenerateMipmap(Shit::Filter::LINEAR, imageCreateInfo.initialLayout, Shit::ImageLayout::SHADER_READ_ONLY_OPTIMAL);

		Shit::ImageViewCreateInfo imageViewCreateInfo{
			.pImage = testImage,
			.viewType = Shit::ImageViewType::TYPE_2D,
			//.format = Shit::Format::R8G8B8A8_UNORM,
			.format = Shit::Format::R8G8B8A8_SRGB,
			.subresourceRange = {Shit::ImageAspectFlagBits::COLOR_BIT, 0, testImage->GetCreateInfoPtr()->mipLevels, 0, 1},
		};
		testImageView = device->Create(imageViewCreateInfo);
		stbi_image_free(pixels);
	}
	void createSamplers()
	{
		Shit::SamplerCreateInfo samplerCreateInfo{
			.magFilter = Shit::Filter::LINEAR,
			.minFilter = Shit::Filter::LINEAR,
			.mipmapMode = Shit::SamplerMipmapMode::LINEAR,
			.wrapModeU = Shit::SamplerWrapMode::REPEAT,
			.wrapModeV = Shit::SamplerWrapMode::REPEAT,
			.wrapModeW = Shit::SamplerWrapMode::REPEAT,
			.minLod = 0.f,
			.maxLod = 100.f,
		};
		sampler = device->Create(samplerCreateInfo);
	}
	void createUBOMVPBuffer()
	{
		UBO_MVP uboMVP{
			glm::rotate(glm::mat4(1), glm::radians(15.f), glm::vec3(0, 0, 1)),
			glm::perspective(
				glm::radians(45.f),
				float(swapchain->GetCreateInfoPtr()->imageExtent.width) / float(swapchain->GetCreateInfoPtr()->imageExtent.height),
				1.f, 10.f) *
				glm::lookAt(glm::vec3(0, 0, 2), glm::vec3(0), glm::vec3(0, 1, 0)),
		};
		Shit::BufferCreateInfo bufferCreateInfo{
			.size = sizeof(UBO_MVP),
			.usage = Shit::BufferUsageFlagBits::UNIFORM_BUFFER_BIT,
			.memoryPropertyFlags = Shit::MemoryPropertyFlagBits::HOST_COHERENT_BIT | Shit::MemoryPropertyFlagBits::HOST_VISIBLE_BIT};
		uboMVPBuffers.resize(swapchain->GetImageCount());
		for (auto &&e : uboMVPBuffers)
			e = device->Create(bufferCreateInfo, &uboMVP);
	}
	void createColorResources()
	{
		Shit::ImageCreateInfo createInfo{
			{},
			Shit::ImageType::TYPE_2D,
			swapchain->GetCreateInfoPtr()->format,
			{swapchain->GetCreateInfoPtr()->imageExtent.width, swapchain->GetCreateInfoPtr()->imageExtent.height, 1},
			1,
			1,
			SAMPLE_COUNT,
			Shit::ImageTiling::OPTIMAL,
			Shit::ImageUsageFlagBits::COLOR_ATTACHMENT_BIT,
			Shit::MemoryPropertyFlagBits::DEVICE_LOCAL_BIT,
		};
		colorImage = device->Create(createInfo);
		Shit::ImageViewCreateInfo imageViewCreateInfo{
			colorImage,
			Shit::ImageViewType::TYPE_2D,
			swapchain->GetCreateInfoPtr()->format,
			{},
			{Shit::ImageAspectFlagBits::COLOR_BIT, 0, 1, 0, 1}};
		colorImageView = device->Create(imageViewCreateInfo);
	}
};
#include "common/entry.h"
EXAMPLE_MAIN(Hello)