#include "EditorRenderer.h"

void EditorRenderer::RenderEntities(EditorWorld& world, const Math::FreeCamera& camera, LightSystem& lightSystem,
                                    RenderAPI& renderAPI, AssetManager& assets)
{
    FrameData frameData;
    frameData.camera = camera.GetCameraMatrix();
    frameData.skyboxCamera = camera.GetProjectionMatrix() * Math::Mat4::RelinquishToMat3(camera.GetViewMatrix());
    frameData.cameraPos = camera.GetPosition();
    frameData.lightCamera = lightSystem.m_Directional.m_LightCamera;
    frameData.lightDir = lightSystem.m_Directional.m_Direction;
    renderAPI.SetFrameData(frameData); // Uploads frame data to the GPU

    // Draw ModelComponent
    const auto& modelPool = world.template GetPool<ModelComponent>();
    for (const auto& modelComponent : modelPool.GetComponents()) {
        Ref<Model> model = assets.GetModel(modelComponent.m_ModelID);
        for (auto mesh : model->GetMeshes()) {
            Ref<Material> mat = mesh->GetMaterial();
            mat->Bind(renderAPI);
            const Math::Mat4& modelMatrix = modelComponent.m_Transformation.m_Model;
            const Math::Mat3 model3x3(
                Math::Vec3(modelMatrix.m_Columns[0].x, modelMatrix.m_Columns[0].y, modelMatrix.m_Columns[0].z),
                Math::Vec3(modelMatrix.m_Columns[1].x, modelMatrix.m_Columns[1].y, modelMatrix.m_Columns[1].z),
                Math::Vec3(modelMatrix.m_Columns[2].x, modelMatrix.m_Columns[2].y, modelMatrix.m_Columns[2].z));
            renderAPI.SetDrawData(
                {.model = modelMatrix, .normalMatrix = Math::Mat3::Transpose(Math::Mat3::Inverse(model3x3))});
            mesh->DrawGeometry(renderAPI);
        }
    }
}

void EditorRenderer::DrawScene(EditorScene* scene, const Math::FreeCamera& camera, RenderAPI& renderAPI,
                               AssetManager& assets)
{
    // Dodo::Scene* runtimeScene = &scene->GetRuntimeScene();
    // m_Renderer3D.DrawShadowedScene(runtimeScene, camera, renderAPI, assets);
    auto& world = scene->GetWorld();
    RenderEntities(world, camera, scene->m_LightSystem, renderAPI, assets);
    if (scene->m_SkyBox) scene->m_SkyBox->Draw(renderAPI);
}
