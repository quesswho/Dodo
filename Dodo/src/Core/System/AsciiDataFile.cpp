#include "pch.h"
#include "AsciiDataFile.h"

namespace Dodo {

	AsciiDataFile::AsciiDataFile()
		: m_CurrentLine(0), m_CurrentIndent(0), m_Indent({""})
	{
		m_CurrentLine = 0;
	}

	void AsciiDataFile::Clear()
	{
		m_File.clear();
		m_CurrentIndent = 0;
	}

	void AsciiDataFile::Indent()
	{ 
		m_CurrentIndent++;
		if (m_Indent.size()-1 < m_CurrentIndent)
		{
			std::string indent = "\t";
			for (int i = 0; i < m_Indent.size() - 1; i++)
				indent.append("\t");
			m_Indent.push_back(indent);
		}
	}

	void AsciiDataFile::UnIndent()
	{
		if(m_CurrentIndent > 0)
			m_CurrentIndent--;
	}


	void AsciiDataFile::BeginRead(const char* path)
	{
		m_File.clear();

		std::string temp;
		std::stringstream ss(FileUtils::ReadTextFile(path));

		while (std::getline(ss, temp, '\n'))
			m_File.push_back(temp);
		m_CurrentLine = 0;
	}

	std::string AsciiDataFile::GetEntryName(uint lineindex) const
	{
		std::string line = m_File[lineindex];
		size_t index = line.find(':');
		if (index == std::string::npos)
			if (index = line.find('=') != std::string::npos)
				return "";

		line.erase(index);

		if (line.find('\t') != std::string::npos)
			return line.substr(line.find_last_of('\t') + 1, line.size());
		else
			return line.substr(0, line.size());
	}

	std::string AsciiDataFile::GetSection()
	{
		std::string line = m_File[m_CurrentLine];
		m_CurrentLine++;
		size_t index = line.find(':');
		if (index == std::string::npos)
			if(index = line.find('=') != std::string::npos)
				return "";
		
		line.erase(index);

		if(line.find('\t') != std::string::npos)
			return line.substr(line.find_last_of('\t') + 1, line.size());
		else
			return line.substr(0, line.size());
	}

	std::string AsciiDataFile::GetString()
	{
		std::string line = m_File[m_CurrentLine];
		m_CurrentLine++;
		size_t quote = line.find('"');
		if (line[quote - 1] == '=')
		{
			line = line.substr(quote + 1);
			line.erase(line.end() - 1);
			return line;
		}

		DD_WARN("Not able to retrieve string at line: {}", m_CurrentLine-1);
		return "";
	}

	int AsciiDataFile::GetInt()
	{
		std::string str = m_File[m_CurrentLine];
		m_CurrentLine++;
		try
		{
			size_t loc = str.find('=');
			if(loc != std::string::npos)
				return std::stoi(str.substr(loc + 1));
		}
		catch (...)
		{}
		DD_WARN("Not able to retrieve int at line: {}", m_CurrentLine-1);
		return 0;
	}

	double AsciiDataFile::GetDouble()
	{
		std::string str = m_File[m_CurrentLine];
		m_CurrentLine++;
		try
		{
			size_t loc = str.find('=');
			if (loc != std::string::npos)
				return std::stod(str.substr(loc + 1));
		}
		catch (...)
		{
		}
		DD_WARN("Not able to retrieve double at line: {}", m_CurrentLine-1);
		return 0.0;
	}

	float AsciiDataFile::GetFloat()
	{
		std::string str = m_File[m_CurrentLine];
		m_CurrentLine++;
		try
		{
			size_t loc = str.find('=');
			if (loc != std::string::npos)
				return std::stof(str.substr(loc + 1));
		}
		catch (...)
		{
		}
		DD_WARN("Not able to retrieve float at line: {}", m_CurrentLine-1);
		return 0.0f;
	}

	Math::Vec2 AsciiDataFile::GetVec2()
	{
		std::string str = m_File[m_CurrentLine];
		m_CurrentLine++;
		try
		{
			size_t loc = str.find(',');
			return Math::Vec2(std::stof(str.substr(str.find('=') + 1, loc - 1)), std::stof(str.substr(loc + 1, str.size())));
		}
		catch (...)
		{
		}
		DD_WARN("Not able to retrieve vec2 at line: {}", m_CurrentLine-1);
		return Math::Vec2(0.0f);
	}

	Math::Vec3 AsciiDataFile::GetVec3()
	{
		std::string str = m_File[m_CurrentLine];
		m_CurrentLine++;
		try
		{
			size_t loc = str.find(',');
			float x = std::stof(str.substr(str.find('=') + 1, loc - 1));
			size_t loc2 = str.find(',', loc);
			float y = std::stof(str.substr(loc + 1, loc2 - 1));
			float z = std::stof(str.substr(loc2 + 1, str.size()));
			return Math::Vec3(x, y, z);
		}
		catch (...)
		{
		}
		DD_WARN("Not able to retrieve vec3 at line: {}", m_CurrentLine-1);
		return Math::Vec3(0.0f);
	}

	Math::Vec4 AsciiDataFile::GetVec4()
	{
		std::string str = m_File[m_CurrentLine];
		m_CurrentLine++;
		try
		{
			size_t loc = str.find(',');
			float x = std::stof(str.substr(str.find('=') + 1, loc - 1));
			size_t loc2 = str.find(',', loc);
			float y = std::stof(str.substr(loc + 1, loc2 - 1));
			loc = str.find(',', loc2);
			float z = std::stof(str.substr(loc2 + 1, loc - 1));
			float w = std::stof(str.substr(loc + 1, str.size()));
			return Math::Vec4(x, y, z, w);
		}
		catch (...)
		{
		}
		DD_WARN("Not able to retrieve vec4 at line: {}", m_CurrentLine-1);
		return Math::Vec4(0.0f);
	}

	Math::Transformation AsciiDataFile::GetTransformation()
	{
		try
		{
			std::string str = m_File[m_CurrentLine];
			size_t loc = str.find(',');
			float posX = std::stof(str.substr(str.find('=') + 1, loc - 1));
			size_t loc2 = str.find(',', loc);
			float posY = std::stof(str.substr(loc + 1 , loc2 - 1));
			float posZ = std::stof(str.substr(loc2 + 1, str.size()));
			str = m_File[m_CurrentLine + 1];
			loc = str.find(',');
			float scaleX = std::stof(str.substr(str.find('=') + 1, loc - 1));
			loc2 = str.find(',', loc);
			float scaleY = std::stof(str.substr(loc + 1, loc2 - 1));
			float scaleZ = std::stof(str.substr(loc2 + 1, str.size()));
			str = m_File[m_CurrentLine + 2];
			loc = str.find(',');
			float rotateX = std::stof(str.substr(str.find('=') + 1, loc - 1));
			loc2 = str.find(',', loc);
			float rotateY = std::stof(str.substr(loc + 1, loc2 - 1));
			float rotateZ = std::stof(str.substr(loc2 + 1, str.size()));

			m_CurrentLine+=3;
			return Math::Transformation(posX, posY, posZ, scaleX, scaleY, scaleZ, rotateX, rotateY, rotateZ);
		}
		catch (...)
		{
		}

		m_CurrentLine += 3;
		DD_WARN("Not able to retrieve transformation at line: {}", m_CurrentLine-3);
		return Math::Transformation();
	}
}