#pragma once

#include <Core/Common.h>

#include <Core/Graphics/Skybox.h>
#include <Core/Graphics/Scene/Scene.h>
#include <Core/ECS/Component/ModelComponent.h>
#include <Core/ECS/Component/Rectangle2DComponent.h>

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

		void UpdateCamera(Math::FreeCamera* camera) {
			m_Camera = camera;
		}
	};
}