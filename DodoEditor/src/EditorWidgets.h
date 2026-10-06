#pragma once

#include <Dodo.h>

#include <imgui.h>
#include <string>

/**
 * Small building blocks shared by the editor panels, styled after EditorTheme.
 */
class EditorWidgets {
  public:
    /**
     * Square toolbar button that shows an icon.
     *
     * @param label Identifies the button, and is shown instead of the icon when the icon is missing.
     * @param icon ImGui texture from EditorIcons, may be null.
     * @param tooltip Shown while hovering, may be null.
     * @param selected Draws the button in the accent color, for the active choice of a tool group.
     * @return True when the button was clicked.
     */
    static bool ToolButton(const char* label, void* icon, const char* tooltip, bool selected = false);

    /**
     * Thin vertical divider between groups of toolbar buttons. Stays on the current line.
     */
    static void ToolSeparator();

    /**
     * Text field with a leading search icon.
     *
     * @return True when the text was edited.
     */
    static bool SearchField(const char* id, std::string& text, void* icon, float width);

    /**
     * Case insensitive test of a name against the text of a search field. An empty filter matches everything.
     */
    static bool MatchesFilter(const std::string& name, const std::string& filter);

    /**
     * Three drag fields marked with the axis colors, filling the available width.
     *
     * @return True when any of the components was edited.
     */
    static bool Vec3Control(const char* id, Dodo::Math::Vec3& value, float speed, const char* format);

    /**
     * Collapsible title bar of a group of properties, such as a component. Open by default.
     *
     * @return True while the section is open and its content should be drawn.
     */
    static bool SectionHeader(const char* label);

    /**
     * Starts a two column layout with the property names on the left and their widgets on the right.
     * Only call EndProperties when this returns true.
     */
    static bool BeginProperties(const char* id);

    /**
     * Starts a new row and leaves the cursor in the widget column, where the next item takes the full width.
     */
    static void Property(const char* label);

    static void EndProperties();

    /**
     * Dimmed, centered message for a panel that has nothing to show.
     */
    static void EmptyState(const char* text);

    /**
     * Draws a single line of text centered in the given width, cut off with an ellipsis when it does not fit.
     */
    static void TextEllipsis(ImDrawList* drawList, const ImVec2& pos, float width, ImU32 color,
                             const std::string& text);
};
