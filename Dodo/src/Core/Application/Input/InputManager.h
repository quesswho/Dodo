#pragma once

#include <functional>

#include "Input.h"

#include <Core/Application/Event.h>
namespace Dodo {

    class InputManager {
      public:
        InputManager() = default;

        void KeyPressed(int key)
        {
            m_Input.SetKeyState(key, true);
            FireEvent(KeyPressEvent(key));
        }

        void KeyReleased(int key)
        {
            m_Input.SetKeyState(key, false);
            FireEvent(KeyReleaseEvent(key));
        }

        void MousePressed(int button)
        {
            m_Input.SetMouseState(button, true);
            FireEvent(MousePressEvent(button));
        }

        void MouseReleased(int button)
        {
            m_Input.SetMouseState(button, false);
            FireEvent(MouseReleaseEvent(button));
        }

        void MouseMoved(Math::TVec2<double> mousePos)
        {
            m_Input.SetMousePosition(mousePos);
            FireEvent(MouseMoveEvent(mousePos));
        }

        const Input &GetInput() const { return m_Input; }

        void SetEventCallback(std::function<void(const Event &)> cb) { m_EventCallback = cb; }

      private:
        void FireEvent(const Event &e)
        {
            if (m_EventCallback)
                m_EventCallback(e);
        }

        Input m_Input;
        std::function<void(const Event &)> m_EventCallback;
    };
} // namespace Dodo