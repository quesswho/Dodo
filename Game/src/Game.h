#pragma once

#include "world/WorldManager.h"
#include <Dodo.h>

using namespace Dodo;

struct PostEffectData {
    float gamma;
    float exposure;
    float padding[2]; // Padding to ensure 16 byte alignment (std140)
};

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

    FreeCameraController* m_Camera;

    Scene* m_Scene;

    PostEffect* m_PostEffect;

    Math::Mat4 m_LightProjection;
    Math::Mat4 m_LightView;

    Math::Vec3 m_LightLook;

    PostEffectData m_PostEffectData;
};