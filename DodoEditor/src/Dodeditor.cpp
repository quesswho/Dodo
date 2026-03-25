#include "Dodeditor.h"

using namespace Dodo;
using namespace Math;

GameLayer::GameLayer(Application& app)
{
    RenderAPI& renderAPI = *app.m_RenderAPI;
    AssetManager& assets = *app.m_AssetManager;

    renderAPI.ClearColor(0.2f, 0.2f, 0.9f);
    // renderAPI.DepthTest(true);
    // renderAPI.Blending(true);

    BufferProperties bufferprop = {{"POSITION", 3}, {"TEXCOORD", 2}, {"NORMAL", 3}, {"TANGENT", 3}};

    m_Camera = new FreeCameraController(Vec3(0.0f, 0.0f, 20.0f), -90.0f, 0.0f,
                                        (float)app.m_Window->GetWindowProperties().m_Width /
                                            (float)app.m_Window->GetWindowProperties().m_Height,
                                        0.04f, 10.0f);

    FrameBufferProperties frameprop;
    frameprop.m_Width = app.m_Window->GetWindowProperties().m_Width;
    frameprop.m_Height = app.m_Window->GetWindowProperties().m_Height;
    frameprop.m_SamplerProperties = SamplerProperties(SamplerFilter::MIN_MAG_LINEAR);

    m_FrameBuffer = std::make_shared<FrameBuffer>(frameprop);

    m_Renderer = new EditorRenderer();
    m_Scene = new EditorScene();

    std::vector<std::string> skyboxPath = {
        "res/texture/skybox/right.jpg",  "res/texture/skybox/left.jpg",  "res/texture/skybox/top.jpg",
        "res/texture/skybox/bottom.jpg", "res/texture/skybox/front.jpg", "res/texture/skybox/back.jpg",
    };

    m_Scene->m_SkyBox = new Skybox(skyboxPath, assets, renderAPI);

    m_Interface = new Interface(m_Scene);
}

void GameLayer::SetScene(EditorScene* scene)
{
    m_Scene = scene;
}

GameLayer::~GameLayer()
{
    delete m_Camera;
    delete m_Scene;
    delete m_Interface;
}

void GameLayer::Update(float elapsed)
{
    if (m_Interface->m_EditorProperties.m_ViewportInput)
        m_Camera->Update(Application::s_Application->GetInput(), elapsed);
}

void GameLayer::Render(RenderAPI& renderAPI, AssetManager& assets)
{
    if (m_Interface->BeginDraw()) {
        m_Scene = m_Interface->m_EditorState.scene; // Maybe work out something better when changing scene
    }

    m_Interface->BeginViewport();
    if (m_Interface->ViewportResize()) {
        m_Camera->Resize(m_Interface->m_ViewportState.width, m_Interface->m_ViewportState.height);
        m_FrameBuffer->Resize(m_Interface->m_ViewportState.width, m_Interface->m_ViewportState.height);
    }
    DrawScene(renderAPI, assets);
    m_Interface->EndViewport(renderAPI, m_FrameBuffer);
    m_Interface->EndDraw();
}

void GameLayer::DrawScene(RenderAPI& renderAPI, AssetManager& assets)
{
    m_FrameBuffer->Bind();

    m_Renderer->DrawScene(m_Scene, m_Camera->GetCamera(), renderAPI, assets);

    renderAPI.DefaultFrameBuffer();
}

void GameLayer::OnEvent(const Event& event)
{
    switch (event.GetType()) {
    case EventType::KEY_PRESSED:
        switch (static_cast<const KeyPressEvent&>(event).m_Key) {
        case DODO_KEY_ESCAPE:
            Application::s_Application->Shutdown();
            break;
        case DODO_KEY_Z:
            if (m_Interface->m_EditorProperties.m_ViewportHover && !m_Interface->m_EditorProperties.m_ViewportInput) {
                m_Interface->m_EditorProperties.m_ViewportInput = true;
                Application::s_Application->m_Window->SetCursorVisible(false);
                m_Camera->ResetMouse();
            } else if (m_Interface->m_EditorProperties.m_ViewportInput) {
                m_Interface->m_EditorProperties.m_ViewportInput = false;
                Application::s_Application->m_Window->SetCursorVisible(true);
            }
            break;
        case DODO_KEY_DELETE:
            if (!Application::s_Application->GetInput().IsKeyPressed(DODO_KEY_LEFT_CONTROL)) {
                break;
            }

            for (int entityId : m_Interface->m_EditorState.selection.entities) {
                m_Interface->m_EditorState.scene->GetWorld().DeleteEntity(entityId);
            }
            break;
        }
        break;
    case EventType::MOUSE_PRESSED:
        break;
    case EventType::MOUSE_POSITION:
        if (m_Interface->m_EditorProperties.m_ViewportInput)
            m_Camera->UpdateRotation(Application::s_Application->GetInput());
        break;
    }
}

// Entry //

Dodeditor::Dodeditor() : Application(PreInit()) {}

ApplicationConfig Dodeditor::PreInit()
{
    WindowProperties props;
    props.m_Title = "Dodeditor";
    props.m_Width = 1600;
    props.m_Height = 960;
    props.m_Settings.imgui = true;
    props.m_Settings.imguiDocking = true;

    ApplicationConfig conf;
    conf.m_WindowProperties = props;
    return conf;
}

void Dodeditor::Init()
{
    PushLayer(new GameLayer(*this));
}

int main()
{
    Dodeditor* sandBox = new Dodeditor();
    sandBox->Run();
    delete sandBox;
}
