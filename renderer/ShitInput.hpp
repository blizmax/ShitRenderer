/**
 * @file ShitInput.hpp
 * @author yangzs
 * @brief
 * @version 0.1
 *
 * @copyright Copyright (c) 2021
 *
 */

#pragma once
#include "ShitRendererPrerequisites.hpp"

#define SHIT_INPUT_EVENT_CONSTRUCTOR_IMPL(class_, type_) \
	class_() : InputEvent(type_) {}

namespace Shit
{
	struct MouseMoveEvent
	{
		int xpos;
		int ypos;
	};
	struct MouseEnterEvent
	{
		bool entered;
	};
	struct MouseButtonEvent
	{
		MouseButton button;
		bool state; // true down, false up
	};
	struct MouseWheelEvent
	{
		int xoffset; //+-1
		int yoffset; //+-1
	};
	struct KeyState
	{
		bool down;
		bool toggled;
	};
	struct KeyEvent
	{
		KeyCode keyCode;
		bool state;			// true down, false up
		bool previousState; // true down , false up
		bool contexCode;	// alt key true down, false up
	};
	struct CharEvent
	{
		uint32_t codepoint;
	};
	struct DropEvent
	{
		uint32_t count;
		char **paths;
	};
	struct WindowCreateEvent
	{
	};
	struct WindowCloseEvent
	{
	};
	struct WindowQuitEvent
	{
	};
	struct WindowResizeEvent
	{
		uint32_t width;
		uint32_t height;
	};
	struct WindowContentScaleEvent
	{
		float xscale;
		float yscale;
	};
	struct WindowPosEvent
	{
		int xpos;
		int ypos;
	};
	struct WindowIconifyEvent
	{
		bool iconified;
	};
	struct WindowMaximizeEvent
	{
		bool maximized;
	};
	struct WindowFocusEvent
	{
		bool focused;
	};
	struct WindowRefreshEvent
	{
	};

	struct Event
	{
		using EventType =
			std::variant<
				KeyEvent,
				MouseMoveEvent,
				MouseButtonEvent,
				MouseEnterEvent,
				CharEvent,
				MouseWheelEvent,
				DropEvent,

				WindowCreateEvent,
				WindowCloseEvent,
				WindowQuitEvent,
				WindowResizeEvent,
				WindowContentScaleEvent,
				WindowPosEvent,
				WindowIconifyEvent,
				WindowMaximizeEvent,
				WindowFocusEvent,
				WindowRefreshEvent>;

		EventModifierBits modifier;
		Window *pWindow;
		EventType value;
	};

	void InitEvent();
	uint32_t MapKey(KeyCode keycode);
	KeyCode MapKey(uint32_t keycode);
	EventModifierBits MapKeyModifier(uint32_t mods);

	KeyState GetKeyState(KeyCode keycode);

	bool SetCursorPos(int x, int y);

}