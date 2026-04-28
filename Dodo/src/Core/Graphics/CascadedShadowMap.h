#pragma once

#include "Core/Graphics/FrameBuffer.h"
#include "Core/Graphics/RenderAPI.h"
#include "Core/Math/Matrix/Mat4.h"
namespace Dodo {

    class CascadedShadowMap {
      private:
        Ref<FrameBuffer> m_FrameBuffer;

      public:
        CascadedShadowMap(RenderAPI& renderAPI, uint32_t layers);
        ~CascadedShadowMap();

        void UpdateCamera(Math::Mat4 projection, Math::Mat4 view);
        void Bind(RenderAPI& renderAPI);

        Ref<FrameBuffer> GetFrameBuffer() const { return m_FrameBuffer; }

        uint32_t m_Layers;
    };
} // namespace Dodo