#pragma once

#include <Core/ECS/Entity.h>
#include <Core/ECS/ComponentPool.h>
#include <Core/ECS/Component/NameComponent.h>
#include <Core/ECS/Component/ModelComponent.h>

#include <tuple>
#include <unordered_set>

namespace Dodo {
    template<typename... ComponentTypes>
    class BasicWorld {
    public:
        EntityID CreateEntity() {
            EntityID id = m_NextEntity++;
            m_AliveEntities.insert(id);
            return id;
        }

        void DeleteEntity(EntityID entity) {
            m_AliveEntities.erase(entity);
            (std::get<ComponentPool<ComponentTypes>>(m_Pools).RemoveComponent(entity), ...); // remove from all pools
        }

        template<typename T>
        void AddComponent(EntityID entity, T&& component) {
            static_assert((std::is_same_v<T, ComponentTypes> || ...));
            GetPool<T>().AddComponent(entity, std::forward<T>(component));
        }

        template<typename T>
        bool HasComponent(EntityID entity) const {
            static_assert((std::is_same_v<T, ComponentTypes> || ...));
            return GetPool<T>().Exists(entity);
        }

        template<typename T>
        T& GetComponent(EntityID entity) {
            static_assert((std::is_same_v<T, ComponentTypes> || ...));
            return GetPool<T>().Get(entity);
        }

        template<typename T>
        ComponentPool<T>& GetPool() {
            static_assert((std::is_same_v<T, ComponentTypes> || ...));
            return std::get<ComponentPool<T>>(m_Pools);
        }

        template<typename T>
        const ComponentPool<T>& GetPool() const {
            static_assert((std::is_same_v<T, ComponentTypes> || ...));
            return std::get<ComponentPool<T>>(m_Pools);
        }

        const std::unordered_set<EntityID>& GetAliveEntities() const {
            return m_AliveEntities;
        }

        bool HasAnyComponent(EntityID entity) const {
            return (std::get<ComponentPool<ComponentTypes>>(m_Pools).Exists(entity) || ...);
        }

    private:
        EntityID m_NextEntity = 1; // Used for generating new unique ids.
        std::tuple<ComponentPool<ComponentTypes>...> m_Pools;
        std::unordered_set<EntityID> m_AliveEntities;
    };

    using World = BasicWorld<NameComponent, ModelComponent>;
}