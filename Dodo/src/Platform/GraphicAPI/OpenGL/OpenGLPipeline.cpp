#include "OpenGLPipeline.h"
#include "pch.h"

#include "Core/Application/Application.h"
#include "Core/System/FileUtils.h"

#include <glad/gl.h>

namespace Dodo::Platform {

    OpenGLPipeline::~OpenGLPipeline()
    {
        glDeleteProgram(m_ShaderID);
    }
} // namespace Dodo::Platform