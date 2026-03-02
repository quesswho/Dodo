#pragma once

#include "RenameState.h"
#include <Dodo.h>

struct Selection {
    std::vector<EntityID> entities;

    bool Empty() const { return entities.empty(); }
    bool Single() const { return entities.size() == 1; }
    bool Contains(EntityID e) const { return std::find(entities.begin(), entities.end(), e) != entities.end(); }

    void Single(EntityID e)
    {
        entities.clear();
        entities.push_back(e);
    }

    void Toggle(EntityID e)
    {
        if (Contains(e))
        {
            // We want to keep the order of the entities in the selection, so we can't swap and delete the last element.
            entities.erase(std::remove(entities.begin(), entities.end(), e), entities.end());
        } else
        {
            entities.push_back(e);
        }
    }

    void Clear() { entities.clear(); }
};

struct EditorState {
    Scene* scene = nullptr;
    Selection selection;
    RenameState renameState;
};

struct ViewportState {
    uint width = 0, height = 0;
    uint x = 0, y = 0;
};

struct TransformEditState {
    Math::Vec3 translate = {0.0f, 0.0f, 0.0f};
    Math::Vec3 scale = {1.0f, 1.0f, 1.0f};
    Math::Vec3 rotate = {0.0f, 0.0f, 0.0f};
    bool syncScale = true;
};

struct InspectorState {
    bool visible = false;
    bool dirty = false;

    TransformEditState transformState;
};

struct HierarchyState {
    bool visible = false;
};