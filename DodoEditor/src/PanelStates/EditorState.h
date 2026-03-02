#pragma once

#include "PanelStates/RenameState.h"
#include "Scene/EditorScene.h"
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
    EditorScene* scene = nullptr;
    Selection selection;
    RenameState renameState;
};

struct ViewportState {
    uint width = 0, height = 0;
    uint x = 0, y = 0;
};