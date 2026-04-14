#pragma once

#include <Core/Common.h>

#include "Core/Data/ShaderAsset.h"
#include "Core/Graphics/Pipeline/PipelineDesc.h"

#include <glad/gl.h>
#include <vector>

namespace Dodo::Platform {

    struct PushConstantUniformLoc {
        GLint location;
        uint32_t offset;
        uint32_t elementCount;
        PushConstantMemberType scalarType;
    };

    class OpenGLPipeline {
        friend class OpenGLRenderAPI;

      private:
        uint m_ShaderID;
        PipelineDesc m_Desc;
        std::vector<PushConstantUniformLoc> m_PushConstantLocs;

      public:
        OpenGLPipeline(const PipelineDesc& desc, uint shaderID) : m_ShaderID(shaderID), m_Desc(desc) {}
        ~OpenGLPipeline();

        const PipelineDesc& GetDesc() const { return m_Desc; }
    };
} // namespace Dodo::Platform