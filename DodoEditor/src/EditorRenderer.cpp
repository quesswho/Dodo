#include "EditorRenderer.h"

void EditorRenderer::RenderEntities(EditorWorld& world, Math::FreeCamera* camera, LightSystem& lightSystem,
                                    RenderAPI& renderAPI, AssetManager& assets)
{
    // Draw ModelComponent
    const auto& modelPool = world.template GetPool<ModelComponent>();
    for (const auto& modelComponent : modelPool.GetComponents()) {
        Model* model = assets.GetModel(modelComponent.m_ModelID);
        for (auto mesh : model->GetMeshes()) {
            Ref<Material> mat = mesh->GetMaterial();
            mat->Bind(renderAPI);
            mat->SetUniform("u_LightCamera", lightSystem.m_Directional.m_LightCamera);
            mat->SetUniform("u_LightDir", lightSystem.m_Directional.m_Direction);
            mat->SetUniform("u_Model", modelComponent.m_Transformation.m_Model);
            mat->SetUniform("u_Camera", camera->GetCameraMatrix());
            mat->SetUniform("u_CameraPos", camera->GetCameraPos());
            mesh->DrawGeometry(renderAPI);
        }
    }
}

void EditorRenderer::DrawScene(EditorScene* scene, RenderAPI& renderAPI, AssetManager& assets)
{
    auto& world = scene->GetWorld();
    RenderEntities(world, m_Camera, scene->m_LightSystem, renderAPI, assets);
    if (scene->m_SkyBox) scene->m_SkyBox->Draw(m_Camera->GetViewMatrix(), renderAPI);
}