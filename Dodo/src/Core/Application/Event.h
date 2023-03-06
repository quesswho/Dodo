#pragma once

#include <Core/Common.h>

#include "Core/Math/Vector/Vec2.h"

namespace Dodo {

	enum class EventType {
		KEY_PRESSED,
		KEY_RELEASED,
		MOUSE_PRESSED,
		MOUSE_RELEASED,
		MOUSE_SCROLL,
		MOUSE_POSITION,
		WINDOW_RESIZE,
		WINDOW_FOCUS,
		WINDOW_CLOSE
	};

	class Event {
	private:
		EventType m_EventType;
	public:
		bool m_Handled;

		Event(EventType type) 
			: m_EventType(type), m_Handled(false)
		{}
		
		EventType GetType() const { return m_EventType; }
	};

	struct KeyPressEvent : public Event {
		uint m_Key;

		KeyPressEvent(uint keycode)
			: Event(EventType::KEY_PRESSED), m_Key(keycode)
		{}
	};

	struct KeyReleaseEvent : public Event {
		uint m_Key;

		KeyReleaseEvent(uint keycode)
			: Event(EventType::KEY_RELEASED), m_Key(keycode)
		{}
	};

	struct MousePressEvent : public Event {
		uint m_Key;

		MousePressEvent(uint keycode)
			: Event(EventType::MOUSE_PRESSED), m_Key(keycode)
		{}
	};


	struct MouseReleaseEvent : public Event {
		uint m_Key;

		MouseReleaseEvent(uint keycode)
			: Event(EventType::MOUSE_RELEASED), m_Key(keycode)
		{}
	};

	struct MouseScrollEvent : public Event {
		Math::TVec2<double> m_MouseScrollOffset;

		MouseScrollEvent(const Math::TVec2<double>& mouseScrollOffset)
			: Event(EventType::MOUSE_SCROLL), m_MouseScrollOffset(mouseScrollOffset)
		{}
	};

	struct MouseMoveEvent : public Event {
		Math::TVec2<long> m_MousePos;

		MouseMoveEvent(const Math::TVec2<long>& mousePos)
			: Event(EventType::MOUSE_POSITION), m_MousePos(mousePos)
		{}
	};

	struct WindowResizeEvent : public Event {
		Math::TVec2<int> m_ScreenSize;

		WindowResizeEvent(const Math::TVec2<int>& screenSize)
			: Event(EventType::WINDOW_RESIZE), m_ScreenSize(screenSize)
		{}
	};

	struct WindowFocusEvent : public Event {
		bool m_Focus;

		WindowFocusEvent(bool focus)
			: Event(EventType::WINDOW_FOCUS), m_Focus(focus)
		{}
	};

	struct WindowCloseEvent : public Event {

		WindowCloseEvent()
			: Event(EventType::WINDOW_CLOSE)
		{}
	};
}