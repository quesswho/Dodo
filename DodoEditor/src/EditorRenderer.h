#pragma once

#include "Scene/EditorScene.h"
#include <Dodo.h>

using namespace Dodo;

class EditorRenderer {
  public:
    EditorRenderer(RenderAPI& renderAPI, AssetManager& assets) : m_Renderer3D(renderAPI, assets) {};

    ~EditorRenderer() = default;

    void RenderEntities(EditorWorld& world, const Math::FreeCamera& camera, LightSystem& lightSystem,
                        RenderAPI& renderAPI, AssetManager& assets);

    void DrawScene(EditorScene* scene, const Math::FreeCamera& camera, RenderAPI& renderAPI, AssetManager& assets);

  private:
    Renderer3D m_Renderer3D;
};