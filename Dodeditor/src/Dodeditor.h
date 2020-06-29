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
	VertexBuffer* m_VBuffer;
	IndexBuffer* m_IBuffer;
};