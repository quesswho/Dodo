#include "InspectorPanel.h"

#include "FileDialog.h"

#include <Dodo.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

void InspectorPanel::Draw(EditorState& editorState, InspectorState& state)
{
    if (!ImGui::Begin(state.name.c_str(), &state.visible)) {
        ImGui::End();
        return;
    }

    auto& world = editorState.scene->GetWorld();
    for (int entityId : editorState.selection.entities) {
        if (editorState.renameState.entityId != entityId) {
            std::string name = world.HasComponent<NameComponent>(entityId)
                                   ? world.GetComponent<NameComponent>(entityId).name
                                   : "Entity_" + std::to_string(entityId);
            ImGui::Text(name.c_str());
            if (ImGui::IsItemClicked()) {
                editorState.renameState.Begin(world, entityId);
            }
        }

        if (editorState.renameState.entityId == entityId) {
            ImGui::SetKeyboardFocusHere();
            if (ImGui::InputText("##label", &editorState.renameState.nameBuffer,
                                 ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsNoBlank)) {
                editorState.renameState.Finish(world);
            }
        }

        ImGui::Separator();
        ImGui::Text("Components:");

        if (ImGui::BeginPopupContextWindow("Right-Click Inspector", ImGuiPopupFlags_MouseButtonRight)) {
            if (ImGui::BeginMenu("Add..")) {
                if (ImGui::BeginMenu("Geometry")) {
                    if (ImGui::MenuItem("ModelComponent")) {
                        if (!world.HasComponent<ModelComponent>(entityId)) {
                            ModelID id =
                                Application::s_Application->m_AssetManager->GetBuiltinModel(BuiltinModel::Cube);
                            world.AddComponent<ModelComponent>(entityId, ModelComponent(id, Math::Transformation()));
                        }
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Script")) {
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Audio")) {
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Other")) {
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            ImGui::EndPopup();
        }

        // ModelComponent
        if (world.HasComponent<ModelComponent>(entityId)) {
            if (ImGui::TreeNodeEx("ModelComponent", ImGuiTreeNodeFlags_DefaultOpen)) {
                ModelComponent& model = world.GetComponent<ModelComponent>(entityId);

                ImGui::Indent();
                if (ImGui::Button("Browse")) {
                    std::filesystem::path path = FileDialog::OpenFile("Open file", "Model\0*.fbx;*.obj\0");
                    if (!path.empty()) {
                        // Replace the component with new model
                        ModelID id = Application::s_Application->m_AssetManager->LoadModel(path.string());
                        world.GetComponent<ModelComponent>(entityId) = ModelComponent(id, model.m_Transformation);
                    }
                }
                ImGui::SameLine();

                // Check if it's a built-in model first to avoid error logs
                if (Application::s_Application->m_AssetManager->HasPath(model.m_ModelID)) {
                    std::string modelPath = Application::s_Application->m_AssetManager->GetModelPath(model.m_ModelID);
                    if (modelPath.empty()) {
                        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Path not found");
                    } else {
                        ImGui::Text("%s", modelPath.c_str());
                    }
                } else {
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Built-in model");
                }

                TransformEditState& tState = state.transformState;
                if (state.dirty) {
                    tState.translate = model.m_Transformation.m_Position;
                    tState.scale = model.m_Transformation.m_Scale;
                    tState.rotate = Math::Vec3(Math::ToDegrees(model.m_Transformation.m_Rotation.x),
                                               Math::ToDegrees(model.m_Transformation.m_Rotation.y),
                                               Math::ToDegrees(model.m_Transformation.m_Rotation.z));
                    state.dirty = false;
                }
                if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Text("Translate:");
                    if (ImGui::DragFloat3("##translate", &tState.translate.x, 0.05f)) {
                        model.m_Transformation.Move(tState.translate);
                    }

                    ImGui::Text("Scale:");
                    if (ImGui::DragFloat3("##scale", &tState.scale.x, 0.0001f, -100000.0f, 100000.0f, "%.4f", 1.001f)) {
                        if (tState.syncScale) {
                            Math::Vec3 diff = tState.scale - model.m_Transformation.m_Scale;
                            model.m_Transformation.m_Scale += diff.x + diff.y + diff.z;
                            tState.scale = model.m_Transformation.m_Scale;
                            model.m_Transformation.Calculate(); // Recompute model matrix after manual change
                        } else {
                            model.m_Transformation.Scale(tState.scale);
                        }
                    }
                    ImGui::Checkbox("Sync", &tState.syncScale);

                    ImGui::Text("Rotate:");
                    if (ImGui::DragFloat3("##rotate", &tState.rotate.x, 0.5f)) {
                        tState.rotate =
                            Math::Vec3(std::fmod(tState.rotate.x, 360.0f), std::fmod(tState.rotate.y, 360.0f),
                                       std::fmod(tState.rotate.z, 360.0f));
                        model.m_Transformation.Rotate(tState.rotate);
                    }

                    ImGui::TreePop();
                }
                ImGui::Unindent();
                ImGui::TreePop();
            }
        }

        // Show message if no components
        if (!world.HasAnyComponent(entityId)) {
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.34f, 129.0f, 0, 255), "Right click here!");
        }

        break;
    }
    ImGui::End();
}