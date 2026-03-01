#include "SceneFile.h"
#include "pch.h"

#include "Core/Application/Application.h"
#include "Core/System/FileUtils.h"

#include <filesystem>

namespace Dodo {

    SceneFile::SceneFile(const std::string &path) : m_Path(path)
    {}

    // Public API
    SceneFileError SceneFile::Write(Scene *scene)
    {
        if (!HasPath())
        {
            SetError(SceneFileError::IOError);
            return m_LastError;
        }
        return WriteAs(m_Path, scene);
    }

    SceneFileError SceneFile::WriteAs(const std::string &path, Scene *scene)
    {
        m_Path = path;
        m_File.BeginWrite();

        // Header with version
        m_File.WriteComment("Scene v" + std::to_string(CURRENT_VERSION));
        m_File.WriteBlankLine();

        World &world = scene->GetWorld();
        for (EntityID entityId : world.GetAliveEntities())
        {
            m_File.WriteSection("Entity:" + std::to_string(entityId));

            // Write entity name
            if (world.HasComponent<NameComponent>(entityId))
            {
                m_File.WriteString("name", world.GetComponent<NameComponent>(entityId).m_Name);
            }

            // Write ModelComponent if entity has one
            if (world.HasComponent<ModelComponent>(entityId))
            {
                auto &comp = world.GetComponent<ModelComponent>(entityId);
                std::string path = Application::s_Application->m_AssetManager->GetModelPath(comp.m_ModelID);
                m_File.WriteSection("ModelComponent");
                m_File.WriteString("path", path);
                m_File.WriteVec3("position", comp.m_Transformation.m_Position);
                m_File.WriteVec3("scale", comp.m_Transformation.m_Scale);
                m_File.WriteVec3("rotation", comp.m_Transformation.m_Rotation);
            }

            m_File.WriteBlankLine();
        }

        m_File.EndWrite(path);
        m_LastError = SceneFileError::None;
        return m_LastError;
    }

    Scene *SceneFile::Read()
    {
        if (!HasPath())
        {
            SetError(SceneFileError::FileNotFound);
            return nullptr;
        }
        return Read(m_Path);
    }

    Scene *SceneFile::Read(const std::string &path)
    {
        m_Path = path;

        if (!m_File.BeginRead(path))
        {
            SetError(SceneFileError::FileNotFound);
            return nullptr;
        }

        // Skip header comment and blank lines until first section
        while (m_File.HasMore() && !m_File.IsSection())
        {
            m_File.SkipLine();
        }

        Scene *result =
            new Scene(new Math::FreeCamera(Math::Vec3(0.0f, 0.0f, 20.0f),
                                           (float)Application::s_Application->m_RenderAPI->m_ViewportWidth /
                                               (float)Application::s_Application->m_RenderAPI->m_ViewportHeight,
                                           0.04f, 10.0f));

        int currentEntityId = -1;

        while (m_File.HasMore())
        {
            // Skip blank lines
            if (!m_File.IsSection())
            {
                m_File.SkipLine();
                continue;
            }

            std::string section = m_File.ReadSection();

            // Entity header: Entity:0
            if (section.find("Entity:") == 0)
            {
                currentEntityId = std::stoi(section.substr(7));
                std::string name = m_File.ReadString();

                World &world = result->GetWorld();
                EntityID createdId = world.CreateEntity();
                currentEntityId = createdId;
                world.AddComponent<NameComponent>(currentEntityId, NameComponent{name});
                continue;
            }

            // Component sections
            else if (section == "ModelComponent")
            {
                if (currentEntityId < 0)
                {
                    SetError(SceneFileError::ParseError, m_File.GetCurrentOffset());
                    return result;
                }
                std::string modelPath = m_File.ReadString();
                Math::Vec3 position = m_File.ReadVec3();
                Math::Vec3 scale = m_File.ReadVec3();
                Math::Vec3 rotation = m_File.ReadVec3();
                Math::Transformation transform(position, scale, rotation);

                // This is temporary until we can retrieve ids without loading
                ModelID id = Application::s_Application->m_AssetManager->LoadModel(modelPath);

                result->GetWorld().AddComponent<ModelComponent>(currentEntityId, ModelComponent(id, transform));
                continue;
            } else
            {
                SetError(SceneFileError::UnsupportedComponent, m_File.GetCurrentOffset());
            }
        }

        m_File.EndRead();
        m_LastError = SceneFileError::None;
        return result;
    }

    void SceneFile::SetError(SceneFileError error, size_t line)
    {
        m_LastError = error;
        switch (error)
        {
        case SceneFileError::None:
            break;
        case SceneFileError::FileNotFound:
            DD_ERR("Scene file not found: {}", m_Path);
            break;
        case SceneFileError::InvalidFormat:
            DD_ERR("Invalid scene file format: {}", m_Path);
            break;
        case SceneFileError::VersionMismatch:
            DD_ERR("Scene file version mismatch: {}", m_Path);
            break;
        case SceneFileError::CorruptedData:
            DD_ERR("Scene file is corrupted: {}", m_Path);
            break;
        case SceneFileError::IOError:
            DD_ERR("IO error while accessing scene file: {}", m_Path);
            break;
        case SceneFileError::ParseError:
            if (line > 0)
                DD_ERR("Parse error in scene file '{}' at line {}", m_Path, line);
            else
                DD_ERR("Parse error in scene file: {}", m_Path);
            break;
        case SceneFileError::UnsupportedComponent:
            if (line > 0)
                DD_ERR("Unsupported component in scene file '{}' at line {}", m_Path, line);
            else
                DD_ERR("Unsupported component in scene file: {}", m_Path);
            break;
        }
    }

} // namespace Dodo