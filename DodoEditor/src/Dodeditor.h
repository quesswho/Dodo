#pragma once

#include "Interface.h"

using namespace Dodo;

class GameLayer : public Layer {
  private:
  public:
    GameLayer();
    ~GameLayer();

    void DrawScene();
    void Update(float elapsed);
    void Render();
    void OnEvent(const Event &event);
    void SetScene(Scene *scene);

  private:
    FrameBuffer *m_FrameBuffer;

    Math::FreeCamera *m_Camera;

    EditorRenderer *m_Renderer;
    Scene *m_Scene;

    Interface *m_Interface;
};

class Dodeditor : public Application {
  private:
    ApplicationConfig PreInit();

  public:
    Dodeditor();

    void Init();
};