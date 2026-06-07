#pragma once
#include <filesystem>
#include <memory>
#include <string>

namespace Dodo {

class Project {
  public:
    std::string m_Name;
    std::filesystem::path m_RootPath;
    std::filesystem::path m_StartScene; // relative to root

    /** Creates the workspace directory structure and a .dproject file. Returns null on failure. */
    static std::unique_ptr<Project> New(const std::filesystem::path& parentDir, const std::string& name);

    /** Reads a .dproject file. Returns null on failure. */
    static std::unique_ptr<Project> Open(const std::filesystem::path& projectFile);

    void Save() const;

    std::filesystem::path GetAssetsDir() const { return m_RootPath / "assets"; }
    std::filesystem::path GetScenesDir() const { return m_RootPath / "scenes"; }
    std::filesystem::path GetProjectFilePath() const { return m_RootPath / (m_Name + ".dproject"); }
};

} // namespace Dodo
