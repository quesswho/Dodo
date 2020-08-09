#pragma once
#include <Dodo.h>

using namespace Dodo;

struct EditorProperties
{
	bool m_ShowViewport;
	bool m_ShowHierarchy;
	bool m_ShowInspector;

	bool m_ViewportHover;
	bool m_ViewportInput;

	const char* m_HierarchyName;
	const char* m_ViewportName;
	const char* m_InspectorName;
};

struct Component {
	Component()
		: m_Name("None"), m_Selected(false)
	{}

	Component(const char* name)
		: m_Name(name), m_Selected(false)
	{}

	const char* m_Name;
	bool m_Selected;
};

class GameLayer : public Layer {
private:

public:
	GameLayer();
	~GameLayer();

	void InitEditor();

	void ResetDockspace(uint dockspace_id);
	void DrawImGui();
	void DrawScene();
	void Update(float elapsed);
	void Render();
	void OnEvent(const Event& event);

	void DrawViewport();
	void DrawHierarchy();
	void DrawInspector();

	void ChangeScene(Scene* scene);
private:
	FrameBuffer* m_FrameBuffer;

	Math::FreeCamera* m_Camera;

	Scene* m_Scene;

	EditorProperties m_EditorProperties;

	std::unordered_map<uint, bool> m_SelectedEntity;

	std::vector<Component> m_HierarchyComponents;
	std::vector<Component> m_InspectorComponents;

	char* m_CurrentInspectorName;
	bool m_InspectorSelectNew;

	AsciiSceneFile m_File;
};

class Dodeditor : public Application
{
private:
	WindowProperties PreInit();
public:
	Dodeditor();

	void Init();
};