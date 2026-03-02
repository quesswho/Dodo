#pragma once

#include <Dodo.h>

using namespace Dodo;

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