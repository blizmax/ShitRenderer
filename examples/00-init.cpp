#include "common/common.h"

uint32_t WIDTH = 800, HEIGHT = 600;

class Hello
{
	Shit::RenderSystem *renderSystem;
	Shit::Window *window;

public:
	void initRenderSystem()
	{
		Shit::RenderSystemCreateInfo renderSystemCreateInfo{
			g_RendererVersion,
			// Shit::RendererVersion::VULKAN,
			Shit::RenderSystemCreateFlagBits::CONTEXT_DEBUG_BIT};

		renderSystem = LoadRenderSystem(renderSystemCreateInfo);
		//1. create window
		//auto func=std::make_shared<void()>
		Shit::WindowCreateInfo windowCreateInfo{
			{},
			__WFILE__,
			{{DEFAULT_WINDOW_X, DEFAULT_WINDOW_Y},
			 {DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT}},
			std::bind(&Hello::ProcessEvent, this, std::placeholders::_1)};
		window = renderSystem->CreateRenderWindow(windowCreateInfo);
	}

	/**
	 * @brief process window event, do not write render code here
	 * 
	 * @param ev 
	 */
	void ProcessEvent(const Shit::Event &ev)
	{
		std::visit(Shit::overloaded{
					   [](auto &&) {},
					   [&ev](const Shit::KeyEvent &value)
					   {
						   if (value.keyCode == Shit::KeyCode::KEY_ESCAPE)
							   ev.pWindow->Close();
					   },
				   },
				   ev.value);
	}
	void mainLoop()
	{
		while (window->PollEvents())
		{
		}
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
};
#include "common/entry.h"
EXAMPLE_MAIN(Hello)