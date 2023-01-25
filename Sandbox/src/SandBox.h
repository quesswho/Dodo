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
	FrameBuffer* m_FrameBuffer;

	Math::FreeCamera* m_Camera;

	Scene* m_Scene;

	AsciiSceneFile m_File;
};