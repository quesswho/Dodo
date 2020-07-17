#pragma once
#include <Dodo.h>

using namespace Dodo;

class GameLayer : public Layer {
private:

public:
	GameLayer();
	~GameLayer();

	void ResetDockspace(uint dockspace_id);
	void DrawImGui();
	void DrawScene();
	void Update(float elapsed);
	void Render();
	void OnEvent(const Event& event);
private:
	Shader* m_Shader;
	FrameBuffer* m_FrameBuffer;

	Math::FreeCamera* m_Camera;

	Math::Mat4 m_Rotation;
	Scene* m_Scene;

	Model* m_Model;

	bool m_ViewportActive;
	bool m_ViewportHover;

	std::unordered_map<uint, bool> m_SelectedEntity;
};

class Dodeditor : public Application
{
private:
	WindowProperties PreInit();
public:
	Dodeditor();

	void Init();
};