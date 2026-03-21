#include "OpenGLRenderAPI.h"
#include "pch.h"

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
        glDeleteBuffers(1, &m_FrameUBO);
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
        }

        // Create frame UBO
        glGenBuffers(1, &m_FrameUBO);
        glBindBuffer(GL_UNIFORM_BUFFER, m_FrameUBO);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(FrameData), nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_FrameUBO); // Bind to slot 0
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        // Create push constant UBO (binding 1, 128 bytes max)
        glGenBuffers(1, &m_PushConstantUBO);
        glBindBuffer(GL_UNIFORM_BUFFER, m_PushConstantUBO);
        glBufferData(GL_UNIFORM_BUFFER, 128, nullptr, GL_DYNAMIC_DRAW); // 128 byte Vulkan guaranteed min
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_PushConstantUBO);      // binding point 1
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        return RenderInitError(RenderInitStatus::Success, "");
    }

    void OpenGLRenderAPI::BindPipeline(Ref<Pipeline> pipeline)
    {
        m_CurrentPipelineID = pipeline->m_ShaderID;
        glUseProgram(pipeline->m_ShaderID);

        // Apply pipeline state from desc
        if (pipeline->m_Desc.depthTest) {
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_NEVER + (uint)pipeline->m_Desc.depthMode);
        } else {
            glDisable(GL_DEPTH_TEST);
        }

        if (pipeline->m_Desc.blending) {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_BLEND);
        } else {
            glDisable(GL_BLEND);
        }

        if (pipeline->m_Desc.culling) {
            glEnable(GL_CULL_FACE);
            glCullFace(pipeline->m_Desc.backfaceCull ? GL_BACK : GL_FRONT);
        } else {
            glDisable(GL_CULL_FACE);
        }

        if (pipeline->m_Desc.stencilTest) {
            glEnable(GL_STENCIL_TEST);
        } else {
            glDisable(GL_STENCIL_TEST);
        }
    }

    void OpenGLRenderAPI::PushConstants(const void* data, size_t size)
    {
        glBindBuffer(GL_UNIFORM_BUFFER, m_PushConstantUBO);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, size, data);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    void OpenGLRenderAPI::SetFrameData(const FrameData& data)
    {
        FrameData uboData;
        uboData.lightCamera = data.lightCamera;
        uboData.camera = data.camera;
        uboData.lightDir = data.lightDir;
        uboData.pad0 = 0.0f;
        uboData.cameraPos = data.cameraPos;
        uboData.pad1 = 0.0f;

        glBindBuffer(GL_UNIFORM_BUFFER, m_FrameUBO);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(FrameData), &uboData);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        /*GLint uLoc = glGetUniformLocation(m_CurrentPipelineID, "u_LightCamera");
        glUniformMatrix4fv(uLoc, 1, GL_FALSE, &data.lightCamera.m_Elements[0]);
        uLoc = glGetUniformLocation(m_CurrentPipelineID, "u_LightDir");
        glUniform3f(uLoc, data.lightDir.x, data.lightDir.y, data.lightDir.z);
        uLoc = glGetUniformLocation(m_CurrentPipelineID, "u_Camera");
        glUniformMatrix4fv(uLoc, 1, GL_FALSE, &data.camera.m_Elements[0]);
        uLoc = glGetUniformLocation(m_CurrentPipelineID, "u_CameraPos");
        glUniform3f(uLoc, data.cameraPos.x, data.cameraPos.y, data.cameraPos.z);*/
    }

    void OpenGLRenderAPI::SetDrawData(const DrawData& data)
    {
        GLint uLoc = glGetUniformLocation(m_CurrentPipelineID, "u_Model");
        glUniformMatrix4fv(uLoc, 1, GL_FALSE, &data.model.m_Elements[0]);
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

    Ref<Pipeline> OpenGLRenderAPI::CreatePipeline(const PipelineDesc& desc, const ShaderSource& source)
    {
        uint program = OpenGLShaderCompiler::Compile(source);

        return std::make_shared<Pipeline>(desc, program);
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