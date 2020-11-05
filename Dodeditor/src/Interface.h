#pragma once

#include <Dodo.h>

using namespace Dodo;

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

class Interface
{
public:
	uint m_ViewportWidth = 0;
	uint m_ViewportHeight = 0;

	EditorProperties m_EditorProperties;
	
	Scene* m_Scene;
public:
	Interface(Scene* scene);

	bool BeginDraw();
	bool BeginViewport();
	bool ViewportResize();
	void EndViewport(FrameBuffer* framebuffer);
	void EndDraw();

	void ChangeScene(Scene* scene);

	void SetChangeSceneCallback(void (*scene)(Scene*));
private:
	void InitInterface();

	void ResetDockspace(uint dockspace_id);

	uint m_PosX = 0;
	uint m_PosY = 0;

	void DrawHierarchy();
	void DrawInspector();

	std::vector<Component> m_HierarchyComponents;
	std::vector<Component> m_InspectorComponents;

	std::unordered_map<uint, bool> m_SelectedEntity;

	char* m_CurrentInspectorName;
	bool m_InspectorSelectNew;

	bool m_ChangeScene;

	AsciiSceneFile m_File;

	void (*m_ChangeSceneFunc)(Scene*);
};