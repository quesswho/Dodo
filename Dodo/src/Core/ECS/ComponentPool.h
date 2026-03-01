#pragma once

#include "Entity.h"

#include <unordered_map>
#include <vector>

namespace Dodo {

    template <typename T> class ComponentPool {
      public:
        void AddComponent(EntityID id, T &&component)
        {
            m_Lookup[id] = m_Components.size();
            m_Entities.emplace_back(id);
            m_Components.emplace_back(std::move(component));
        }

        void RemoveComponent(EntityID id)
        {
            auto it = m_Lookup.find(id);
            if (it == m_Lookup.end()) return;

            // Instead of deleting in O(n), we swap the component to be deleted with the last component and pop back in
            // constant time.
            size_t index = it->second;
            size_t last = m_Components.size() - 1;

            m_Components[index] = std::move(m_Components[last]);
            m_Entities[index] = m_Entities[last];
            m_Lookup[m_Entities[index]] = index;

            m_Components.pop_back();
            m_Entities.pop_back();
            m_Lookup.erase(it);
        }

        bool Exists(EntityID id) const { return m_Lookup.find(id) != m_Lookup.end(); }

        T &Get(EntityID id) { return m_Components[m_Lookup.at(id)]; }

        const std::vector<T> &GetComponents() const { return m_Components; }
        const std::vector<EntityID> &GetEntities() const { return m_Entities; }

      private:
        std::vector<T> m_Components;
        std::vector<EntityID> m_Entities;              // List of all entities that have this component
        std::unordered_map<EntityID, size_t> m_Lookup; // Map entityid to index in m_Components
    };
} // namespace Dodo