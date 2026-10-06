#include "EditorTheme.h"

#include <Dodo.h>

using namespace Dodo;

void EditorTheme::Apply()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ApplyColors(style);
    ApplyMetrics(style);
    LoadFont();
}

void EditorTheme::ApplyColors(ImGuiStyle& style)
{
    ImGui::StyleColorsDark(&style);
    ImVec4* colors = style.Colors;

    const ImVec4 transparent = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    colors[ImGuiCol_Text] = Text;
    colors[ImGuiCol_TextDisabled] = TextDim;
    colors[ImGuiCol_TextSelectedBg] = Selection;
    colors[ImGuiCol_InputTextCursor] = Text;

    colors[ImGuiCol_WindowBg] = Panel;
    colors[ImGuiCol_ChildBg] = transparent;
    colors[ImGuiCol_PopupBg] = Raised;
    colors[ImGuiCol_Border] = Border;
    colors[ImGuiCol_BorderShadow] = transparent;
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.55f);

    colors[ImGuiCol_FrameBg] = Field;
    colors[ImGuiCol_FrameBgHovered] = Raised;
    colors[ImGuiCol_FrameBgActive] = Field;

    colors[ImGuiCol_TitleBg] = Background;
    colors[ImGuiCol_TitleBgActive] = Background;
    colors[ImGuiCol_TitleBgCollapsed] = Background;
    colors[ImGuiCol_MenuBarBg] = Background;

    colors[ImGuiCol_ScrollbarBg] = transparent;
    colors[ImGuiCol_ScrollbarGrab] = RaisedHover;
    colors[ImGuiCol_ScrollbarGrabHovered] = RaisedActive;
    colors[ImGuiCol_ScrollbarGrabActive] = TextDim;

    colors[ImGuiCol_CheckMark] = Accent;
    colors[ImGuiCol_SliderGrab] = Accent;
    colors[ImGuiCol_SliderGrabActive] = AccentHover;

    colors[ImGuiCol_Button] = Raised;
    colors[ImGuiCol_ButtonHovered] = RaisedHover;
    colors[ImGuiCol_ButtonActive] = RaisedActive;

    // Selectables, tree nodes, collapsing headers and menu items
    colors[ImGuiCol_Header] = Selection;
    colors[ImGuiCol_HeaderHovered] = Hover;
    colors[ImGuiCol_HeaderActive] = Selection;

    colors[ImGuiCol_Separator] = Border;
    colors[ImGuiCol_SeparatorHovered] = Accent;
    colors[ImGuiCol_SeparatorActive] = AccentHover;
    colors[ImGuiCol_ResizeGrip] = transparent;
    colors[ImGuiCol_ResizeGripHovered] = Accent;
    colors[ImGuiCol_ResizeGripActive] = AccentHover;

    // The selected tab takes the panel color, so a panel and its tab read as one surface
    colors[ImGuiCol_Tab] = Background;
    colors[ImGuiCol_TabHovered] = Raised;
    colors[ImGuiCol_TabSelected] = Panel;
    colors[ImGuiCol_TabSelectedOverline] = Accent;
    colors[ImGuiCol_TabDimmed] = Background;
    colors[ImGuiCol_TabDimmedSelected] = Panel;
    colors[ImGuiCol_TabDimmedSelectedOverline] = transparent;

    colors[ImGuiCol_DockingEmptyBg] = Background;
    colors[ImGuiCol_DockingPreview] = Selection;

    colors[ImGuiCol_TableHeaderBg] = Raised;
    colors[ImGuiCol_TableBorderStrong] = Border;
    colors[ImGuiCol_TableBorderLight] = Border;
    colors[ImGuiCol_TableRowBg] = transparent;
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.02f);

    colors[ImGuiCol_DragDropTarget] = Accent;
    colors[ImGuiCol_NavCursor] = Accent;
    colors[ImGuiCol_TextLink] = AccentHover;
}

void EditorTheme::ApplyMetrics(ImGuiStyle& style)
{
    style.WindowPadding = ImVec2(10.0f, 10.0f);
    style.FramePadding = ImVec2(8.0f, 5.0f);
    style.CellPadding = ImVec2(6.0f, 4.0f);
    style.ItemSpacing = ImVec2(8.0f, 6.0f);
    style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
    style.IndentSpacing = 16.0f;
    style.ScrollbarSize = 12.0f;
    style.GrabMinSize = 10.0f;

    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.TabBorderSize = 0.0f;
    style.TabBarBorderSize = 1.0f;
    style.TabBarOverlineSize = 2.0f;
    style.DockingSeparatorSize = 2.0f;
    style.SeparatorTextBorderSize = 1.0f;

    style.WindowRounding = 6.0f;
    style.ChildRounding = Rounding;
    style.PopupRounding = 6.0f;
    style.FrameRounding = Rounding;
    style.GrabRounding = Rounding;
    style.TabRounding = Rounding;
    style.ScrollbarRounding = 6.0f;

    style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
    style.WindowMenuButtonPosition = ImGuiDir_None;
    style.ColorButtonPosition = ImGuiDir_Left;
    style.DisabledAlpha = 0.45f;
}

void EditorTheme::LoadFont()
{
    const char* path = "res/font/opensans/opensans.ttf";
    if (FileUtils::FileExists(path)) {
        ImGui::GetIO().Fonts->AddFontFromFileTTF(path, FontSize);
    } else {
        DD_WARN("Could not find: {}, using default font.", path);
    }
}
