#pragma once

#include "EditorRenderer.h"
#include "Interface.h"
#include "Scene/EditorScene.h"

using namespace Dodo;

class GameLayer : public Layer {
  private:
  public:
    GameLayer(Application& app);
    ~GameLayer();

    void DrawScene(RenderAPI& renderAPI, AssetManager& assets);
    void Update(float elapsed);
    void Render(RenderAPI& renderAPI, AssetManager& assets);
    void OnEvent(const Event& event);
    void SetScene(EditorScene* scene);

  private:
    /**
     * Loads the test scene the editor opens with, or an empty scene if it is missing.
     */
    EditorScene* LoadStartupScene();

    Ref<FrameBuffer> m_FrameBuffer;

    FreeCameraController* m_Camera;

    EditorRenderer* m_Renderer;
    EditorScene* m_Scene;

    Interface* m_Interface;
};

class Dodeditor : public Application {
  private:
    ApplicationConfig PreInit();

  public:
    Dodeditor();

    void Init();
};