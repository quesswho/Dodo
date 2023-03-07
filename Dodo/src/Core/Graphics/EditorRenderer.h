#pragma once

#include <Core/Common.h>

#include "Core/Graphics/Skybox.h"
#include "Core/Graphics/Scene/Scene.h"
#include "Core/Graphics/Scene/Component/ModelComponent.h"
#include "Core/Graphics/Scene/Component/Rectangle2DComponent.h"

#include "Core/Math/Camera/FreeCamera.h"

namespace Dodo {

	class EditorRenderer {
		Math::FreeCamera* m_Camera;
	public:
		EditorRenderer(Math::FreeCamera* camera)
			: m_Camera(camera)
		{}

		~EditorRenderer() {}

		void Begin();
		void Flush();

		void DrawModel(ModelComponent* model);
		void DrawRectangle(Rectangle2DComponent* rectangle);
		void DrawSkybox(Skybox* skybox);
		void DrawScene(Scene* scene);

		void UpdateCamera(Math::FreeCamera* camera) {
			m_Camera = camera;
		}
	};
}