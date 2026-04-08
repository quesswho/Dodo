#include "Renderer3D.h"
#include "pch.h"

#include "Core/Application/Application.h"

namespace Dodo {

    Renderer3D::Renderer3D(RenderAPI& renderAPI, AssetManager& assets) : m_ShadowMap(new ShadowMap(renderAPI))
    {
        ShaderID id = assets.LoadShaderFromPath("res/shader/builtin/Passes/Shadow.slang");
        PipelineDesc shadowPipelineDesc;
        shadowPipelineDesc.shaderID = id;
        shadowPipelineDesc.culling = CullMode::Front;
        shadowPipelineDesc.depthOnly = true;
        PipelineID shadowPipelineID = assets.CreatePipeline(shadowPipelineDesc, renderAPI);
        m_ShadowMapMaterial = std::make_shared<Material>(assets.GetPipeline(shadowPipelineID));

        FrameBufferProperties frameprop;
        frameprop.m_Width = renderAPI.m_ViewportWidth;
        frameprop.m_Height = renderAPI.m_ViewportHeight;
        frameprop.m_FrameBufferType = FrameBufferType::FRAMEBUFFER_COLOR_DEPTH_STENCIL;
        m_PostEffect = new PostEffect(frameprop, "res/shader/builtin/Passes/Gamma.slang", renderAPI, assets);
    }

    void Renderer3D::RenderEntities(World& world, const Math::FreeCamera& camera, LightSystem& lightSystem,
                                    RenderAPI& renderAPI, AssetManager& assets)
    {
        FrameData frameData;
        frameData.camera = camera.GetCameraMatrix();
        frameData.cameraPos = camera.GetPosition();
        frameData.lightCamera = lightSystem.m_Directional.m_LightCamera;
        frameData.lightDir = lightSystem.m_Directional.m_Direction;
        renderAPI.SetFrameData(frameData); // Uploads frame data to the GPU

        // Draw ModelComponent
        const auto& modelPool = world.GetPool<ModelComponent>();
        for (const auto& modelComponent : modelPool.GetComponents()) {
            Ref<Model> model = assets.GetModel(modelComponent.m_ModelID);
            for (auto mesh : model->GetMeshes()) {
                Ref<Material> mat = mesh->GetMaterial();
                mat->Bind(renderAPI);
                renderAPI.SetDrawData(MakeDrawData(modelComponent.m_Transformation.m_Model));
                mesh->DrawGeometry(renderAPI);
            }
        }
    }

    void Renderer3D::RenderGeometry(World& world, RenderAPI& renderAPI, AssetManager& assets)
    {
        // Draw ModelComponents with custom material
        const auto& modelPool = world.GetPool<ModelComponent>();
        for (const auto& modelComponent : modelPool.GetComponents()) {
            renderAPI.SetDrawData(MakeDrawData(modelComponent.m_Transformation.m_Model));
            Ref<Model> model = assets.GetModel(modelComponent.m_ModelID);
            model->DrawGeometry(renderAPI);
        }
    }

    void Renderer3D::DrawScene(Scene* scene, const Math::FreeCamera& camera, RenderAPI& renderAPI, AssetManager& assets)
    {
        RenderEntities(scene->GetWorld(), camera, scene->m_LightSystem, renderAPI, assets);
        if (scene->m_SkyBox) scene->m_SkyBox->Draw(camera, renderAPI);
    }

    void Renderer3D::DrawShadowedScene(Scene* scene, const Math::FreeCamera& camera, RenderAPI& renderAPI,
                                       AssetManager& assets)
    {
        // Draw to shadowmap
        FrameData frameData;
        frameData.camera = camera.GetCameraMatrix();
        frameData.cameraPos = camera.GetPosition();
        frameData.lightCamera = scene->m_LightSystem.m_Directional.m_LightCamera;
        frameData.lightDir = scene->m_LightSystem.m_Directional.m_Direction;
        renderAPI.SetFrameData(frameData);

        // Bind target, shadow pipeline and draw geometry to shadowmap
        m_ShadowMap->Bind(renderAPI);
        renderAPI.BindPipeline(m_ShadowMapMaterial->GetShader());
        World& world = scene->GetWorld();
        RenderGeometry(world, renderAPI, assets);

        // Bind postfx render target
        m_PostEffect->Bind(renderAPI);

        // Bind shadowmap to index 3
        renderAPI.BindFrameBufferTexture(3, m_ShadowMap->GetFrameBuffer());
        DrawScene(scene, camera, renderAPI, assets);

        // Draw postfx to screen target
        m_PostEffect->Draw(renderAPI);
    }

    DrawData Renderer3D::MakeDrawData(const Math::Mat4& model)
    {
        const Math::Mat3 model3x3(Math::Vec3(model.m_Columns[0].x, model.m_Columns[0].y, model.m_Columns[0].z),
                                  Math::Vec3(model.m_Columns[1].x, model.m_Columns[1].y, model.m_Columns[1].z),
                                  Math::Vec3(model.m_Columns[2].x, model.m_Columns[2].y, model.m_Columns[2].z));

        return DrawData{
            .model = model,
            .normalMatrix = Math::Mat3::Transpose(Math::Mat3::Inverse(model3x3)),
        };
    }
} // namespace Dodo
