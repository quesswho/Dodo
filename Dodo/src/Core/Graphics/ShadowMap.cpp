#include "ShadowMap.h"
#include "pch.h"

namespace Dodo {

    ShadowMap::ShadowMap()
        : m_FrameBuffer(new FrameBuffer(FrameBufferProperties(4096, 4096, FrameBufferType::FRAMEBUFFER_DEPTH)))
    {}

    ShadowMap::~ShadowMap() { delete m_FrameBuffer; }

    void ShadowMap::BindTexture(uint index) const { m_FrameBuffer->BindTexture(index); }

    void ShadowMap::Bind() const { m_FrameBuffer->Bind(); }
} // namespace Dodo