#pragma once

#include "EditorWorld.h"
#include <Dodo.h>

class EditorScene {
  public:
    explicit EditorScene(Dodo::Math::FreeCamera* camera, const std::string& name = "Unnamed")
        : EditorScene(new Dodo::Scene(camera, name))
    {}

    explicit EditorScene(Dodo::Scene* runtimeScene)
        : m_RuntimeScene(runtimeScene), m_Name(m_RuntimeScene->m_Name), m_SkyBox(m_RuntimeScene->m_SkyBox),
          m_LightSystem(m_RuntimeScene->m_LightSystem), m_World(m_RuntimeScene->GetWorld(), m_Overlay)
    {}

    ~EditorScene() { delete m_RuntimeScene; }

    EditorWorld& GetWorld() { return m_World; }
    const EditorWorld& GetWorld() const { return m_World; }

    Dodo::Scene& GetRuntimeScene() { return *m_RuntimeScene; }
    const Dodo::Scene& GetRuntimeScene() const { return *m_RuntimeScene; }

    void UpdateCamera(Dodo::Math::FreeCamera* camera) { m_RuntimeScene->UpdateCamera(camera); }

  private:
    Dodo::Scene* m_RuntimeScene;
    Overlay m_Overlay;
    EditorWorld m_World;

  public:
    std::string& m_Name;
    Dodo::Skybox*& m_SkyBox;
    Dodo::LightSystem& m_LightSystem;
};
