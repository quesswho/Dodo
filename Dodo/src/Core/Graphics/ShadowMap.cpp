#include "ShadowMap.h"

namespace Dodo {

    ShadowMap::ShadowMap(RenderAPI& renderAPI)
        : m_FrameBuffer(
              renderAPI.CreateFrameBuffer(FrameBufferProperties(4096, 4096, FrameBufferType::FRAMEBUFFER_DEPTH)))
    {}

    ShadowMap::~ShadowMap() {}

    void ShadowMap::Bind(RenderAPI& renderAPI)
    {
        renderAPI.BindFrameBuffer(m_FrameBuffer);
    }
} // namespace Dodo