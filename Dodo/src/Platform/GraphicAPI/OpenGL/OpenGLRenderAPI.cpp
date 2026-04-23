#include "OpenGLRenderAPI.h"

#include "Core/Application/Application.h"
#include "OpenGLShaderCompiler.h"
#include <backends/imgui_impl_opengl3.h>

namespace Dodo::Platform {

    OpenGLRenderAPI::OpenGLRenderAPI(const NativeWindowHandle& handle)
        : m_Handle(handle), m_GPUInfo(""), m_VramKbs(0), m_ViewportWidth(0), m_ViewportHeight(0), m_ViewportPosX(0),
          m_ViewportPosY(0)
    {}

    OpenGLRenderAPI::~OpenGLRenderAPI()
    {
        if (m_ImGuiLoaded) {
            ImGui_ImplOpenGL3_Shutdown();
        }
        glDeleteBuffers(1, &m_FrameUBO);
        glDeleteBuffers(1, &m_ModelUBO);
        glDeleteBuffers(1, &m_PushConstantUBO);
        gladLoaderUnloadGL();
    }

    RenderInitError OpenGLRenderAPI::Init(const WindowProperties& winprop)
    {
        m_Context.CreateContextImpl(m_Handle); // Run glad loader
        m_Version = m_Context.LoadGlad();
        std::string versionStr = reinterpret_cast<const char*>(glGetString(GL_VERSION));
        DD_INFO("OPENGL: {0}", versionStr);
        if (GLAD_VERSION_MAJOR(m_Version) <= 3) {
            return RenderInitError(RenderInitStatus::Failed, "OpenGL version < 4.0 is not supported!");
        }

        glFrontFace(GL_CCW);
        glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE); // Vulkan style depth range [0, 1]
        glEnable(GL_MULTISAMPLE);

        SetViewport(winprop.m_FrameBufferWidth, winprop.m_FrameBufferHeight);
        m_CullingDefault = winprop.m_Settings.backfaceCull;

        m_Context.SetVSync(winprop.m_Settings.vsync);

        std::string vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
        std::string renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));

        glGetIntegerv(0x9048, &m_VramKbs);
        m_GPUInfo = "Vendor: " + vendor + " Renderer: " + renderer;
        m_GPUInfo.append(" VRAM: ")
            .append(StringUtils::KiloByte((size_t)m_VramKbs))
            .append(" : Opengl Version: ")
            .append(versionStr);

        DD_INFO("{}", m_GPUInfo);

        if (winprop.m_Settings.imgui) {
            m_Context.InitializeImGui();
            ImGui_ImplOpenGL3_Init();
            m_ImGuiLoaded = true;
        }

        // Create frame UBO
        glGenBuffers(1, &m_FrameUBO);
        glBindBuffer(GL_UNIFORM_BUFFER, m_FrameUBO);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(FrameData), nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_FrameUBO); // Bind to slot 0
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        // Create model UBO
        glGenBuffers(1, &m_ModelUBO);
        glBindBuffer(GL_UNIFORM_BUFFER, m_ModelUBO);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(DrawDataUBO), nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_ModelUBO); // Bind to slot 1
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        // Create push constant UBO
        glGenBuffers(1, &m_PushConstantUBO);
        glBindBuffer(GL_UNIFORM_BUFFER, m_PushConstantUBO);
        glBufferData(GL_UNIFORM_BUFFER, 128, nullptr, GL_DYNAMIC_DRAW); // 128 byte Vulkan guaranteed min
        glBindBufferBase(GL_UNIFORM_BUFFER, 2, m_PushConstantUBO);      // binding point 2
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        return RenderInitError(RenderInitStatus::Success, "");
    }

    void OpenGLRenderAPI::BindPipeline(Ref<Pipeline> pipeline)
    {
        m_CurrentPipelineID = pipeline->m_ShaderID;
        m_CurrentPipeline = pipeline;
        glUseProgram(pipeline->m_ShaderID);

        // Apply pipeline state from desc
        switch (pipeline->m_Desc.depthMode) {
        case DepthMode::None:
            glDisable(GL_DEPTH_TEST);
            break;
        case DepthMode::Never:
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_NEVER);
            break;
        case DepthMode::Less:
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);
            break;
        case DepthMode::Equal:
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_EQUAL);
            break;
        case DepthMode::LessEqual:
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
            break;
        case DepthMode::Greater:
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_GREATER);
            break;
        case DepthMode::NotEqual:
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_NOTEQUAL);
            break;
        case DepthMode::GreaterEqual:
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_GEQUAL);
            break;
        case DepthMode::Always:
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_ALWAYS);
            break;
        }

        glDepthMask(pipeline->m_Desc.depthWrite ? GL_TRUE : GL_FALSE);

        switch (pipeline->m_Desc.blendMode) {
        case BlendMode::None:
            glDisable(GL_BLEND);
            break;
        case BlendMode::Opaque:
            glDisable(GL_BLEND);
            break;
        case BlendMode::AlphaBlend:
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            break;
        case BlendMode::Additive:
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            break;
        case BlendMode::Multiply:
            glEnable(GL_BLEND);
            glBlendFunc(GL_DST_COLOR, GL_ZERO);
            break;
        }

        switch (pipeline->m_Desc.culling) {
        case CullMode::None:
            glDisable(GL_CULL_FACE);
            break;
        case CullMode::Back:
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            break;
        case CullMode::Front:
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
            break;
        }

        if (pipeline->m_Desc.stencilTest) {
            glEnable(GL_STENCIL_TEST);
        } else {
            glDisable(GL_STENCIL_TEST);
        }
    }

    void OpenGLRenderAPI::PushConstants(const void* data, size_t size)
    {
        if (!m_CurrentPipeline) return;
        for (const auto& loc : m_CurrentPipeline->m_PushConstantLocs) {
            if (loc.location == -1) continue;
            if (loc.offset + loc.elementCount * sizeof(float) > size) continue;

            const float* fptr = reinterpret_cast<const float*>(static_cast<const uint8_t*>(data) + loc.offset);
            const int* iptr = reinterpret_cast<const int*>(static_cast<const uint8_t*>(data) + loc.offset);

            switch (loc.scalarType) {
            case PushConstantMemberType::Float:
                switch (loc.elementCount) {
                case 1:
                    glUniform1fv(loc.location, 1, fptr);
                    break;
                case 2:
                    glUniform2fv(loc.location, 1, fptr);
                    break;
                case 3:
                    glUniform3fv(loc.location, 1, fptr);
                    break;
                case 4:
                    glUniform4fv(loc.location, 1, fptr);
                    break;
                }
                break;
            case PushConstantMemberType::Int:
            case PushConstantMemberType::UInt:
                switch (loc.elementCount) {
                case 1:
                    glUniform1iv(loc.location, 1, iptr);
                    break;
                case 2:
                    glUniform2iv(loc.location, 1, iptr);
                    break;
                case 3:
                    glUniform3iv(loc.location, 1, iptr);
                    break;
                case 4:
                    glUniform4iv(loc.location, 1, iptr);
                    break;
                }
                break;
            }
        }
    }

    void OpenGLRenderAPI::SetFrameData(const FrameData& data)
    {
        FrameData uboData;
        uboData.lightCamera = data.lightCamera;
        uboData.camera = data.camera;
        uboData.skyboxCamera = data.skyboxCamera;
        uboData.lightDir = data.lightDir;
        uboData.pad0 = 0.0f;
        uboData.cameraPos = data.cameraPos;
        uboData.pad1 = 0.0f;

        glBindBuffer(GL_UNIFORM_BUFFER, m_FrameUBO);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(FrameData), &uboData);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    void OpenGLRenderAPI::SetDrawData(const DrawData& data)
    {
        DrawDataUBO uboData{};
        uboData.model = data.model;
        uboData.normalMatrix = Math::Mat4(data.normalMatrix);

        glBindBuffer(GL_UNIFORM_BUFFER, m_ModelUBO);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(DrawDataUBO), &uboData);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    void OpenGLRenderAPI::DefaultFrameBuffer() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, m_ViewportWidth, m_ViewportHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    void OpenGLRenderAPI::SetViewport(uint width, uint height)
    {
        m_ViewportWidth = width;
        m_ViewportHeight = height;
        glViewport(m_ViewportPosX, m_ViewportPosY, width, height);
    }

    void OpenGLRenderAPI::SetViewport(uint width, uint height, uint posX, uint posY)
    {
        m_ViewportPosX = posX;
        m_ViewportPosY = posY;
        SetViewport(width, height);
    }

    Ref<Pipeline> OpenGLRenderAPI::CreatePipeline(const PipelineDesc& desc, AssetManager& assets)
    {
        uint program = OpenGLShaderCompiler::Compile(desc.shaderID, assets);
        auto pipeline = std::make_shared<Pipeline>(desc, program);

        const ShaderAsset& asset = assets.GetShaderAsset(desc.shaderID);
        if (asset.pushConstant.hasPushConstant) {
            for (const auto& member : asset.pushConstant.members) {
                std::string uniformName = asset.pushConstant.instanceName + "." + member.name;
                Platform::PushConstantUniformLoc loc{};
                loc.location = glGetUniformLocation(program, uniformName.c_str());
                loc.offset = member.offset;
                loc.elementCount = member.elementCount;
                loc.scalarType = member.scalarType;
                pipeline->m_PushConstantLocs.push_back(loc);
            }
        }

        return pipeline;
    }

    Ref<CubeMap> OpenGLRenderAPI::CreateCubeMapFromEquirectangular(Ref<Texture> equirect, uint faceSize,
                                                                    AssetManager& assets)
    {
        // Destination cube texture (RGB16F for HDR)
        uint cubeTexID;
        glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &cubeTexID);
        const int mipLevels = 1 + (int)std::floor(std::log2((double)faceSize));
        glTextureStorage2D(cubeTexID, mipLevels, GL_RGB16F, (GLsizei)faceSize, (GLsizei)faceSize);

        // Temporary depth renderbuffer and FBO for the capture pass
        uint rbo, fbo;
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, (GLsizei)faceSize, (GLsizei)faceSize);
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

        // Pipeline for the equirectangular transform pass
        ShaderID shaderID =
            assets.LoadShaderFromPath("res/shader/builtin/Passes/EquirectangularTransform.slang");
        PipelineDesc desc;
        desc.shaderID = shaderID;
        desc.culling = CullMode::None;
        Ref<Pipeline> pipeline = assets.GetPipeline(assets.CreatePipeline(desc, *this));

        // Unit cube geometry: same layout as the Skybox (positions only, 36 vertices)
        static const float s_CubeVertices[] = {
            -1.0f,  1.0f, -1.0f,   -1.0f, -1.0f, -1.0f,    1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,    1.0f,  1.0f, -1.0f,   -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,   -1.0f, -1.0f, -1.0f,   -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,   -1.0f,  1.0f,  1.0f,   -1.0f, -1.0f,  1.0f,

             1.0f, -1.0f, -1.0f,    1.0f, -1.0f,  1.0f,    1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,    1.0f,  1.0f, -1.0f,    1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,   -1.0f,  1.0f,  1.0f,    1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,    1.0f, -1.0f,  1.0f,   -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,    1.0f,  1.0f, -1.0f,    1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,   -1.0f,  1.0f,  1.0f,   -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,   -1.0f, -1.0f,  1.0f,    1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,   -1.0f, -1.0f,  1.0f,    1.0f, -1.0f,  1.0f,
        };
        Ref<VertexBuffer> vbo = CreateVertexBuffer(s_CubeVertices, sizeof(s_CubeVertices),
                                                   BufferProperties({{"POSITION", 3}}));

        Ref<TextureSampler> sampler = CreateSampler(
            SamplerProperties(SamplerFilter::MIN_MAG_LINEAR, SamplerWrapMode::WRAP_CLAMP_TO_EDGE,
                              SamplerWrapMode::WRAP_CLAMP_TO_EDGE));

        // 90 degree FOV, square aspect ratio to capture exactly one face
        Math::Mat4 proj = Math::Mat4::Perspective(90.0f, 1.0f, 0.1f, 10.0f);
        const Math::Vec3 origin(0.0f, 0.0f, 0.0f);
        Math::Mat4 captureViews[6] = {
            Math::Mat4::LookAt(origin, Math::Vec3( 1.0f,  0.0f,  0.0f), Math::Vec3(0.0f, -1.0f,  0.0f)),
            Math::Mat4::LookAt(origin, Math::Vec3(-1.0f,  0.0f,  0.0f), Math::Vec3(0.0f, -1.0f,  0.0f)),
            Math::Mat4::LookAt(origin, Math::Vec3( 0.0f,  1.0f,  0.0f), Math::Vec3(0.0f,  0.0f,  1.0f)),
            Math::Mat4::LookAt(origin, Math::Vec3( 0.0f, -1.0f,  0.0f), Math::Vec3(0.0f,  0.0f, -1.0f)),
            Math::Mat4::LookAt(origin, Math::Vec3( 0.0f,  0.0f,  1.0f), Math::Vec3(0.0f, -1.0f,  0.0f)),
            Math::Mat4::LookAt(origin, Math::Vec3( 0.0f,  0.0f, -1.0f), Math::Vec3(0.0f, -1.0f,  0.0f)),
        };

        // Render each face of the cube into the destination cube texture
        BindPipeline(pipeline);
        BindTexture(0, equirect);
        BindTextureSampler(0, sampler);
        BindVertexBuffer(vbo);
        SetDrawData({.model = Math::Mat4(1.0f), .normalMatrix = Math::Mat3(1.0f)});

        for (int i = 0; i < 6; i++) {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubeTexID, 0);
            glViewport(0, 0, (GLsizei)faceSize, (GLsizei)faceSize);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            Dodo::FrameData fd{};
            fd.camera = proj * captureViews[i];
            SetFrameData(fd);

            DrawArray(36);
        }

        glGenerateTextureMipmap(cubeTexID);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &fbo);
        glDeleteRenderbuffers(1, &rbo);

        SetViewport(m_ViewportWidth, m_ViewportHeight, m_ViewportPosX, m_ViewportPosY);

        return std::make_shared<CubeMap>(cubeTexID);
    }

    void OpenGLRenderAPI::ImGuiNewFrame() const
    {
        ImGui_ImplOpenGL3_NewFrame();
    }

    void OpenGLRenderAPI::ImGuiEndFrame() const
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
} // namespace Dodo::Platform
