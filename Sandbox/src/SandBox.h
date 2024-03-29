#pragma once

#include <Dodo.h>

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

	Math::FreeCamera* m_Camera;

	Renderer3D* m_Renderer;
	Scene* m_Scene;

	AsciiSceneFile m_File;

	PostEffect* m_PostEffect;

	Math::Mat4 m_LightProjection;
	Math::Mat4 m_LightView;

	Math::Vec3 m_LightLook;

	float m_Gamma;
};