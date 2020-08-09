#include "pch.h"
#include "AsciiSceneFile.h"
#include "Core/System/FileUtils.h"
#include "Core/Application/Application.h"

namespace Dodo {
	
	AsciiSceneFile::AsciiSceneFile()
	{}

	void AsciiSceneFile::Write(const char* path, Scene* scene)
	{
		m_File.BeginWrite();

		m_File.CreateSection("Entities");
		for (auto ent : scene->m_Entities)
		{
			m_File.CreateSection(std::to_string(ent.first));
			m_File.AddValue("name", ent.second.m_Name);
			if (ent.second.m_ComponentFlags != ComponentFlag_None)
			{
				m_File.CreateSection("components");
				if(ent.second.m_ComponentFlags & ComponentFlag_ModelComponent)
				{
					m_File.CreateSection("ModelComponent");
					m_File.AddValue("path", scene->m_ModelComponent[ent.first]->m_Path);
					m_File.AddValue("transformation", scene->m_ModelComponent[ent.first]->m_Transformation);
					m_File.UnIndent();
				}
				m_File.UnIndent();
			}
			m_File.UnIndent();
		}
		m_File.UnIndent();

		m_File.EndWrite(path);
	}

	Scene* AsciiSceneFile::Read(const char* path)
	{
		Application::s_Application->m_Window->DefaultWorkDirectory();
		Scene* result = new Scene(new Math::FreeCamera(Math::Vec3(0.0f, 0.0f, 20.0f), (float)Application::s_Application->m_RenderAPI->m_ViewportWidth / (float)Application::s_Application->m_RenderAPI->m_ViewportWidth, 0.04f, 10.0f));
		m_File.BeginRead(path);
		while (m_File.m_File.size() > m_File.m_CurrentLine)
		{
			if (m_File.GetSection() == "Entities")
			{
				while (true)
				{
					try {
						int id = std::stoi(m_File.GetSection());
						if (m_File.EntryExists("name"))
						{
							result->CreateEntity(id, m_File.GetString());
							if (m_File.EntryExists("components"))
							{
								m_File.NextLine();
								if (m_File.GetSection() == "ModelComponent" && m_File.EntryExists("path"))
								{
									std::string path = m_File.GetString();
									result->AddComponent(id, new ModelComponent(path.c_str(), m_File.GetTransformation()));
								}
								else
									Error(m_File.m_CurrentLine);
							}
							else
								Error(m_File.m_CurrentLine);
						}
						else
							Error(m_File.m_CurrentLine);
					}
					catch (...)
					{
						break; // No more entities
					}
					if (!m_File.GetLine(m_File.m_CurrentLine)._Starts_with("\t"))
						break;
				}
			}
		}
		return result;
	}
}