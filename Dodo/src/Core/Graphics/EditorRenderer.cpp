#include "pch.h"
#include "EditorRenderer.h"
#include "Core/Application/Application.h"

namespace Dodo {

	void EditorRenderer::RenderEntities(World& world, Math::FreeCamera* camera, LightSystem& lightSystem, AssetManager& assets) {
		// Draw ModelComponent
		const auto& modelPool = world.GetPool<ModelComponent>();
		for (const auto& modelComponent : modelPool.GetComponents()) {
            Model* model = assets.GetModel(modelComponent.m_ModelID);
            for(auto mesh : model->GetMeshes()) { 
                Ref<Material> mat = mesh->GetMaterial(); 
                mat->Bind(); 
                mat->SetUniform("u_LightCamera", lightSystem.m_Directional.m_LightCamera); 
                mat->SetUniform("u_LightDir", lightSystem.m_Directional.m_Direction); 
                mat->SetUniform("u_Model", modelComponent.m_Transformation.m_Model); 
                mat->SetUniform("u_Camera", camera->GetCameraMatrix()); 
                mat->SetUniform("u_CameraPos", camera->GetCameraPos()); 
                mesh->DrawGeometry(); 
            }
		}
	}

	void EditorRenderer::DrawScene(Scene* scene) {
        World& world = scene->GetWorld();
        RenderEntities(world, m_Camera, scene->m_LightSystem, *Application::s_Application->m_AssetManager);
		if (scene->m_SkyBox) scene->m_SkyBox->Draw(m_Camera->GetViewMatrix());
	}
}