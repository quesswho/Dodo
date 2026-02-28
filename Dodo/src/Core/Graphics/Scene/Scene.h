#pragma once

#include <Core/Common.h>

#include "Light.h"

#include <Core/ECS/World.h>

#include <Core/Graphics/Skybox.h>
#include <Core/Math/Camera/FreeCamera.h>

namespace Dodo {

	class Scene {
	public:
		std::string m_Name;

        // Contains all entities and their components.
        World* m_World;

		Skybox* m_SkyBox;
		LightSystem m_LightSystem;
	public:
		Scene(Math::FreeCamera* camera, std::string name = "Unnamed");
		~Scene();

        World& GetWorld();

		inline void UpdateCamera(Math::FreeCamera* camera) { m_Camera = camera; }
	private:
		Math::FreeCamera* m_Camera;
	};
}