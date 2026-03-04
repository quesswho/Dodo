#pragma once

#include "Components/NameComponent.h"
#include "Components/SelectionTag.h"

#include <Core/ECS/ComponentPool.h>
#include <Core/ECS/RuntimeComponentList.h>
#include <Core/ECS/World.h>
#include <Core/Utilities/TypeList.h>

#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_set>
#include <utility>

using Dodo::AlwaysFalse;
using Dodo::EntityID;
using Dodo::TypeList;

using RuntimeWorld = ::Dodo::World;

template <typename T>
using ComponentPool = Dodo::ComponentPool<T>;

template <typename... ComponentTypes>
class EditorOverlayWorld {
  public:
    template <typename T>
    void AddComponent(EntityID entity, T&& component)
    {
        static_assert((std::is_same_v<std::remove_reference_t<T>, ComponentTypes> || ...));
        GetPool<std::remove_reference_t<T>>().AddComponent(entity, std::forward<T>(component));
    }

    template <typename T>
    bool HasComponent(EntityID entity) const
    {
        static_assert((std::is_same_v<T, ComponentTypes> || ...));
        return GetPool<T>().Exists(entity);
    }

    template <typename T>
    T& GetComponent(EntityID entity)
    {
        static_assert((std::is_same_v<T, ComponentTypes> || ...));
        return GetPool<T>().Get(entity);
    }

    template <typename T>
    ComponentPool<T>& GetPool()
    {
        static_assert((std::is_same_v<T, ComponentTypes> || ...));
        return std::get<ComponentPool<T>>(m_Pools);
    }

    template <typename T>
    const ComponentPool<T>& GetPool() const
    {
        static_assert((std::is_same_v<T, ComponentTypes> || ...));
        return std::get<ComponentPool<T>>(m_Pools);
    }

    bool HasAnyComponent(EntityID entity) const
    {
        return (std::get<ComponentPool<ComponentTypes>>(m_Pools).Exists(entity) || ...);
    }

    void RemoveAll(EntityID entity) { (std::get<ComponentPool<ComponentTypes>>(m_Pools).RemoveComponent(entity), ...); }

  private:
    std::tuple<ComponentPool<ComponentTypes>...> m_Pools;
};

// Defines ECS strucuture of the editor overlay
using OverlayComponentList = TypeList<NameComponent, SelectionTag>;

using Overlay = OverlayComponentList::Apply<EditorOverlayWorld>;

// Wrapper around core World and for the editor
class EditorWorld {
  public:
    using RuntimeComponentList = ::Dodo::RuntimeComponentList;

    EditorWorld(RuntimeWorld& runtimeWorld, Overlay& overlay) : m_RuntimeWorld(&runtimeWorld), m_Overlay(&overlay) {}

    EntityID CreateEntity()
    {
        EntityID id = m_RuntimeWorld->CreateEntity();
        if (!m_Overlay->HasComponent<NameComponent>(id)) {
            m_Overlay->AddComponent<NameComponent>(id, NameComponent{"Entity_" + std::to_string(id)});
        }
        return id;
    }

    void DeleteEntity(EntityID entity)
    {
        m_RuntimeWorld->DeleteEntity(entity);
        m_Overlay->RemoveAll(entity);
    }

    const std::unordered_set<EntityID>& GetAliveEntities() const { return m_RuntimeWorld->GetAliveEntities(); }

    template <typename T>
    void AddComponent(EntityID entity, T&& component)
    {
        using U = std::remove_reference_t<T>; // Remove reference for type checks
        if constexpr (RuntimeComponentList::template includes<U>()) {
            m_RuntimeWorld->AddComponent<U>(entity, std::forward<T>(component));
        } else if constexpr (OverlayComponentList::template includes<U>()) {
            m_Overlay->AddComponent<U>(entity, std::forward<T>(component));
        } else {
            static_assert(AlwaysFalse<U>::value, "Component type is not recognized!");
        }
    }

    template <typename T>
    bool HasComponent(EntityID entity) const
    {
        if constexpr (RuntimeComponentList::template includes<T>()) {
            return m_RuntimeWorld->HasComponent<T>(entity);
        } else if constexpr (OverlayComponentList::template includes<T>()) {
            return m_Overlay->HasComponent<T>(entity);
        } else {
            static_assert(AlwaysFalse<T>::value, "Component type is not recognized!");
        }
    }

    template <typename T>
    T& GetComponent(EntityID entity)
    {
        if constexpr (RuntimeComponentList::template includes<T>()) {
            return m_RuntimeWorld->GetComponent<T>(entity);
        } else if constexpr (OverlayComponentList::template includes<T>()) {
            return m_Overlay->GetComponent<T>(entity);
        } else {
            static_assert(AlwaysFalse<T>::value, "Component type is not recognized!");
        }
    }

    template <typename T>
    ComponentPool<T>& GetPool()
    {
        if constexpr (RuntimeComponentList::template includes<T>()) {
            return m_RuntimeWorld->GetPool<T>();
        } else if constexpr (OverlayComponentList::template includes<T>()) {
            return m_Overlay->GetPool<T>();
        } else {
            static_assert(AlwaysFalse<T>::value, "Component type is not recognized!");
        }
    }

    template <typename T>
    const ComponentPool<T>& GetPool() const
    {
        if constexpr (RuntimeComponentList::template includes<T>()) {
            return m_RuntimeWorld->GetPool<T>();
        } else if constexpr (OverlayComponentList::template includes<T>()) {
            return m_Overlay->GetPool<T>();
        } else {
            static_assert(AlwaysFalse<T>::value, "Component type is not recognized!");
        }
    }

    bool HasAnyComponent(EntityID entity) const
    {
        return m_RuntimeWorld->HasAnyComponent(entity) || m_Overlay->HasAnyComponent(entity);
    }

  private:
    RuntimeWorld* m_RuntimeWorld;
    Overlay* m_Overlay;
};