#include "Renderer3D.h"

#include "Core/Application/Application.h"
#include "Core/Graphics/GpuTimings.h"

namespace Dodo {

    Renderer3D::Renderer3D(RenderAPI& renderAPI, AssetManager& assets)
        : m_CascadedShadowMap(new CascadedShadowMap(renderAPI, 4, 4096))
    {
        ShaderID id = assets.LoadShaderFromPath("res/shader/builtin/Passes/Shadow.slang");
        PipelineDesc shadowPipelineDesc;
        shadowPipelineDesc.shaderID = id;
        shadowPipelineDesc.culling = CullMode::Back;
        shadowPipelineDesc.depthOnly = true;
        shadowPipelineDesc.depthClip = true;
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
        frameData.cameraView = camera.GetViewMatrix();
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

        // Shadow pass: each mesh is drawn with instanceCount = numCascades so the VS
        // fans the geometry across all cascade layers via SV_InstanceID / SV_RenderTargetArrayIndex.
        m_CascadedShadowMap->UpdateCamera(camera.GetProjectionMatrix(), camera.GetViewMatrix(),
        scene->m_LightSystem.m_Directional.m_Direction, camera.GetNearPlane(),
        camera.GetFarPlane(), camera.GetFov(), camera.GetAspectRatio());

        renderAPI.BeginTimestamp(GpuTimestampSlot::Shadow);
        
        CsmData csmData = m_CascadedShadowMap->GetCsmData();
        renderAPI.SetCSMData(csmData);
        FrameData shadowFrameData;
        shadowFrameData.lightCamera = csmData.lightSpaceMatrices[0]; // first cascade for FrameData compat
        shadowFrameData.lightDir = scene->m_LightSystem.m_Directional.m_Direction;
        renderAPI.SetFrameData(shadowFrameData);
        m_CascadedShadowMap->Bind(renderAPI);
        renderAPI.BindPipeline(m_ShadowMapMaterial->GetShader());
        World& world = scene->GetWorld();
        const auto& shadowModelPool = world.GetPool<ModelComponent>();
        for (const auto& modelComponent : shadowModelPool.GetComponents()) {
            renderAPI.SetDrawData(MakeDrawData(modelComponent.m_Transformation.m_Model));
            auto model = assets.GetModel(modelComponent.m_ModelID);

            // Skip transparent meshes in the shadow pass
            for (auto mesh : model->GetMeshes()) {
                auto shader = mesh->GetMaterial()->GetShader();
                if (shader && shader->GetDesc().blendMode == BlendMode::AlphaBlend) continue;
                mesh->DrawGeometryInstanced(renderAPI, csmData.numCascades);
            }
        }
        renderAPI.EndTimestamp(GpuTimestampSlot::Shadow);

        // Scene pass: geometry + skybox to post-effect framebuffer
        renderAPI.BeginTimestamp(GpuTimestampSlot::Scene);
        renderAPI.SetCSMData(csmData); // re-upload so fragment shader cascade selection is live
        m_PostEffect->Bind(renderAPI);
        renderAPI.BindFrameBufferTexture(3, m_CascadedShadowMap->GetFrameBuffer());
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
