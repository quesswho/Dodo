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
		Scene();
		~Scene();

		void Draw();

		uint CreateEntity(const std::string& name = "Unnamed");
		bool RenameEntity(uint id, const std::string& name); // true ? Success : No entity with specified id
		bool DeleteEntity(uint id);

		void AddComponent(uint id, ModelComponent* comp);

		const uint m_AmountOfComponents;
	};
}