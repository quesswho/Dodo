#include "SlangCompiler.h"

namespace Dodo {
    SlangCompiler::SlangCompiler(Target target)
    {
        SlangGlobalSessionDesc globalDesc = {};
        globalDesc.enableGLSL = true;
        slang::createGlobalSession(&globalDesc, m_GlobalSession.writeRef());

        // Temporarily set target to GLSL
        slang::TargetDesc targetDesc = {};
        switch (target) {
        case Target::GLSL:
            targetDesc.format = SLANG_GLSL;
            targetDesc.profile = m_GlobalSession->findProfile("glsl_450");
            break;
        case Target::SPIRV:
            targetDesc.format = SLANG_SPIRV;
            targetDesc.profile = m_GlobalSession->findProfile("spirv_1_5");
        }

        slang::SessionDesc sessionDesc = {};
        sessionDesc.targets = &targetDesc;
        sessionDesc.targetCount = 1;

        m_GlobalSession->createSession(sessionDesc, m_Session.writeRef());
    }

    SlangCompiler::~SlangCompiler()
    {
        m_Session.setNull();
        m_GlobalSession.setNull();
    }

    ShaderSource SlangCompiler::CompileFile(const std::string& path)
    {
        Slang::ComPtr<slang::IBlob> diagnostics;

        slang::IModule* module = m_Session->loadModule(path.c_str(), diagnostics.writeRef());

        if (diagnostics) {
            DD_WARN("Slang: {}", (const char*)diagnostics->getBufferPointer());
        }
        if (!module) {
            DD_ERR("Failed to load slang module: {}", path);
            return {};
        }

        return CompileModule(module);
    }

    ShaderSource SlangCompiler::CompileFromString(const std::string& source, const std::string& name)
    {
        Slang::ComPtr<slang::IBlob> diagnostics;

        slang::IModule* module =
            m_Session->loadModuleFromSourceString(name.c_str(), name.c_str(), source.c_str(), diagnostics.writeRef());

        if (diagnostics) {
            DD_WARN("Slang: {}", (const char*)diagnostics->getBufferPointer());
        }
        if (!module) {
            DD_ERR("Failed to compile slang source: {}", name);
            return {};
        }

        return CompileModule(module);
    }

    ShaderSource SlangCompiler::CompileModule(slang::IModule* module)
    {
        ShaderSource result;

        int entryPointCount = module->getDefinedEntryPointCount();
        for (int i = 0; i < entryPointCount; i++) {
            Slang::ComPtr<slang::IEntryPoint> entryPoint;
            module->getDefinedEntryPoint(i, entryPoint.writeRef());

            // Get entry point reflection to determine stage
            slang::EntryPointReflection* reflection = entryPoint->getLayout()->getEntryPointByIndex(0);
            SlangStage slangStage = reflection->getStage();

            ShaderStage stage;
            switch (slangStage) {
            case SLANG_STAGE_VERTEX:
                stage = ShaderStage::Vertex;
                break;
            case SLANG_STAGE_FRAGMENT:
                stage = ShaderStage::Fragment;
                break;
            case SLANG_STAGE_GEOMETRY:
                stage = ShaderStage::Geometry;
                break;
            case SLANG_STAGE_COMPUTE:
                stage = ShaderStage::Compute;
                break;
            default:
                DD_WARN("Slang: unknown stage, skipping entry point");
                continue;
            }

            slang::IComponentType* components[] = {module, entryPoint};
            Slang::ComPtr<slang::IComponentType> program;
            m_Session->createCompositeComponentType(components, 2, program.writeRef());

            Slang::ComPtr<slang::IComponentType> linked;
            Slang::ComPtr<slang::IBlob> diagnostics;
            program->link(linked.writeRef(), diagnostics.writeRef());
            if (diagnostics) DD_WARN("Slang link: {}", (const char*)diagnostics->getBufferPointer());

            Slang::ComPtr<slang::IBlob> code;
            linked->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef());
            if (diagnostics) DD_WARN("Slang codegen: {}", (const char*)diagnostics->getBufferPointer());

            if (!code) continue;

            ShaderStageSource stageSource;
            stageSource.stage = stage;
            stageSource.EntryPoint = reflection->getName();
            stageSource.source = std::string(static_cast<const char*>(code->getBufferPointer()), code->getBufferSize());
            result.stages.push_back(std::move(stageSource));
        }

        return result;
    }
} // namespace Dodo