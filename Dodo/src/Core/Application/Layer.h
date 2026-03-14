#pragma once

#include "Event.h"

#include "Core/Data/AssetManager.h"
#include "Core/Graphics/RenderAPI.h"

namespace Dodo {

    class Layer {
      public:
        Layer() {}
        virtual ~Layer() {}

        virtual void Update(float elapsed) = 0;
        virtual void Render(RenderAPI& renderAPI, AssetManager& assets) = 0;
        virtual void OnEvent(const Event& event) = 0;
    };

} // namespace Dodo