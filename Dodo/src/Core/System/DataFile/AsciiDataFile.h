#pragma once

#include <Core/Common.h>

#include "IDataFile.h"

#include "Core/Math/Matrix/Transformation.h"
#include "Core/System/FileUtils.h"

#include <string>
#include <vector>

namespace Dodo {

	class AsciiDataFile : public IDataFile {
	public:
		// IDataFile interface
		bool BeginRead(const std::string& path, bool throwOnFail = false) override;
		void EndRead() override;
		void BeginWrite() override;
		void EndWrite(const std::string& path) override;

		std::size_t GetCurrentOffset() const override;
		bool HasMore() const override { return m_Offset < m_File.size(); }

		// Section handling
		void WriteSection(const std::string& name) override;
		std::string ReadSection() override;
		bool IsSection() const override;

		// Value writing
		void WriteString(const std::string& key, const std::string& val) override;
		void WriteInt(const std::string& key, int val) override;
		void WriteFloat(const std::string& key, float val) override;
		void WriteVec2(const std::string& key, const Math::Vec2& v) override;
		void WriteVec3(const std::string& key, const Math::Vec3& v) override;
		void WriteVec4(const std::string& key, const Math::Vec4& v) override;

		// Value reading
		std::string ReadString() override;
		int ReadInt() override;
		float ReadFloat() override;
		Math::Vec2 ReadVec2() override;
		Math::Vec3 ReadVec3() override;
		Math::Vec4 ReadVec4() override;

		// Utility
		void WriteComment(const std::string& comment) override;
		void WriteBlankLine() override;
		void SkipLine() override;

		// Additional ASCII-specific helpers (not in interface)
		Math::Transformation GetTransformation();
		void AddValue(const std::string& name, const Math::Transformation& t);

	private:
		std::vector<std::string> m_File;
		std::size_t m_Offset{0};
	};

}