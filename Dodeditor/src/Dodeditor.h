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
	VertexBuffer* m_VBuffer;
	IndexBuffer* m_IBuffer;
	Shader* m_Shader;
	Shader* m_Shader2;
	FrameBuffer* m_FrameBuffer;

	Math::Mat4 m_Rotation;
	Math::Mat4 m_Projection;
	Scene* m_Scene;

	Model* m_Model;
};

class Dodeditor : public Application
{
private:
	WindowProperties PreInit();
public:
	Dodeditor();

	void Init();
};