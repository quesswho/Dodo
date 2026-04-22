#include "SandBox.h"

using namespace Dodo;
using namespace Math;

GameLayer::GameLayer(Application& app)
{
    RenderAPI& renderAPI = *app.m_RenderAPI;
    AssetManager& assets = *app.m_AssetManager;

    DD_INFO("Working directory: {}", std::filesystem::current_path().string());

    renderAPI.ClearColor(0.2f, 0.2f, 0.9f);

    // FPS camera containing view matrix
    m_Camera = new FreeCameraController(
        Vec3(0.0f, 0.0f, 15.0f), -90.0f, 0.0f,
        (float)Application::s_Application->m_Window->GetWindowProperties().m_FrameBufferWidth /
            (float)Application::s_Application->m_Window->GetWindowProperties().m_FrameBufferHeight,
        0.04f, 10.0f);

    // Framebuffer initialization data
    FrameBufferProperties frameprop;
    frameprop.m_Width = Application::s_Application->m_Window->GetWindowProperties().m_FrameBufferWidth;
    frameprop.m_Height = Application::s_Application->m_Window->GetWindowProperties().m_FrameBufferHeight;
    frameprop.m_FrameBufferType = FrameBufferType::FRAMEBUFFER_COLOR_DEPTH_STENCIL;

    m_PostEffect = new PostEffect(frameprop, "res/shader/builtin/Passes/Gamma.slang", renderAPI, assets);
    m_PostEffectData.gamma = 1.0f;
    m_PostEffect->SetEffectData(m_PostEffectData);

    m_LightLook = Vec3(0.0, 0.0f, 15.0f);
    m_LightProjection = Mat4::Orthographic(-50.0f, 50.0f, -50.0f, 50.0f, 1.0f, 100.0f);
    m_LightView = Mat4::LookAt(Vec3(0.0f, 35.0f, 23.0f), m_LightLook, Vec3(0.0, 1.0, 0.0));

    m_Renderer = new Renderer3D(renderAPI, assets);
    m_Renderer->SetPostEffect(m_PostEffect);

    m_Scene = m_File.Read("res/sponza/sponza.das");
    // m_Scene = m_File.Read("res/knife.das");
    DD_INFO("Finished loading scene");

    std::vector<std::string> skyboxPath = {
        "res/texture/skybox/right.jpg",  "res/texture/skybox/left.jpg",  "res/texture/skybox/top.jpg",
        "res/texture/skybox/bottom.jpg", "res/texture/skybox/front.jpg", "res/texture/skybox/back.jpg",
    };

    m_Scene->m_SkyBox = new Skybox(skyboxPath, assets, renderAPI);
    DD_INFO("Finished loading skybox");
    m_Scene->m_LightSystem.m_Directional.m_Direction = Normalize(Vec3(0.2f, -0.5f, -0.5f));
    m_Scene->m_LightSystem.m_Directional.m_LightCamera = m_LightProjection * m_LightView;
    Application::s_Application->m_Window->SetCursorVisible(false);
    m_Camera->ResetMouse();
    DD_INFO("Finished game layer init");
}
GameLayer::~GameLayer()
{
    delete m_Renderer;
    delete m_PostEffect;
    delete m_Camera;
    delete m_Scene;
}

void GameLayer::Update(float elapsed)
{
    double gammaChangeSpeed = 0.0f;
    if (Application::s_Application->GetInput().IsKeyPressed(DODO_KEY_9)) gammaChangeSpeed += 1.0f * elapsed;
    if (Application::s_Application->GetInput().IsKeyPressed(DODO_KEY_8)) gammaChangeSpeed -= 1.0f * elapsed;

    // Change directional light
    if (Application::s_Application->GetInput().IsKeyPressed(DODO_KEY_1))
        m_Scene->m_LightSystem.m_Directional.m_Direction += elapsed * Vec3(1.0f, 0, 0);
    if (Application::s_Application->GetInput().IsKeyPressed(DODO_KEY_2))
        m_Scene->m_LightSystem.m_Directional.m_Direction -= elapsed * Vec3(1.0f, 0, 0);
    if (Application::s_Application->GetInput().IsKeyPressed(DODO_KEY_3))
        m_Scene->m_LightSystem.m_Directional.m_Direction += elapsed * Vec3(0, 1.0f, 0);
    if (Application::s_Application->GetInput().IsKeyPressed(DODO_KEY_4))
        m_Scene->m_LightSystem.m_Directional.m_Direction -= elapsed * Vec3(0, 1.0f, 0);
    if (Application::s_Application->GetInput().IsKeyPressed(DODO_KEY_5))
        m_Scene->m_LightSystem.m_Directional.m_Direction += elapsed * Vec3(0, 0, 1.0f);
    if (Application::s_Application->GetInput().IsKeyPressed(DODO_KEY_6))
        m_Scene->m_LightSystem.m_Directional.m_Direction -= elapsed * Vec3(0, 0, 1.0f);
    if (Application::s_Application->GetInput().IsKeyPressed(DODO_KEY_0))
        m_Scene->m_LightSystem.m_Directional.m_Direction = Vec3(0.0f, -1.0f, 1.0f);
    m_Scene->m_LightSystem.m_Directional.m_Direction = Normalize(m_Scene->m_LightSystem.m_Directional.m_Direction);

    if (Application::s_Application->GetInput().IsKeyPressed(DODO_KEY_0))
        m_Scene->m_LightSystem.m_Directional.m_Direction = Vec3(0.0f, -1.0f, 1.0f);

    if (Application::s_Application->GetInput().IsKeyPressed(DODO_KEY_UP))
        m_LightLook += elapsed * Vec3(1.0f, 0.0f, 0.0f) * 10.0f;
    if (Application::s_Application->GetInput().IsKeyPressed(DODO_KEY_DOWN))
        m_LightLook -= elapsed * Vec3(1.0f, 0.0f, 0.0f) * 10.0f;
    if (Application::s_Application->GetInput().IsKeyPressed(DODO_KEY_RIGHT))
        m_LightLook += elapsed * Vec3(0.0f, 0.0f, 1.0f) * 10.0f;
    if (Application::s_Application->GetInput().IsKeyPressed(DODO_KEY_LEFT))
        m_LightLook -= elapsed * Vec3(0.0f, 0.0f, 1.0f) * 10.0f;

    Vec3 lightPos = Vec3(-8.0f, 35.0f, 23.0f);
    m_LightView = Mat4::LookAt(lightPos, m_LightLook, Vec3(0.0, 1.0, 0.0));

    m_Scene->m_LightSystem.m_Directional.m_Direction = Normalize(m_LightLook - lightPos);
    m_Scene->m_LightSystem.m_Directional.m_LightCamera = m_LightProjection * m_LightView;

    if (gammaChangeSpeed != 0.0f) {
        m_PostEffectData.gamma += gammaChangeSpeed;
        m_PostEffectData.gamma = (std::max)(0.1f, (std::min)(m_PostEffectData.gamma, 5.0f));
        m_PostEffect->SetEffectData(m_PostEffectData);
    }

    m_Camera->Update(Application::s_Application->GetInput(), elapsed);

    std::stringstream stream;
    stream << Application::s_Application->m_FramesPerSecond << " FPS, " << std::fixed << std::setprecision(2)
           << Application::s_Application->m_FrameTimeMs << "ms";
    std::string s = stream.str();
    Application::s_Application->m_Window->SetTitle(stream.str().c_str());
}

void GameLayer::Render(RenderAPI& renderAPI, AssetManager& assets)
{
    m_Renderer->DrawShadowedScene(m_Scene, m_Camera->GetCamera(), renderAPI, assets);
}

void GameLayer::OnEvent(const Event& event)
{
    switch (event.GetType()) {
    case EventType::KEY_PRESSED:
        if (static_cast<const KeyPressEvent&>(event).m_Key == DODO_KEY_F11) {
            Application::s_Application->m_Window->FullScreen();
        }

        if (static_cast<const KeyPressEvent&>(event).m_Key == DODO_KEY_ESCAPE) {
            Application::s_Application->Shutdown();
        }
        break;
    case EventType::MOUSE_PRESSED:
        break;
    case EventType::MOUSE_POSITION:
        m_Camera->UpdateRotation(Application::s_Application->GetInput());
        break;
    case EventType::WINDOW_RESIZE:
        m_Camera->Resize(Application::s_Application->m_Window->GetWindowProperties().m_FrameBufferWidth,
                         Application::s_Application->m_Window->GetWindowProperties().m_FrameBufferHeight);
        m_PostEffect->Resize(Application::s_Application->m_Window->GetWindowProperties().m_FrameBufferWidth,
                             Application::s_Application->m_Window->GetWindowProperties().m_FrameBufferHeight);
        break;
    }
}

class Sandbox : public Application {
  public:
    Sandbox() : Application(PreInit()) {}

    ApplicationConfig PreInit()
    {
        WindowProperties props;
        props.m_Title = "SandBox";
        props.m_Width = 1080;
        props.m_Height = 720;
        props.m_Settings.backfaceCull = true;

        ApplicationConfig conf;
        conf.m_WindowProperties = props;

        return conf;
    }

    void Init() { PushLayer(new GameLayer(*this)); }
};

int main()
{
    Sandbox* sandBox = new Sandbox();
    sandBox->Run();
    delete sandBox;
}
