#include "CascadedShadowMap.h"

namespace Dodo {

    CascadedShadowMap::CascadedShadowMap(RenderAPI& renderAPI, uint32_t layers)
        : m_Layers(layers)
    {}

    CascadedShadowMap::~CascadedShadowMap() {}

    void CascadedShadowMap::UpdateCamera(Math::Mat4 projection, Math::Mat4 view)
    {
        
    }

    void CascadedShadowMap::Bind(RenderAPI& renderAPI)
    {
        renderAPI.BindFrameBuffer(m_FrameBuffer);
    }
}