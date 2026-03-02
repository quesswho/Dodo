#pragma once

#include "Core/Utilities/TypeList.h"

#include "Core/ECS/Component/ModelComponent.h"

namespace Dodo {

    // Update this list when adding new components
    using RuntimeComponentList = TypeList<ModelComponent>;

} // namespace Dodo
