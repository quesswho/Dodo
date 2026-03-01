#pragma once

#include "pch.h"

#include "Input/InputManager.h"
#include "Layer.h"
#include "Window.h"

#include "Event.h"

#include "Core/Data/AssetManager.h"
#include "Core/Graphics/RenderAPI.h"
#include "Core/System/ThreadManager.h"

namespace Dodo {

    struct ApplicationConfig {
        WindowProperties m_WindowProperties;
    };

    class Application {
      public:
        static Application *s_Application;

        Application(const ApplicationConfig &conf);
        virtual ~Application();

        void CoreInit(const ApplicationConfig &conf);
        void Run(); // Main loop is contained in here
        void Shutdown();

        void PushLayer(Layer *layer);
        void PopLayer(Layer *layer);

        void OnEvent(const Event &event);

        void ImGuiNewFrame();
        void ImGuiEndFrame();

        virtual void Init();
        virtual void Update(float elapsed);

        const WindowProperties &GetWindowProperties() const { return m_Window->GetWindowProperties(); }
        const Input &GetInput() const { return m_InputManager.GetInput(); }

      public:
        float m_FrameTimeMs;
        uint m_FramesPerSecond;

        InputManager m_InputManager;
        Logger *m_Logger;
        Window *m_Window;
        RenderAPI *m_RenderAPI;
        ThreadManager *m_ThreadManager;
        AssetManager *m_AssetManager;
        bool m_Initializing;

      private:
        double m_TotalPhysMemGbs;
        std::string m_CpuBrand;

        std::vector<Layer *> m_Layers;

        float m_FrameTime;
        bool m_Closed;
        uint m_NumLogicalProcessors;
    };
} // namespace Dodo