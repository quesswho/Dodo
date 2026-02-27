#pragma once

#include "Core/Math/Matrix/Transformation.h"

#include <cstddef>
#include <string>

namespace Dodo {

    enum class DataFileType {
        ASCII,
        BINARY
    };

    class IDataFile {
    public:
        virtual ~IDataFile() = default;

        // File operations
        virtual bool BeginRead(const std::string& path, bool throwOnFail = false) = 0;
        virtual void EndRead() = 0;
        virtual void BeginWrite() = 0;
        virtual void EndWrite(const std::string& path) = 0;

        // Position tracking
        virtual std::size_t GetCurrentOffset() const = 0;
        virtual bool HasMore() const = 0;

        // Section handling
        virtual void WriteSection(const std::string& name) = 0;
        virtual std::string ReadSection() = 0;  // Returns section name or empty if not a section
        virtual bool IsSection() const = 0;     // Check if current position is a section header

        // Value writing
        virtual void WriteString(const std::string& key, const std::string& val) = 0;
        virtual void WriteInt(const std::string& key, int val) = 0;
        virtual void WriteFloat(const std::string& key, float val) = 0;
        virtual void WriteVec2(const std::string& key, const Math::Vec2& v) = 0;
        virtual void WriteVec3(const std::string& key, const Math::Vec3& v) = 0;
        virtual void WriteVec4(const std::string& key, const Math::Vec4& v) = 0;

        // Value reading
        virtual std::string ReadString() = 0;
        virtual int ReadInt() = 0;
        virtual float ReadFloat() = 0;
        virtual Math::Vec2 ReadVec2() = 0;
        virtual Math::Vec3 ReadVec3() = 0;
        virtual Math::Vec4 ReadVec4() = 0;

        // Utility
        virtual void WriteComment(const std::string& comment) = 0;
        virtual void WriteBlankLine() = 0;
        virtual void SkipLine() = 0;
    };

}
