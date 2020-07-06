#pragma once
#include <Dodo.h>

using namespace Dodo;

class GameLayer : public Layer {
private:

public:
	GameLayer();
	~GameLayer();

	void DrawImGui();
	void DrawScene();
	void Update(float elapsed);
	void Render();
	void OnEvent(const Event& event);
private:
	VertexBuffer* m_VBuffer;
	IndexBuffer* m_IBuffer;
	ArrayBuffer* m_VAO;
	Shader* m_Shader;
	FrameBuffer* m_FrameBuffer;

	Math::Mat4 m_Rotation;
};