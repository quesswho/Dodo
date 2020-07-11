#pragma once

#include "Core/Common.h"
#include <unordered_map>
#include "Entity.h"

namespace Dodo {

	class Scene {
	public:
		std::unordered_map<uint, Entity> m_Entities;
	public:
		Scene();
		~Scene();

		uint CreateEntity(const std::string& name = "Unnamed");
		bool RenameEntity(uint id, const std::string& name); // true ? Success : No entity with specified id
		bool DeleteEntity(uint id);
	};
}