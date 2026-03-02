#pragma once

#include "Scene/EditorScene.h"
#include <Dodo.h>

using namespace Dodo;

class EditorRenderer {
    Math::FreeCamera* m_Camera;

  public:
    EditorRenderer(Math::FreeCamera* camera) : m_Camera(camera) {}

    ~EditorRenderer() {}

    void DrawScene(EditorScene* scene);

    void DrawGenericScene(EditorScene* scene)
    {
        auto& world = scene->GetWorld();
        RenderEntities(world, m_Camera, scene->m_LightSystem, *Application::s_Application->m_AssetManager);
        if (scene->m_SkyBox) scene->m_SkyBox->Draw(m_Camera->GetViewMatrix());
    }

    void RenderEntities(EditorWorld& world, Math::FreeCamera* camera, LightSystem& lightSystem, AssetManager& assets);

    void UpdateCamera(Math::FreeCamera* camera) { m_Camera = camera; }
};