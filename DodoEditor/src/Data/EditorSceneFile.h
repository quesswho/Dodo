#pragma once

#include "Components/NameComponent.h"
#include "Scene/EditorScene.h"

#include <Dodo.h>

using namespace Dodo;

class EditorSceneFile {
  public:
    EditorSceneFile() = default;
    explicit EditorSceneFile(const std::string& path) : m_Path(path) {}

    SceneFileError Write(EditorScene* scene);
    SceneFileError WriteAs(const std::string& path, EditorScene* scene);

    EditorScene* Read();
    EditorScene* Read(const std::string& path);

    const std::string& GetPath() const { return m_Path; }
    bool HasPath() const { return !m_Path.empty(); }
    SceneFileError GetLastError() const { return m_LastError; }

    SceneFile& GetCoreSceneFile() { return m_Core; }

  private:
    void SetError(SceneFileError error) { m_LastError = error; }

    std::string m_Path;
    SceneFileError m_LastError = SceneFileError::None;
    SceneFile m_Core;
};