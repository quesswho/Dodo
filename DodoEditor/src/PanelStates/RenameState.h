#pragma once

#include "Scene/EditorWorld.h"
#include <Dodo.h>

using namespace Dodo;

struct RenameState {
    EntityID entityId = -1;
    std::string nameBuffer = "";

    bool isActive() const { return entityId != -1; }

    void Begin(EditorWorld& world, EntityID id)
    {
        entityId = id;
        if (!world.HasComponent<NameComponent>(id)) {
            world.AddComponent<NameComponent>(id, NameComponent{"Unnamed"});
        }

        nameBuffer = world.GetComponent<NameComponent>(id).name;
    }

    void Update(EditorWorld& world)
    {
        if (!isActive()) return;

        if (!world.HasComponent<NameComponent>(entityId)) {
            world.AddComponent<NameComponent>(entityId, NameComponent{"Unnamed"});
        }

        if (!nameBuffer.empty()) {
            world.GetComponent<NameComponent>(entityId).name = nameBuffer;
        }
    }

    void Finish(EditorWorld& world)
    {
        Update(world);
        Cancel();
    }

    void Cancel()
    {
        entityId = -1;
        nameBuffer.clear();
    }
};