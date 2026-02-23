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
		for (auto& ent : scene->m_Entities)
		{
			m_File.CreateSection(std::to_string(ent.first));
			m_File.AddValue("name", ent.second.m_Name);
			if (ent.second.m_ComponentFlags != ComponentFlag_None)
			{
				m_File.CreateSection("components");
				// Components
				for (auto& varcomp : ent.second.m_Components)
				{
					switch (varcomp.index())
					{
						case 0:
						{
							auto& comp = std::get<0>(varcomp);
							m_File.CreateSection("ModelComponent");
							m_File.AddValue("path", comp->m_Path);
							m_File.AddValue("transformation", comp->m_Transformation);
							m_File.UnIndent();
							break;
						}
						case 1:
						{
							auto& comp = std::get<1>(varcomp);
							m_File.CreateSection("Rectangle2D");
							m_File.AddValue("path", comp->m_Path);
							m_File.AddValue("transformation", comp->m_Transformation);
							m_File.UnIndent();
							break;
						}
						default:
							DD_ERR("Missing implementation while writing to scene file!");
					}
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
						if (!m_File.EntryExists("name"))
						{
							Error(m_File.m_CurrentLine);
							return result;
						}
						result->CreateEntity(id, m_File.GetString());
						if (!m_File.EntryExists("components"))
						{
							Error(m_File.m_CurrentLine);
							return result;
						}
						// Components
						m_File.NextLine();
						std::string section = m_File.GetSection();
						if (section == "ModelComponent")
						{
							std::string path = m_File.GetString();
							if (path == "") Error(m_File.m_CurrentLine);
							result->AddComponent(id, new ModelComponent(path.c_str(), m_File.GetTransformation()));
						}
						else if (section == "Rectangle2D")
						{
							std::string path = m_File.GetString();
							if (path == "") Error(m_File.m_CurrentLine);
							result->AddComponent(id, new Rectangle2DComponent(path.c_str(), m_File.GetTransformation()));
						}
						else
						{
							Error(m_File.m_CurrentLine);
						}
					}
					catch (...)
					{
						break; // No more entities
					}
				}
			}
		}
		return result;
	}
}