#include "InspectorPanel.h"

#include "../FileDialog.h"

#include <Dodo.h>
#include <imgui.h>

void InspectorPanel::Draw(EditorState& state, InspectorState& inspector)
{
    static float translate[3] = {0.0f, 0.0f, 0.0f};
    static float scale[3] = {1.0f, 1.0f, 1.0f};
    static float rotate[3] = {0.0f, 0.0f, 0.0f};
    ImGui::Begin("Inspector");
    World &world = state.scene->GetWorld();
    for (int entityId : state.selection.entities)
    {
        static char nameBuffer[64] = "";
        if (inspector.dirty)
        {
            strncpy(nameBuffer, inspector.nameBuffer.c_str(), sizeof(nameBuffer) - 1);
            nameBuffer[sizeof(nameBuffer) - 1] = '\0';
        }

        if (ImGui::InputText("##label", nameBuffer, sizeof(nameBuffer),
                             ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsNoBlank))
        {
            inspector.nameBuffer = std::string(nameBuffer);

            if (!inspector.nameBuffer.empty())
            {
                if (world.HasComponent<NameComponent>(entityId))
                {
                    world.GetComponent<NameComponent>(entityId).m_Name = inspector.nameBuffer;
                } else
                {
                    world.AddComponent<NameComponent>(entityId, NameComponent{inspector.nameBuffer});
                }
            } else
            {
                if (world.HasComponent<NameComponent>(entityId))
                {
                    inspector.nameBuffer = world.GetComponent<NameComponent>(entityId).m_Name;
                    strncpy(nameBuffer, inspector.nameBuffer.c_str(), sizeof(nameBuffer) - 1);
                }
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
                            // Add empty ModelComponent for user to configure
                            // world.AddComponent<ModelComponent>(entityId,
                            // Application::s_Application->m_AssetManager->m_MeshFactory->GetRectangleMesh());
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
                ModelComponent &model = world.GetComponent<ModelComponent>(entityId);
                if (inspector.dirty)
                {
                    memcpy(translate, (float *)&model.m_Transformation.m_Position, 3 * sizeof(float));
                    memcpy(scale, (float *)&model.m_Transformation.m_Scale, 3 * sizeof(float));
                    memcpy(rotate, (float *)&model.m_Transformation.m_Rotation, 3 * sizeof(float));
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
                ImGui::Text(Application::s_Application->m_AssetManager->GetModelPath(model.m_ModelID).c_str());

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
                            memcpy(scale, (float *)&model.m_Transformation.m_Scale, sizeof(Math::Vec3));
                            model.m_Transformation.Calculate();
                        } else
                            model.m_Transformation.Scale(Math::Vec3(scale[0], scale[1], scale[2]));
                    }

                    ImGui::Text("Rotate:");
                    if (ImGui::DragFloat3("##rotate", rotate, 0.5f))
                    {
                        Math::Vec3 temp = ((Math::Vec3)rotate);
                        temp = Math::Vec3(std::fmod(temp.x, 360.0f), std::fmod(temp.y, 360.0f), std::fmod(temp.z, 360.0f));
                        memcpy(rotate, (float *)&temp, sizeof(Math::Vec3));
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

        if (inspector.dirty) inspector.dirty = false;
        break;
    }
    ImGui::End();
}