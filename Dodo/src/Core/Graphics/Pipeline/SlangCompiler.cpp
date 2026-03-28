#include "SlangCompiler.h"

#include "Core/System/FileUtils.h"

namespace Dodo {
    SlangCompiler::SlangCompiler()
    {
        SlangGlobalSessionDesc globalDesc = {};
        slang::createGlobalSession(&globalDesc, m_GlobalSession.writeRef());

        slang::TargetDesc targetDesc = {};
        targetDesc.format = SLANG_SPIRV;
        targetDesc.profile = m_GlobalSession->findProfile("spirv_1_5");

        slang::SessionDesc sessionDesc = {};
        sessionDesc.targets = &targetDesc;
        sessionDesc.targetCount = 1;

        // Default to column-major layout to match the engine's matrix implementation
        sessionDesc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;

        m_GlobalSession->createSession(sessionDesc, m_Session.writeRef());
    }

    SlangCompiler::~SlangCompiler()
    {
        m_Session.setNull();
        m_GlobalSession.setNull();
    }

    ShaderAsset SlangCompiler::CompileFile(const std::string& path)
    {
        std::string slangSource = FileUtils::ReadTextFile(path.c_str());
        if (slangSource == "-1") {
            DD_ERR("Failed to read slang source file: {}", path);
            return {};
        }

        Slang::ComPtr<slang::IBlob> diagnostics;

        slang::IModule* module = m_Session->loadModule(path.c_str(), diagnostics.writeRef());

        if (diagnostics) {
            DD_WARN("Slang: {}", static_cast<const char*>(diagnostics->getBufferPointer()));
        }
        if (!module) {
            DD_ERR("Failed to load slang module: {}", path);
            return {};
        }

        return CompileModule(module, path, std::move(slangSource));
    }

    ShaderAsset SlangCompiler::CompileFromString(const std::string& source, const std::string& name)
    {
        Slang::ComPtr<slang::IBlob> diagnostics;

        slang::IModule* module =
            m_Session->loadModuleFromSourceString(name.c_str(), name.c_str(), source.c_str(), diagnostics.writeRef());
        if (diagnostics) {
            DD_WARN("Slang: {}", static_cast<const char*>(diagnostics->getBufferPointer()));
        }
        if (!module) {
            DD_ERR("Failed to compile slang source: {}, {}", name, source);
            return {};
        }

        return CompileModule(module, name, source);
    }

    ShaderAsset SlangCompiler::CompileModule(slang::IModule* module, const std::string& path, std::string slangSource)
    {
        ShaderAsset result;
        result.path = path;
        result.slangSource = std::move(slangSource);

        const int entryPointCount = module->getDefinedEntryPointCount();
        for (int i = 0; i < entryPointCount; ++i) {
            Slang::ComPtr<slang::IEntryPoint> entryPoint;
            module->getDefinedEntryPoint(i, entryPoint.writeRef());

            ShaderStageBinary stage = CompileEntryPoint(module, entryPoint);
            if (stage.stage == ShaderStage::Unknown) continue;

            result.stages.push_back(std::move(stage));
        }

        return result;
    }

    ShaderStageBinary SlangCompiler::CompileEntryPoint(slang::IModule* module, slang::IEntryPoint* entryPoint)
    {
        ShaderStageBinary result;

        slang::EntryPointReflection* reflection = entryPoint->getLayout()->getEntryPointByIndex(0);
        if (!reflection) {
            DD_WARN("Slang: failed to reflect entry point");
            return result;
        }

        result.stage = GetShaderStage(reflection->getStage());
        result.entryPoint = reflection->getName();

        if (result.stage == ShaderStage::Unknown) {
            DD_WARN("Slang: unknown stage, skipping entry point");
            return result;
        }

        slang::IComponentType* components[] = {module, entryPoint};
        Slang::ComPtr<slang::IComponentType> program;
        SlangResult compositeResult = m_Session->createCompositeComponentType(components, 2, program.writeRef());
        if (SLANG_FAILED(compositeResult) || !program) {
            DD_ERR("Slang: failed to create composite component for {}", result.entryPoint);
            result.stage = ShaderStage::Unknown;
            return result;
        }

        Slang::ComPtr<slang::IComponentType> linked;
        Slang::ComPtr<slang::IBlob> diagnostics;
        SlangResult linkResult = program->link(linked.writeRef(), diagnostics.writeRef());
        if (diagnostics) DD_WARN("Slang link: {}", static_cast<const char*>(diagnostics->getBufferPointer()));
        if (SLANG_FAILED(linkResult) || !linked) {
            DD_ERR("Slang: failed to link entry point {}", result.entryPoint);
            result.stage = ShaderStage::Unknown;
            return result;
        }

        Slang::ComPtr<slang::IBlob> code;
        SlangResult codeResult = linked->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef());
        if (diagnostics) DD_WARN("Slang codegen: {}", static_cast<const char*>(diagnostics->getBufferPointer()));
        if (SLANG_FAILED(codeResult) || !code) {
            DD_ERR("Slang: failed to generate code for entry point {}", result.entryPoint);
            result.stage = ShaderStage::Unknown;
            return result;
        }

        result.spirv = BlobToSPIRV(code);
        if (result.spirv.empty()) {
            DD_ERR("Slang: generated empty SPIR-V for entry point {}", result.entryPoint);
            result.stage = ShaderStage::Unknown;
            return result;
        }

        return result;
    }

    ShaderStage SlangCompiler::GetShaderStage(SlangStage stage)
    {
        switch (stage) {
        case SLANG_STAGE_VERTEX:
            return ShaderStage::Vertex;
        case SLANG_STAGE_FRAGMENT:
            return ShaderStage::Fragment;
        case SLANG_STAGE_GEOMETRY:
            return ShaderStage::Geometry;
        case SLANG_STAGE_COMPUTE:
            return ShaderStage::Compute;
        default:
            return ShaderStage::Unknown;
        }
    }

    std::vector<uint32_t> SlangCompiler::BlobToSPIRV(slang::IBlob* code)
    {
        if (code == nullptr || code->getBufferSize() == 0) return {};

        const size_t wordCount = code->getBufferSize() / sizeof(uint32_t);
        std::vector<uint32_t> spirv(wordCount);
        std::memcpy(spirv.data(), code->getBufferPointer(), wordCount * sizeof(uint32_t));
        return spirv;
    }

} // namespace Dodo
