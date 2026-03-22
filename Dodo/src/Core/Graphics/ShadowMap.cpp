#include "ShadowMap.h"
#include "pch.h"

namespace Dodo {

    ShadowMap::ShadowMap()
        : m_FrameBuffer(
              std::make_shared<FrameBuffer>(FrameBufferProperties(4096, 4096, FrameBufferType::FRAMEBUFFER_DEPTH)))
    {}

    ShadowMap::~ShadowMap() {}

    void ShadowMap::Bind() const
    {
        m_FrameBuffer->Bind();
    }
} // namespace Dodo