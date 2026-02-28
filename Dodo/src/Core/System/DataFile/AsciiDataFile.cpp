#include "pch.h"
#include "AsciiDataFile.h"

namespace Dodo {

	bool AsciiDataFile::BeginRead(const std::string& path, bool throwOnFail)
	{
		m_File.clear();
		std::ifstream in(path);
		if (!in) {
			if (throwOnFail) throw std::runtime_error("Failed to open ASCII file: " + path);
			return false;
		}

		std::string line;
		while (std::getline(in, line)) m_File.push_back(std::move(line));
		m_Offset = 0;
		return true;
	}

	void AsciiDataFile::EndRead()
	{
		m_File.clear();
		m_Offset = 0;
	}

	void AsciiDataFile::BeginWrite()
	{
		m_File.clear();
		m_Offset = 0;
	}

	void AsciiDataFile::EndWrite(const std::string& path)
	{
		std::ofstream out(path);
		for (auto& line : m_File) out << line << "\n";
	}

	std::size_t AsciiDataFile::GetCurrentOffset() const
	{
		return m_Offset;
	}

	// Section handling
	void AsciiDataFile::WriteSection(const std::string& name)
	{
		m_File.push_back("[" + name + "]");
	}

	std::string AsciiDataFile::ReadSection()
	{
		if (m_Offset >= m_File.size()) return "";
		
		const std::string& line = m_File[m_Offset];
		if (line.empty() || line[0] != '[') return "";
		
		size_t end = line.find(']');
		if (end == std::string::npos) return "";
		
		++m_Offset;
		return line.substr(1, end - 1);
	}

	bool AsciiDataFile::IsSection() const
	{
		if (m_Offset >= m_File.size()) return false;
		const std::string& line = m_File[m_Offset];
		return !line.empty() && line[0] == '[';
	}

	// Value writing
	void AsciiDataFile::WriteString(const std::string& key, const std::string& val)
	{
		m_File.push_back(key + "=\"" + val + "\"");
	}

	void AsciiDataFile::WriteInt(const std::string& key, int val)
	{
		m_File.push_back(key + "=" + std::to_string(val));
	}

	void AsciiDataFile::WriteVec2(const std::string& key, const Math::Vec2& v)
	{
		m_File.push_back(key + "=" + std::to_string(v.x) + "," + std::to_string(v.y));
	}
	
	void AsciiDataFile::WriteVec3(const std::string& key, const Math::Vec3& v)
	{
		m_File.push_back(key + "=" + std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z));
	}

	void AsciiDataFile::WriteVec4(const std::string& key, const Math::Vec4& v)
	{
		m_File.push_back(key + "=" + std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z) + "," + std::to_string(v.w));
	}

	void AsciiDataFile::WriteFloat(const std::string& key, float val)
	{
		m_File.push_back(key + "=" + std::to_string(val));
	}


	// Value reading
	std::string AsciiDataFile::ReadString()
	{
		if (m_Offset >= m_File.size()) {
			DD_WARN("No more lines in this file!");
			return "";
		}
		std::string line = m_File[m_Offset++];
		size_t quote = line.find('"');
		if (quote == std::string::npos || quote == 0 || line[quote - 1] != '=') {
			DD_WARN("Not able to retrieve string at line: {}", m_Offset - 1);
			return "";
		}

		std::string s = line.substr(quote + 1);
		if (!s.empty() && s.back() == '"') s.pop_back();
		return s;
	}

	int AsciiDataFile::ReadInt()
	{
		if (m_Offset >= m_File.size()) return 0;
		std::string line = m_File[m_Offset++];
		size_t eq = line.find('=');
		if (eq == std::string::npos) return 0;
		return std::stoi(line.substr(eq + 1));
	}

	float AsciiDataFile::ReadFloat()
	{
		if (m_Offset >= m_File.size()) return 0.0f;
		std::string line = m_File[m_Offset++];
		size_t eq = line.find('=');
		if (eq == std::string::npos) return 0.0f;
		return std::stof(line.substr(eq + 1));
	}

	Math::Vec2 AsciiDataFile::ReadVec2()
	{
		if (m_Offset >= m_File.size()) {
			DD_WARN("No more lines in this file!");
			return Math::Vec2(0.0f);
		}

		std::string line = m_File[m_Offset++];
		size_t eq = line.find('=');
		size_t comma1 = line.find(',', eq + 1);
		if (eq == std::string::npos || comma1 == std::string::npos) {
			DD_ERR("Trying to read invalid Vec2!");
			return Math::Vec2(0.0f);
		}
		float x = std::stof(line.substr(eq + 1, comma1 - eq - 1));
		float y = std::stof(line.substr(comma1 + 1));
		return { x, y };
	}

	Math::Vec3 AsciiDataFile::ReadVec3()
	{
		if (m_Offset >= m_File.size()) {
			DD_WARN("No more lines in this file!");
			return Math::Vec3(0.0f);
		}

		std::string line = m_File[m_Offset++];
		size_t eq = line.find('=');
		size_t comma1 = line.find(',', eq + 1);
		size_t comma2 = line.find(',', comma1 + 1);
		if (eq == std::string::npos || comma1 == std::string::npos || comma2 == std::string::npos) {
			DD_ERR("Trying to read invalid Vec3!");
			return Math::Vec3(0.0f);
		}
		float x = std::stof(line.substr(eq + 1, comma1 - eq - 1));
		float y = std::stof(line.substr(comma1 + 1, comma2 - comma1 - 1));
		float z = std::stof(line.substr(comma2 + 1));
		return { x, y, z };
	}

	Math::Vec4 AsciiDataFile::ReadVec4()
	{
		if (m_Offset >= m_File.size()) {
			DD_ERR("No more lines in this file!");
			return Math::Vec4(0.0f);
		}

		std::string line = m_File[m_Offset++];
		size_t eq = line.find('=');
		size_t comma1 = line.find(',', eq + 1);
		size_t comma2 = line.find(',', comma1 + 1);
		size_t comma3 = line.find(',', comma2 + 1);
		if (eq == std::string::npos || comma1 == std::string::npos || comma2 == std::string::npos || comma3 == std::string::npos) {
			DD_ERR("Trying to read invalid Vec4!");
			return Math::Vec4(0.0f);
		}
		float x = std::stof(line.substr(eq + 1, comma1 - eq - 1));
		float y = std::stof(line.substr(comma1 + 1, comma2 - comma1 - 1));
		float z = std::stof(line.substr(comma2 + 1, comma3 - comma2 - 1));
		float w = std::stof(line.substr(comma3 + 1));
		return { x, y, z, w };
	}

	// Utility
	void AsciiDataFile::WriteComment(const std::string& comment)
	{
		m_File.push_back("# " + comment);
	}

	void AsciiDataFile::WriteBlankLine()
	{
		m_File.push_back("");
	}

	void AsciiDataFile::SkipLine()
	{
		if (m_Offset < m_File.size()) ++m_Offset;
	}

	// Additional ASCII-specific helpers
	Math::Transformation AsciiDataFile::GetTransformation()
	{
		Math::Transformation t;
		if (m_Offset + 2 >= m_File.size()) return t;
		t.m_Position = ReadVec3();
		t.m_Scale = ReadVec3();
		t.m_Rotation = ReadVec3();
		return t;
	}

	void AsciiDataFile::AddValue(const std::string& name, const Math::Transformation& t)
	{
		WriteVec3(name + "_pos", t.m_Position);
		WriteVec3(name + "_scale", t.m_Scale);
		WriteVec3(name + "_rot", t.m_Rotation);
	}

}