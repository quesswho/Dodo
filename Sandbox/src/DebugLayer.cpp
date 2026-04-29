#include "DebugLayer.h"

#include <imgui.h>

void DebugLayer::Render(RenderAPI& renderAPI, AssetManager& assets)
{
    Application::s_Application->ImGuiNewFrame();

    const GpuTimings& t = renderAPI.m_GpuTimings;
    float frame = t.frameMs > 0.0f ? t.frameMs : 1.0f;

    ImGui::SetNextWindowSize(ImVec2(340, 140), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::Begin("GPU Timings");

    ImGui::Text("Frame total  %6.3f ms", t.frameMs);
    ImGui::Separator();

    auto row = [&](const char* label, float ms) {
        ImGui::Text("%-12s %6.3f ms", label, ms);
        ImGui::SameLine();
        ImGui::ProgressBar(ms / frame, ImVec2(-1.0f, 0.0f));
    };

    row("Shadow", t.shadowMs);
    row("Scene", t.sceneMs);
    row("Post-Effect", t.postEffectMs);

    ImGui::End();

    Application::s_Application->ImGuiEndFrame();
}
