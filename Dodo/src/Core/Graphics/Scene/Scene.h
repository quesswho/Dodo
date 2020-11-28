#pragma once

#include "Core/Common.h"
#include <unordered_map>
#include "Entity.h"

#include "Component/ModelComponent.h"
#include "Component/Rectangle2DComponent.h"

#include "Core/Graphics/Skybox.h"
#include "Core/Math/Camera/FreeCamera.h"

namespace Dodo {

	class Scene {
	public:
		std::string m_Name;

		std::unordered_map<uint, Entity> m_Entities;

		// Could this be replaced with std:vector<std::variant<...>> inside Entity class?
		// Draw function would have to be redone; every component wont be a renderable component. Perhaps a constant variable inside the component class which implies what kind of component it is.

		// Entity Components
		std::unordered_map<uint, ModelComponent*> m_ModelComponent;
		std::unordered_map<uint, Rectangle2DComponent*> m_Rectangle2DComponent;

		Skybox* m_SkyBox;

	public:
		Scene(Math::FreeCamera* camera, std::string name = "Unnamed");
		~Scene();

		void Draw();

		void CreateEntity(uint id, const std::string& name);
		uint CreateEntity(const std::string& name = "Unnamed");
		bool RenameEntity(uint id, const std::string& name); // true ? Success : No entity with specified id
		bool DeleteEntity(uint id);

		void AddComponent(uint id, ModelComponent* comp);
		void AddComponent(uint id, Rectangle2DComponent* comp);

		inline void UpdateCamera(Math::FreeCamera* camera) { m_Camera = camera; }

		const uint m_AmountOfComponents;
	private:
		Math::FreeCamera* m_Camera;
	};
}