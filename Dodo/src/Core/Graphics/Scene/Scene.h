#pragma once

#include "Core/Common.h"
#include <unordered_map>
#include "Entity.h"
#include "Component/ModelComponent.h"

namespace Dodo {

	class Scene {
	public:
		std::unordered_map<uint, Entity> m_Entities;
		std::unordered_map<uint, ModelComponent*> m_ModelComponent;
	public:
		Scene(Math::FreeCamera* camera);
		~Scene();

		void Draw();

		void CreateEntity(uint id, const std::string& name);
		uint CreateEntity(const std::string& name = "Unnamed");
		bool RenameEntity(uint id, const std::string& name); // true ? Success : No entity with specified id
		bool DeleteEntity(uint id);

		void AddComponent(uint id, ModelComponent* comp);

		inline void UpdateCamera(Math::FreeCamera* camera) { m_Camera = camera; }

		const uint m_AmountOfComponents;
	private:
		Math::FreeCamera* m_Camera;
	};
}