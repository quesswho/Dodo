#pragma once
#include <Core/Math/Vector/Vec2.h>
#include <bitset>

namespace Dodo {

    class Input {
      public:
        static constexpr int MAX_KEYS = 512;
        static constexpr int MAX_MOUSE_BUTTONS = 16;

        // Keyboard
        void SetKeyState(int key, bool pressed)
        {
            if (IsValidKey(key))
                m_Keys[key] = pressed;
        }
        bool IsKeyPressed(int key) const { return IsValidKey(key) && m_Keys[key]; }

        // Mouse
        void SetMouseState(int button, bool pressed)
        {
            if (IsValidMouse(button))
                m_Mouse[button] = pressed;
        }
        bool IsMousePressed(int button) const { return IsValidMouse(button) && m_Mouse[button]; }

        // Mouse pos
        void SetMousePosition(Math::TVec2<double> mousePos) { m_MousePos = mousePos; }
        Math::TVec2<double> GetMousePosition() const { return m_MousePos; }

        void Reset()
        {
            m_Keys.reset();
            m_Mouse.reset();
        }

      private:
        std::bitset<MAX_KEYS> m_Keys;
        std::bitset<MAX_MOUSE_BUTTONS> m_Mouse;
        Math::TVec2<double> m_MousePos;

        bool IsValidKey(int key) const { return key >= 0 && key < MAX_KEYS; }
        bool IsValidMouse(int button) const { return button >= 0 && button < MAX_MOUSE_BUTTONS; }
    };
} // namespace Dodo
