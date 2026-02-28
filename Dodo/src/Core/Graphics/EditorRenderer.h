#pragma once

#include <Core/Common.h>

#include <Core/Graphics/Skybox.h>
#include <Core/Graphics/Scene/Scene.h>
#include <Core/ECS/Component/ModelComponent.h>
#include <Core/Data/AssetManager.h>

#include <Core/Math/Camera/FreeCamera.h>

namespace Dodo {

	class EditorRenderer {
		Math::FreeCamera* m_Camera;
	public:
		EditorRenderer(Math::FreeCamera* camera)
			: m_Camera(camera)
		{}

		~EditorRenderer() {}

		void DrawScene(Scene* scene);

		void RenderEntities(World& world, Math::FreeCamera* camera, LightSystem& lightSystem, AssetManager& assets);

		void UpdateCamera(Math::FreeCamera* camera) {
			m_Camera = camera;
		}
	};
}