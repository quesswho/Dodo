#include "EditorSceneFile.h"

SceneFileError EditorSceneFile::Write(EditorScene* scene)
{
    if (!HasPath())
    {
        SetError(SceneFileError::IOError);
        return m_LastError;
    }
    return WriteAs(m_Path, scene);
}

SceneFileError EditorSceneFile::WriteAs(const std::string& path, EditorScene* scene)
{
    m_Path = path;

    // Pass 1: write core/runtime data
    SceneFileError coreErr = m_Core.WriteEntities(path, &scene->GetRuntimeScene());
    if (coreErr != SceneFileError::None)
    {
        m_LastError = coreErr;
        return m_LastError;
    }

    // Pass 2: append editor-only data to the same buffer
    AsciiDataFile& file = m_Core.GetFile();

    file.WriteSection("Editor");

    auto& world = scene->GetWorld();
    for (EntityID entityId : world.GetAliveEntities())
    {
        if (!world.HasComponent<NameComponent>(entityId))
            continue;

        const auto& name = world.GetComponent<NameComponent>(entityId).name;
        file.WriteSection("NameComponent");
        file.WriteInt("id", entityId);
        file.WriteString("name", name);
        file.WriteBlankLine();
    }

    // Flush everything to disk in one pass
    file.EndWrite(path);

    m_LastError = SceneFileError::None;
    return m_LastError;
}

EditorScene* EditorSceneFile::Read()
{
    if (!HasPath())
    {
        SetError(SceneFileError::FileNotFound);
        return nullptr;
    }
    return Read(m_Path);
}

EditorScene* EditorSceneFile::Read(const std::string& path)
{
    m_Path = path;

    // Pass 1: read core/runtime data
    Scene* runtimeScene = m_Core.ReadEntities(path);
    if (!runtimeScene)
    {
        SetError(m_Core.GetLastError());
        return nullptr;
    }

    EditorScene* editorScene = new EditorScene(runtimeScene);

    // Pass 2: reset file offset and read editor-only data
    AsciiDataFile& file = m_Core.GetFile();
    file.ResetOffset();

    auto& world = editorScene->GetWorld();
    bool inEditor = false;

    while (file.HasMore())
    {
        if (!file.IsSection())
        {
            file.SkipLine();
            continue;
        }

        std::string section = file.ReadSection();

        if (section == "Editor")
        {
            inEditor = true;
            continue;
        }
        
        // Skip core components
        if (!inEditor)
            continue;

        if (section == "NameComponent")
        {
            EntityID id = (EntityID)file.ReadInt();
            std::string name = file.ReadString();
            world.AddComponent<NameComponent>(id, NameComponent{name});
            continue;
        }

        // Skip unknown editor sections
        while (file.HasMore() && !file.IsSection())
            file.SkipLine();
    }

    file.EndRead();
    m_LastError = SceneFileError::None;
    return editorScene;
}
