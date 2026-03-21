#pragma once

#include <Core/Common.h>

#include "Core/Graphics/Pipeline/PipelineDesc.h"

#include <unordered_map>

namespace Dodo::Platform {

    class OpenGLPipeline {
        friend class OpenGLRenderAPI;

      private:
        uint m_ShaderID;
        PipelineDesc m_Desc;

      public:
        OpenGLPipeline(const PipelineDesc& desc, uint shaderID) : m_ShaderID(shaderID), m_Desc(desc) {}
        ~OpenGLPipeline();
    };
} // namespace Dodo::Platform