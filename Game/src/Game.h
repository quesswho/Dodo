#pragma once

#include "world/WorldManager.h"
#include <Dodo.h>

using namespace Dodo;

class GameLayer : public Layer {
  private:
  public:
    GameLayer(Application& app);
    ~GameLayer();

    void Update(float elapsed);
    void Render(RenderAPI& renderAPI, AssetManager& assets);
    void OnEvent(const Event& event);

  private:
    Ref<ResourceManager> m_ResourceManager;
    Ref<WorldManager> m_WorldManager;

    Math::FreeCamera* m_Camera;

    Renderer3D* m_Renderer;
    Scene* m_Scene;

    PostEffect* m_PostEffect;

    Math::Mat4 m_LightProjection;
    Math::Mat4 m_LightView;

    Math::Vec3 m_LightLook;

    float m_Gamma;
};