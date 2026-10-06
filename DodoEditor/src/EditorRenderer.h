#pragma once

#include "Scene/EditorScene.h"
#include <Dodo.h>

using namespace Dodo;

class EditorRenderer {
  public:
    /**
     * @param output Framebuffer shown in the viewport panel, the finished image is drawn into it.
     */
    EditorRenderer(RenderAPI& renderAPI, AssetManager& assets, Ref<FrameBuffer> output);

    ~EditorRenderer() = default;

    /**
     * Draws the scene with shadows and tonemapping into the output framebuffer, which stays bound afterwards.
     */
    void DrawScene(EditorScene* scene, const Math::FreeCamera& camera, RenderAPI& renderAPI, AssetManager& assets);

    /**
     * Call when the size of the output framebuffer changes.
     */
    void Resize(uint width, uint height) { m_Renderer3D.Resize(width, height); }

  private:
    Renderer3D m_Renderer3D;
};
