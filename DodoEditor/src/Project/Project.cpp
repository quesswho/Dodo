#include "Project.h"

#include <Core/System/DataFile/AsciiDataFile.h>

#include <filesystem>

namespace Dodo {

std::unique_ptr<Project> Project::New(const std::filesystem::path& parentDir, const std::string& name)
{
    std::filesystem::path root = parentDir / name;

    std::error_code ec;
    std::filesystem::create_directories(root / "assets" / "models", ec);
    std::filesystem::create_directories(root / "assets" / "textures", ec);
    std::filesystem::create_directories(root / "assets" / "materials", ec);
    std::filesystem::create_directories(root / "scenes", ec);

    if (ec)
        return nullptr;

    auto project = std::make_unique<Project>();
    project->m_Name = name;
    project->m_RootPath = root;

    project->Save();
    return project;
}

std::unique_ptr<Project> Project::Open(const std::filesystem::path& projectFile)
{
    AsciiDataFile file;
    if (!file.BeginRead(projectFile.string()))
        return nullptr;

    auto project = std::make_unique<Project>();
    project->m_RootPath = projectFile.parent_path();

    while (file.HasMore()) {
        if (!file.IsSection()) {
            file.SkipLine();
            continue;
        }

        std::string section = file.ReadSection();

        if (section == "Project") {
            project->m_Name = file.ReadString();
        } else if (section == "StartScene") {
            project->m_StartScene = file.ReadString();
        } else {
            // Skip unknown sections
            while (file.HasMore() && !file.IsSection())
                file.SkipLine();
        }
    }

    file.EndRead();

    return project->m_Name.empty() ? nullptr : std::move(project);
}

void Project::Save() const
{
    AsciiDataFile file;
    file.BeginWrite();

    file.WriteComment("Dodo Project v1");
    file.WriteBlankLine();

    file.WriteSection("Project");
    file.WriteString("name", m_Name);

    if (!m_StartScene.empty()) {
        file.WriteBlankLine();
        file.WriteSection("StartScene");
        file.WriteString("path", m_StartScene.generic_string());
    }

    file.EndWrite(GetProjectFilePath().string());
}

} // namespace Dodo
