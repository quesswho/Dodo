#pragma once

#include "Event.h"

namespace Dodo {

    class Layer {
      public:
        Layer() {}
        virtual ~Layer() {}

        virtual void Update(float elapsed) = 0;
        virtual void Render() = 0;
        virtual void OnEvent(const Event &event) = 0;
    };

} // namespace Dodo