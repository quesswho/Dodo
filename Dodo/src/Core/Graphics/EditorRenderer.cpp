#include "pch.h"
#include "EditorRenderer.h"

namespace Dodo {

	void EditorRenderer::RenderEntities(World& world, Math::FreeCamera* camera, LightSystem& lightSystem) {
		for (auto& ent : world.m_Entities)
		{
			for (auto i : ent.second.m_Drawable)
			{
				auto& drawable = ent.second.m_Components[i];
				switch (drawable.index())
				{
				case 0:
					std::get<0>(drawable)->Draw(camera, lightSystem);
					break;
				case 1:
					std::get<1>(drawable)->Draw(camera, lightSystem);
					break;
				default:
					DD_ERR("This should never occurr.");
					break;
				}
			}
		}
	}

	void EditorRenderer::DrawScene(Scene* scene) {
        World& world = scene->GetWorld();
        RenderEntities(world, m_Camera, scene->m_LightSystem);
		if (scene->m_SkyBox) scene->m_SkyBox->Draw(m_Camera->GetViewMatrix());
	}
}