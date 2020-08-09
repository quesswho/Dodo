#pragma once
#include "Core/Common.h"
#include <string>
#include <vector>
#include "Core/Math/Matrix/Transformation.h"
#include "Core/System/FileUtils.h"

namespace Dodo {

	class AsciiDataFile
	{
	private:
		std::vector<std::string> m_Indent;
		uint m_CurrentIndent;
	public:
		AsciiDataFile();

		std::vector<std::string> m_File;
	
		void Clear();

		void Indent();
		void UnIndent();

		// Call UnIndent to get out of the scope
		inline void CreateSection(std::string name) { m_File.emplace_back(m_Indent[m_CurrentIndent] + name + ":"); Indent(); }

		inline void AddValue(std::string name, std::string data) { m_File.emplace_back(m_Indent[m_CurrentIndent] + name + "=\"" + data + "\""); }
		inline void AddValue(std::string name, int data) { m_File.emplace_back(m_Indent[m_CurrentIndent] + name + "=" + std::to_string(data)); }
		inline void AddValue(std::string name, double data) { m_File.emplace_back(m_Indent[m_CurrentIndent] + name + "=" + std::to_string(data)); }
		inline void AddValue(std::string name, float data) { m_File.emplace_back(m_Indent[m_CurrentIndent] + name + "=" + std::to_string(data)); }
		inline void AddValue(std::string name, Math::Vec2 data) { m_File.emplace_back(m_Indent[m_CurrentIndent] + name + "=" + std::to_string(data.x) + "," + std::to_string(data.y)); }
		inline void AddValue(std::string name, Math::Vec3 data) { m_File.emplace_back(m_Indent[m_CurrentIndent] + name + "=" + std::to_string(data.x) + "," + std::to_string(data.y) + "," + std::to_string(data.z)); }
		inline void AddValue(std::string name, Math::Vec4 data) { m_File.emplace_back(m_Indent[m_CurrentIndent] + name + "=" + std::to_string(data.x) + "," + std::to_string(data.y) + "," + std::to_string(data.z) + "," + std::to_string(data.w)); }
		inline void AddValue(std::string name, Math::Transformation data)
		{
			m_File.emplace_back(
				m_Indent[m_CurrentIndent] + name + "=" + std::to_string(data.m_Position.x) + "," + std::to_string(data.m_Position.y) + "," + std::to_string(data.m_Position.z) + "\n"
				+ m_Indent[m_CurrentIndent] + "\t=" + std::to_string(data.m_Scale.x) + "," + std::to_string(data.m_Scale.y) + "," + std::to_string(data.m_Scale.z) + "\n"
				+ m_Indent[m_CurrentIndent] + "\t=" + std::to_string(data.m_Rotation.x) + "," + std::to_string(data.m_Rotation.y) + "," + std::to_string(data.m_Rotation.z));
		}

		inline void BeginWrite() { m_CurrentIndent = 0; Clear(); }
		inline void EndWrite(const char* path) { FileUtils::WriteTextFile(path, m_File); }
		void BeginRead(const char* path); // Read line by line
		inline bool EndRead() { m_CurrentLine = 0; }
		
		inline void NextLine() { m_CurrentLine++; }
		std::string GetEntryName(uint line) const;
		inline bool EntryExists(const std::string& entry) const { return m_File[m_CurrentLine].find(entry) != std::string::npos; }
		std::string GetSection();
		std::string GetString();
		int GetInt();
		double GetDouble();
		float GetFloat();
		Math::Vec2 GetVec2();
		Math::Vec3 GetVec3();
		Math::Vec4 GetVec4();
		Math::Transformation GetTransformation();

		inline std::string GetLine(const uint line) const { return m_File[line]; }
	public:
		uint m_CurrentLine;
	};
}