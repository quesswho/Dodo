#pragma once

#include "Core/Graphics/Scene/Scene.h"
#include "Core/System/DataFile/AsciiDataFile.h"

#include <string>

namespace Dodo {

    enum class SceneFileError
    {
        None = 0,
        FileNotFound,
        InvalidFormat,
        VersionMismatch,
        CorruptedData,
        IOError,
        ParseError,
        UnsupportedComponent,
    };

    class SceneFile {
      public:
        static constexpr uint32_t CURRENT_VERSION = 1;

        SceneFile() = default;
        explicit SceneFile(const std::string &path);

        // Write/Read operations
        SceneFileError Write(Scene *scene);
        SceneFileError WriteAs(const std::string &path, Scene *scene);

        Scene *Read();
        Scene *Read(const std::string &path);

        const std::string &GetPath() const { return m_Path; }
        bool HasPath() const { return !m_Path.empty(); }
        SceneFileError GetLastError() const { return m_LastError; }

      private:
        void SetError(SceneFileError error, size_t line = 0);

        std::string m_Path;
        SceneFileError m_LastError = SceneFileError::None;

        AsciiDataFile m_File;
    };

} // namespace Dodo