#pragma once

#include "Scene/EditorScene.h"
#include <Dodo.h>

using namespace Dodo;

class EditorRenderer {
    Math::FreeCamera* m_Camera;

  public:
    EditorRenderer(Math::FreeCamera* camera) : m_Camera(camera) {}

    ~EditorRenderer() {}

    void DrawScene(EditorScene* scene, RenderAPI& renderAPI, AssetManager& assets);

    void RenderEntities(EditorWorld& world, Math::FreeCamera* camera, LightSystem& lightSystem, RenderAPI& renderAPI,
                        AssetManager& assets);

    void UpdateCamera(Math::FreeCamera* camera) { m_Camera = camera; }
};