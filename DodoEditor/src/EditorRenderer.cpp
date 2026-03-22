#include "EditorRenderer.h"

void EditorRenderer::RenderEntities(EditorWorld& world, const Math::FreeCamera& camera, LightSystem& lightSystem,
                                    RenderAPI& renderAPI, AssetManager& assets)
{
    FrameData frameData;
    frameData.camera = camera.GetCameraMatrix();
    frameData.cameraPos = camera.GetPosition();
    frameData.lightCamera = lightSystem.m_Directional.m_LightCamera;
    frameData.lightDir = lightSystem.m_Directional.m_Direction;
    renderAPI.SetFrameData(frameData); // Uploads frame data to the GPU

    // Draw ModelComponent
    const auto& modelPool = world.template GetPool<ModelComponent>();
    for (const auto& modelComponent : modelPool.GetComponents()) {
        Model* model = assets.GetModel(modelComponent.m_ModelID);
        for (auto mesh : model->GetMeshes()) {
            Ref<Material> mat = mesh->GetMaterial();
            mat->Bind(renderAPI);
            renderAPI.SetDrawData({modelComponent.m_Transformation.m_Model});
            mesh->DrawGeometry(renderAPI);
        }
    }
}

void EditorRenderer::DrawScene(EditorScene* scene, const Math::FreeCamera& camera, RenderAPI& renderAPI,
                               AssetManager& assets)
{
    auto& world = scene->GetWorld();
    RenderEntities(world, camera, scene->m_LightSystem, renderAPI, assets);
    if (scene->m_SkyBox) scene->m_SkyBox->Draw(camera.GetViewMatrix(), renderAPI);
}