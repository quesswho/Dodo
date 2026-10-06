#include "EditorRenderer.h"

EditorRenderer::EditorRenderer(RenderAPI& renderAPI, AssetManager& assets, Ref<FrameBuffer> output)
    : m_Renderer3D(renderAPI, assets, output)
{
    m_Renderer3D.SetEffectData({.gamma = 2.2f, .exposure = 1.5f});
}

void EditorRenderer::DrawScene(EditorScene* scene, const Math::FreeCamera& camera, RenderAPI& renderAPI,
                               AssetManager& assets)
{
    m_Renderer3D.DrawShadowedScene(&scene->GetRuntimeScene(), camera, renderAPI, assets);
}
