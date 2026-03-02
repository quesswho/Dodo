#include "InspectorPanel.h"

#include "../FileDialog.h"

#include <Dodo.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

void InspectorPanel::Draw(EditorState& state, InspectorState& inspector)
{
    static float translate[3] = {0.0f, 0.0f, 0.0f};
    static float scale[3] = {1.0f, 1.0f, 1.0f};
    static float rotate[3] = {0.0f, 0.0f, 0.0f};
    ImGui::Begin("Inspector");
    World& world = state.scene->GetWorld();
    for (int entityId : state.selection.entities)
    {
        if(state.renameState.entityId != entityId) {
            std::string name = world.HasComponent<NameComponent>(entityId) ? world.GetComponent<NameComponent>(entityId).name : "Entity_" + std::to_string(entityId);
            ImGui::Text(name.c_str());
            if(ImGui::IsItemClicked()) {
                state.renameState.Begin(world, entityId);
            }
        }

        if(state.renameState.entityId == entityId) {
            ImGui::SetKeyboardFocusHere();
            if (ImGui::InputText("##label", &state.renameState.nameBuffer, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsNoBlank))
            {
                state.renameState.Finish(world);
            }
            
        }

        ImGui::Separator();
        ImGui::Text("Components:");

        if (ImGui::BeginPopupContextWindow("Right-Click Inspector", ImGuiPopupFlags_MouseButtonRight))
        {
            if (ImGui::BeginMenu("Add.."))
            {
                if (ImGui::BeginMenu("Geometry"))
                {
                    if (ImGui::MenuItem("ModelComponent"))
                    {
                        if (!world.HasComponent<ModelComponent>(entityId))
                        {
                            ModelID id = Application::s_Application->m_AssetManager->GetBuiltinModel(BuiltinModel::Cube);
                            world.AddComponent<ModelComponent>(entityId, ModelComponent(id, Math::Transformation()));
                        }
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Script"))
                {
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Audio"))
                {
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Other"))
                {
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            ImGui::EndPopup();
        }

        // ModelComponent
        if (world.HasComponent<ModelComponent>(entityId))
        {
            if (ImGui::TreeNodeEx("ModelComponent", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ModelComponent& model = world.GetComponent<ModelComponent>(entityId);
                if (inspector.dirty)
                {
                    memcpy(translate, (float*)&model.m_Transformation.m_Position, 3 * sizeof(float));
                    memcpy(scale, (float*)&model.m_Transformation.m_Scale, 3 * sizeof(float));
                    memcpy(rotate, (float*)&model.m_Transformation.m_Rotation, 3 * sizeof(float));
                    rotate[0] = Math::ToDegrees(rotate[0]);
                    rotate[1] = Math::ToDegrees(rotate[1]);
                    rotate[2] = Math::ToDegrees(rotate[2]);
                    inspector.dirty = false;
                }
                ImGui::Indent();
                if (ImGui::Button("Browse"))
                {
                    std::filesystem::path path = FileDialog::OpenFile("Open file", "Model\0*.fbx;*.obj\0");
                    if (!path.empty())
                    {
                        // Replace the component with new model
                        ModelID id = Application::s_Application->m_AssetManager->LoadModel(path.string());
                        world.GetComponent<ModelComponent>(entityId) = ModelComponent(id, model.m_Transformation);
                    }
                }
                ImGui::SameLine();

                // Check if it's a built-in model first to avoid error logs
                if (Application::s_Application->m_AssetManager->HasPath(model.m_ModelID))
                {
                    std::string modelPath = Application::s_Application->m_AssetManager->GetModelPath(model.m_ModelID);
                    if (modelPath.empty())
                    {
                        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Path not found");
                    } else
                    {
                        ImGui::Text("%s", modelPath.c_str());
                    }
                } else
                {
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Built-in model");
                }

                if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::Text("Translate:");
                    if (ImGui::DragFloat3("##translate", translate, 0.05f))
                    {
                        model.m_Transformation.Move(Math::Vec3(translate[0], translate[1], translate[2]));
                    }

                    ImGui::Text("Scale:");
                    static bool sync = true;
                    bool dragscale = false;
                    dragscale = ImGui::DragFloat3("##scale", scale, 0.0001f, -100000.0f, 100000.0f, "%.4f", 1.001f);
                    ImGui::Checkbox("Sync", &sync);
                    if (dragscale)
                    {
                        if (sync)
                        {
                            Math::Vec3 temp = ((Math::Vec3)scale) - model.m_Transformation.m_Scale;
                            model.m_Transformation.m_Scale += temp.x;
                            model.m_Transformation.m_Scale += temp.y;
                            model.m_Transformation.m_Scale += temp.z;
                            memcpy(scale, (float*)&model.m_Transformation.m_Scale, sizeof(Math::Vec3));
                            model.m_Transformation.Calculate();
                        } else
                            model.m_Transformation.Scale(Math::Vec3(scale[0], scale[1], scale[2]));
                    }

                    ImGui::Text("Rotate:");
                    if (ImGui::DragFloat3("##rotate", rotate, 0.5f))
                    {
                        Math::Vec3 temp = ((Math::Vec3)rotate);
                        temp =
                            Math::Vec3(std::fmod(temp.x, 360.0f), std::fmod(temp.y, 360.0f), std::fmod(temp.z, 360.0f));
                        memcpy(rotate, (float*)&temp, sizeof(Math::Vec3));
                        model.m_Transformation.Rotate(temp);
                    }

                    ImGui::TreePop();
                }
                ImGui::Unindent();
                ImGui::TreePop();
            }
        }

        // Show message if no components
        if (!world.HasAnyComponent(entityId))
        {
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.34f, 129.0f, 0, 255), "Right click here!");
        }

        break;
    }
    ImGui::End();
}