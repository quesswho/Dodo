#pragma once

#include <Core/Common.h>

#include <Core/Graphics/Skybox.h>
#include <Core/Graphics/ShadowMap.h>
#include <Core/Graphics/PostEffect.h>
#include <Core/Graphics/Scene/Scene.h>
#include <Core/ECS/Component/ModelComponent.h>
#include <Core/ECS/Component/Rectangle2DComponent.h>

#include <Core/Math/Camera/FreeCamera.h>

namespace Dodo {
	// Make this a deferred renderer
	class Renderer3D {
		Math::FreeCamera* m_Camera;
		PostEffect* m_PostEffect;

		ShadowMap* m_ShadowMap;
		Ref<Material> m_ShadowMapMaterial;
	public:
		Renderer3D(Math::FreeCamera* camera);

		~Renderer3D() {}

		void DrawScene(Scene* scene);
		void DrawShadowedScene(Scene* scene);

		void RenderEntities(World& world, Math::FreeCamera* camera, LightSystem& lightSystem);
		void RenderEntitiesWithMaterial(World& world, Ref<Material> material);

		void UpdateCamera(Math::FreeCamera* camera) {
			m_Camera = camera;
		}

		void SetPostEffect(PostEffect* fx) {
			m_PostEffect = fx;
		}
	};
}