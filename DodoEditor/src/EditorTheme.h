#pragma once

#include <imgui.h>

/**
 * Look of the editor: one palette, one set of metrics and the interface font.
 *
 * Panels take their colors from here instead of hardcoding them, so the whole editor stays consistent.
 */
class EditorTheme {
  public:
    // Surfaces, from the darkest (behind the panels) to the lightest (raised above a panel)
    static constexpr ImVec4 Background = ImVec4(0.071f, 0.075f, 0.086f, 1.0f);
    static constexpr ImVec4 Panel = ImVec4(0.114f, 0.118f, 0.133f, 1.0f);
    static constexpr ImVec4 Raised = ImVec4(0.153f, 0.157f, 0.176f, 1.0f);
    static constexpr ImVec4 RaisedHover = ImVec4(0.196f, 0.204f, 0.227f, 1.0f);
    static constexpr ImVec4 RaisedActive = ImVec4(0.235f, 0.243f, 0.271f, 1.0f);
    static constexpr ImVec4 Field = ImVec4(0.067f, 0.071f, 0.082f, 1.0f);
    static constexpr ImVec4 Border = ImVec4(0.196f, 0.204f, 0.231f, 1.0f);

    static constexpr ImVec4 Text = ImVec4(0.863f, 0.871f, 0.894f, 1.0f);
    static constexpr ImVec4 TextDim = ImVec4(0.541f, 0.557f, 0.604f, 1.0f);
    static constexpr ImVec4 TextError = ImVec4(0.949f, 0.380f, 0.365f, 1.0f);

    static constexpr ImVec4 Accent = ImVec4(0.231f, 0.557f, 0.918f, 1.0f);
    static constexpr ImVec4 AccentHover = ImVec4(0.333f, 0.624f, 0.941f, 1.0f);
    static constexpr ImVec4 AccentActive = ImVec4(0.184f, 0.471f, 0.800f, 1.0f);
    static constexpr ImVec4 Selection = ImVec4(0.231f, 0.557f, 0.918f, 0.30f);
    static constexpr ImVec4 Hover = ImVec4(1.0f, 1.0f, 1.0f, 0.055f);

    // Axis colors: X is red, Y is green and Z is blue, as on the transform gizmo
    static constexpr ImVec4 AxisX = ImVec4(0.878f, 0.322f, 0.298f, 1.0f);
    static constexpr ImVec4 AxisY = ImVec4(0.435f, 0.749f, 0.267f, 1.0f);
    static constexpr ImVec4 AxisZ = ImVec4(0.247f, 0.549f, 0.941f, 1.0f);

    static constexpr float FontSize = 15.0f;
    static constexpr float FontSizeSmall = 13.0f;
    static constexpr float Rounding = 4.0f;

    /**
     * Applies the palette and the metrics to the ImGui style and loads the interface font.
     * Call once, after the ImGui context has been created.
     */
    static void Apply();

  private:
    static void ApplyColors(ImGuiStyle& style);
    static void ApplyMetrics(ImGuiStyle& style);
    static void LoadFont();
};
