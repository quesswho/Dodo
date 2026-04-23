#include "Scene.h"

#include "Core/Application/Application.h"

namespace Dodo {

    Scene::Scene(std::string name)
        : m_Name(name), m_SkyBox(0),
          m_World(new World()) // Math::Mat4::Perspective(45.0f, Application::s_Application->m_WindowProperties.m_Width
                               // / Application::s_Application->m_WindowProperties.m_Height, 0.01f, 100.0f))
    {}

    Scene::~Scene()
    {
        delete m_World;
    }

    World& Scene::GetWorld()
    {
        return *m_World;
    }
} // namespace Dodo