#include "pch.h"
#include "AsciiSceneFile.h"
#include "Core/System/FileUtils.h"

namespace Dodo {
	
	AsciiSceneFile::AsciiSceneFile()
	{}

	void AsciiSceneFile::Write(const char* path, Scene* scene)
	{

		const char* indent = "	";

		if (FileUtils::FileExists(path))
		{

		}

		std::vector<std::string> result;
		
		result.emplace_back("Entities:");
		for (auto ent : scene->m_Entities)
		{
			result.emplace_back("	" + std::to_string(ent.first) + ":\n"
			"		name \"" + ent.second.m_Name + "\"");

			switch (ent.second.m_ComponentFlags)
			{
				case FlagModelComponent:
				{
					Math::Transformation trans = scene->m_ModelComponent[ent.first]->m_Transformation;
					result.emplace_back("		comp \"ModelComponent\":\n"
					"			path \"" + scene->m_ModelComponent[ent.first]->m_Path + "\"\n"
					"			transformation:\n"
					"				" + std::to_string(trans.m_Pos.x) + " " + std::to_string(trans.m_Pos.y) + " " + std::to_string(trans.m_Pos.z) + "\n"
					"				" + std::to_string(trans.m_Scale.x) + " " + std::to_string(trans.m_Scale.y) + " " + std::to_string(trans.m_Scale.z) + "\n"
					"				" + std::to_string(trans.m_Rotation.x) + " " + std::to_string(trans.m_Rotation.y) + " " + std::to_string(trans.m_Rotation.z));
					break;
				}
				case FlagNone:
					break;
			}

		}

		FileUtils::WriteTextFile(path, result);
	}
}