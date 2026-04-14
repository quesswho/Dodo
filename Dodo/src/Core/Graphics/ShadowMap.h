#pragma once

#include "Core/Graphics/FrameBuffer.h"
#include "Core/Graphics/RenderAPI.h"

namespace Dodo {

    // Essentially a wrapper of framebuffer. Implementing point lights will give this class a distinction
    class ShadowMap {
      private:
        Ref<FrameBuffer> m_FrameBuffer;

      public:
        ShadowMap(RenderAPI& renderAPI);
        ~ShadowMap();

        void Bind(RenderAPI& renderAPI);

        Ref<FrameBuffer> GetFrameBuffer() const { return m_FrameBuffer; }
    };
} // namespace Dodo