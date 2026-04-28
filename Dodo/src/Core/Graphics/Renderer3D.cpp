#include "Renderer3D.h"

#include "Core/Application/Application.h"
#include "Core/Graphics/GpuTimings.h"

namespace Dodo {

    Renderer3D::Renderer3D(RenderAPI& renderAPI, AssetManager& assets) : m_ShadowMap(new ShadowMap(renderAPI))
    {
        ShaderID id = assets.LoadShaderFromPath("res/shader/builtin/Passes/Shadow.slang");
        PipelineDesc shadowPipelineDesc;
        shadowPipelineDesc.shaderID = id;
        shadowPipelineDesc.culling = CullMode::Front;
        shadowPipelineDesc.depthOnly = true;
        shadowPipelineDesc.vertexLayout =
            BufferProperties({{"POSITION", 3}, {"TEXCOORD", 2}, {"NORMAL", 3}, {"TANGENT", 4}});
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
        auto drawMesh = [&](const ModelComponent& mc, Ref<Mesh> mesh) {
            Ref<Material> mat = mesh->GetMaterial();
            mat->Bind(renderAPI);
            renderAPI.SetDrawData(MakeDrawData(mc.m_Transformation.m_Model));
            mesh->DrawGeometry(renderAPI);
        };

        auto isTransparent = [](Ref<Mesh> mesh) {
            auto shader = mesh->GetMaterial()->GetShader();
            return shader && shader->GetDesc().blendMode == BlendMode::AlphaBlend;
        };

        const auto& modelPool = world.GetPool<ModelComponent>();

        // Pass 1: opaque and alpha-masked geometry (depth write ON)
        for (const auto& mc : modelPool.GetComponents()) {
            for (auto mesh : assets.GetModel(mc.m_ModelID)->GetMeshes())
                if (!isTransparent(mesh)) drawMesh(mc, mesh);
        }

        // Pass 2: alpha-blended geometry (depth write OFF, reads depth populated by pass 1)
        for (const auto& mc : modelPool.GetComponents()) {
            for (auto mesh : assets.GetModel(mc.m_ModelID)->GetMeshes())
                if (isTransparent(mesh)) drawMesh(mc, mesh);
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
        FrameData frameData;
        frameData.camera = camera.GetCameraMatrix();
        frameData.skyboxCamera = camera.GetProjectionMatrix() * Math::Mat4::RelinquishToMat3(camera.GetViewMatrix());
        frameData.cameraPos = camera.GetPosition();

        frameData.lightCamera = scene->m_LightSystem.m_Directional.m_LightCamera;
        frameData.lightDir = scene->m_LightSystem.m_Directional.m_Direction;
        renderAPI.SetFrameData(frameData);

        RenderEntities(scene->GetWorld(), camera, scene->m_LightSystem, renderAPI, assets);
        if (scene->m_SkyBox) scene->m_SkyBox->Draw(renderAPI);
    }

    void Renderer3D::DrawShadowedScene(Scene* scene, const Math::FreeCamera& camera, RenderAPI& renderAPI,
                                       AssetManager& assets)
    {
        renderAPI.BeginTimestamp(GpuTimestampSlot::Frame);

        // Shadow pass: draw geometry depth to shadow framebuffer
        renderAPI.BeginTimestamp(GpuTimestampSlot::Shadow);
        FrameData shadowFrameData;
        shadowFrameData.lightCamera = scene->m_LightSystem.m_Directional.m_LightCamera;
        shadowFrameData.lightDir = scene->m_LightSystem.m_Directional.m_Direction;
        renderAPI.SetFrameData(shadowFrameData);
        m_ShadowMap->Bind(renderAPI);
        renderAPI.BindPipeline(m_ShadowMapMaterial->GetShader());
        World& world = scene->GetWorld();
        RenderGeometry(world, renderAPI, assets);
        renderAPI.EndTimestamp(GpuTimestampSlot::Shadow);

        // Scene pass: geometry + skybox to post-effect framebuffer
        renderAPI.BeginTimestamp(GpuTimestampSlot::Scene);
        m_PostEffect->Bind(renderAPI);
        renderAPI.BindFrameBufferTexture(3, m_ShadowMap->GetFrameBuffer());
        DrawScene(scene, camera, renderAPI, assets);
        renderAPI.EndTimestamp(GpuTimestampSlot::Scene);

        // Post-effect pass: full-screen composite to swapchain
        renderAPI.BeginTimestamp(GpuTimestampSlot::PostEffect);
        m_PostEffect->Draw(renderAPI);
        renderAPI.EndTimestamp(GpuTimestampSlot::PostEffect);

        renderAPI.EndTimestamp(GpuTimestampSlot::Frame);
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
