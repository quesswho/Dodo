#pragma once

#include "pch.h"
#include "EditorRenderer.h"

namespace Dodo {

	void EditorRenderer::DrawScene(Scene* scene) {
		for (auto& ent : scene->m_Entities)
		{
			for (auto i : ent.second.m_Drawable)
			{
				auto& drawable = ent.second.m_Components[i];
				switch (drawable.index())
				{
				case 0:
					std::get<0>(drawable)->Draw(m_Camera, scene->m_LightSystem);
					break;
				case 1:
					std::get<1>(drawable)->Draw(m_Camera, scene->m_LightSystem);
					break;
				default:
					DD_ERR("This should never occurr.");
					break;
				}
			}
		}

		if (scene->m_SkyBox) scene->m_SkyBox->Draw(m_Camera->GetViewMatrix());
	}
}