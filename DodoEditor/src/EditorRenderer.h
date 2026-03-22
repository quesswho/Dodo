#pragma once

#include "Scene/EditorScene.h"
#include <Dodo.h>

using namespace Dodo;

class EditorRenderer {
  public:
    EditorRenderer() = default;

    ~EditorRenderer() = default;

    void DrawScene(EditorScene* scene, const Math::FreeCamera& camera, RenderAPI& renderAPI, AssetManager& assets);

    void RenderEntities(EditorWorld& world, const Math::FreeCamera& camera, LightSystem& lightSystem, RenderAPI& renderAPI,
                        AssetManager& assets);
};