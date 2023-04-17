#pragma once

#include <Dodo.h>
#include "World/WorldRenderer.h"

using namespace Dodo;

class GameLayer : public Layer {
private:

public:
	GameLayer();
	~GameLayer();

	void Update(float elapsed);
	void Render();
	void OnEvent(const Event& event);
private:

	World* m_World;
	Ref<WorldRenderer> m_WorldRenderer;
	ResourceManager* m_ResourceManager;


	Math::FreeCamera* m_Camera;

	Renderer3D* m_Renderer;
	Scene* m_Scene;


	PostEffect* m_PostEffect;

	Math::Mat4 m_LightProjection;
	Math::Mat4 m_LightView;

	Math::Vec3 m_LightLook;

	float m_Gamma;
};