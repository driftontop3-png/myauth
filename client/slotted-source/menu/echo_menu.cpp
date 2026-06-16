#include "echo_menu.h"
#include "echo_style.h"
#include "echo_icons.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../main/features.h"
#include "../esp.h"
#include <Windows.h>
#include <cfloat>

namespace
{
    static int currentTab = 0;
    static int previousTab = 0;
    static float contentAlpha = 1.0f;
    static float iconGlow[3] = {};

    static const char* tabIcons[] = { ICON_FA_BULLSEYE, ICON_FA_EYE, ICON_FA_COG };

    static bool RoundCheckbox(const char* label, bool* v, float scale)
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 cp = ImGui::GetCursorScreenPos();
        const float sz = 22.0f * scale;
        const float rnd = 6.0f * scale;

        ImGui::PushID(label);
        const bool clicked = ImGui::InvisibleButton("##rcb", ImVec2(sz, sz));
        ImGui::PopID();
        const bool hovered = ImGui::IsItemHovered();
        if (clicked)
            *v = !*v;

        const ImVec2 pMin = cp;
        const ImVec2 pMax(cp.x + sz, cp.y + sz);
        const float cx = cp.x + sz * 0.5f;
        const float cy = cp.y + sz * 0.5f;

        if (*v)
        {
            for (int g = 0; g < 4; ++g)
            {
                const float pad = (g + 1) * 2.0f * scale;
                dl->AddRectFilled(
                    ImVec2(pMin.x - pad, pMin.y - pad),
                    ImVec2(pMax.x + pad, pMax.y + pad),
                    EchoStyle::AccentAlpha(7 - g), rnd + pad * 0.5f);
            }
        }

        ImU32 bg = *v ? EchoStyle::ACCENT_U32 : EchoStyle::CHECKBOX_OFF_U32;
        if (hovered && !*v)
            bg = EchoStyle::CHECKBOX_OFF_HOVER_U32;
        dl->AddRectFilled(pMin, pMax, bg, rnd);
        dl->AddRect(pMin, pMax, *v ? EchoStyle::AccentAlpha(255) : EchoStyle::CHECKBOX_BORDER_U32, rnd, 0, 1.5f * scale);

        if (*v)
        {
            const float s = sz * 0.38f;
            dl->AddLine(ImVec2(cx - s * 0.55f, cy), ImVec2(cx - s * 0.08f, cy + s * 0.5f), EchoStyle::CHECKMARK_U32, 2.5f * scale);
            dl->AddLine(ImVec2(cx - s * 0.08f, cy + s * 0.5f), ImVec2(cx + s * 0.6f, cy - s * 0.5f), EchoStyle::CHECKMARK_U32, 2.5f * scale);
        }

        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 4.0f * scale);
        ImGui::TextUnformatted(label);
        return clicked;
    }

    static void DrawCustomSlider(const char* id, float* value, float minVal, float maxVal, const char* fmt, float scale)
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const float sliderWidth = 200.0f * scale;
        const float sliderHeight = 6.0f * scale;
        const float grabSize = 16.0f * scale;
        const ImVec2 sliderPos = ImGui::GetCursorScreenPos();
        const float trackOffsetY = (grabSize - sliderHeight) * 0.5f;

        dl->AddRectFilled(
            ImVec2(sliderPos.x, sliderPos.y + trackOffsetY),
            ImVec2(sliderPos.x + sliderWidth, sliderPos.y + trackOffsetY + sliderHeight),
            EchoStyle::TRACK_BG_U32, sliderHeight * 0.5f);

        const float progress = (*value - minVal) / (maxVal - minVal);
        const float fillWidth = sliderWidth * progress;
        dl->AddRectFilled(
            ImVec2(sliderPos.x, sliderPos.y + trackOffsetY),
            ImVec2(sliderPos.x + fillWidth, sliderPos.y + trackOffsetY + sliderHeight),
            EchoStyle::ACCENT_U32, sliderHeight * 0.5f);

        ImGui::PushItemWidth(sliderWidth);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, grabSize);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        ImGui::SliderFloat(id, value, minVal, maxVal, "");
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(5);
        ImGui::PopItemWidth();

        const float grabX = sliderPos.x + fillWidth;
        const float grabY = sliderPos.y + grabSize * 0.5f;
        dl->AddCircleFilled(ImVec2(grabX, grabY), grabSize * 0.5f - 1.0f, EchoStyle::SLIDER_GRAB_U32, 32);
        ImGui::SameLine();
        ImGui::Text(fmt, *value);
    }

    static void DrawSidebar(ImDrawList* draw, ImVec2 pos, ImVec2 size, float scale)
    {
        ImGuiIO& io = ImGui::GetIO();
        const float sidebarWidth = EchoStyle::SIDEBAR_WIDTH * scale;
        const float iconSize = EchoStyle::ICON_SIZE * scale;
        const float totalTabsHeight = 3.0f * iconSize + 2.0f * (EchoStyle::ICON_SPACING * scale);
        float iconY = (size.y - totalTabsHeight) * 0.5f;

        const float lineX = pos.x + sidebarWidth - 2.5f * scale;
        const float linePad = 16.0f * scale;
        draw->AddRectFilled(
            ImVec2(lineX, pos.y + iconY - linePad),
            ImVec2(lineX + 2.5f * scale, pos.y + iconY + totalTabsHeight + linePad),
            EchoStyle::ACCENT_U32, 1.5f);

        for (int i = 0; i < 3; ++i)
        {
            const float iconX = (sidebarWidth - iconSize) * 0.5f;
            const ImVec2 btnMin(pos.x + iconX, pos.y + iconY);
            const ImVec2 btnMax(btnMin.x + iconSize, btnMin.y + iconSize);
            const bool selected = (currentTab == i);
            const bool hovered = ImGui::IsMouseHoveringRect(btnMin, btnMax);

            const float targetGlow = selected ? 1.0f : (hovered ? 0.5f : 0.0f);
            iconGlow[i] += (targetGlow - iconGlow[i]) * io.DeltaTime * 8.0f;

            if (iconGlow[i] > 0.01f)
            {
                const ImVec2 center(btnMin.x + iconSize * 0.5f, btnMin.y + iconSize * 0.5f);
                for (int g = 0; g < 6; ++g)
                {
                    const float glowSize = iconSize * 0.5f + g * 3.0f * scale;
                    const int alpha = (int)((4.0f - g * 0.5f) * iconGlow[i]);
                    if (alpha > 0)
                        draw->AddCircleFilled(center, glowSize, EchoStyle::AccentAlpha(alpha), 32);
                }
            }

            ImGui::SetCursorPos(ImVec2(iconX, iconY));
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

            const float colorLerp = iconGlow[i];
            ImVec4 iconColor(
                EchoStyle::TEXT_DIM.x + (EchoStyle::ACCENT.x - EchoStyle::TEXT_DIM.x) * colorLerp,
                EchoStyle::TEXT_DIM.y + (EchoStyle::ACCENT.y - EchoStyle::TEXT_DIM.y) * colorLerp,
                EchoStyle::TEXT_DIM.z + (EchoStyle::ACCENT.z - EchoStyle::TEXT_DIM.z) * colorLerp,
                1.0f);
            ImGui::PushStyleColor(ImGuiCol_Text, iconColor);

            ImGui::PushID(i);
            if (ImGui::Button(tabIcons[i], ImVec2(iconSize, iconSize)))
                currentTab = i;
            ImGui::PopID();
            ImGui::PopStyleColor(4);

            iconY += iconSize + EchoStyle::ICON_SPACING * scale;
        }
    }

    static void DrawHeader(ImDrawList* draw, ImVec2 pos, ImVec2 size, float scale)
    {
        (void)size;
        ImFont* font = ImGui::GetFont();
        const float fontSize = ImGui::GetFontSize();

        const char* titlePurple = "Astryn Cheats ";
        const char* titleWhite = "Private";
        const ImVec2 purpleSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, titlePurple);
        const ImVec2 whiteSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, titleWhite);
        const float titleW = purpleSize.x + whiteSize.x;
        const float titleX = pos.x + ImGui::GetWindowWidth() - titleW - 16.0f * scale;
        const float titleY = pos.y + 19.0f * scale;

        draw->AddText(font, fontSize, ImVec2(titleX, titleY), EchoStyle::ACCENT_U32, titlePurple);
        draw->AddText(font, fontSize, ImVec2(titleX + purpleSize.x, titleY), ImGui::ColorConvertFloat4ToU32(EchoStyle::TEXT_BRIGHT), titleWhite);
    }

    static void PanelHeader(const char* title, float scale)
    {
        ImGui::Dummy(ImVec2(0, 5.0f * scale));
        ImGui::SetCursorPosX(15.0f * scale);
        ImGui::PushStyleColor(ImGuiCol_Text, EchoStyle::ACCENT);
        ImGui::TextUnformatted(title);
        ImGui::PopStyleColor();
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 5.0f * scale));
    }

    struct AimKeyEntry
    {
        const char* label;
        int vk;
    };

    static const AimKeyEntry kAimKeys[] =
    {
        { "Right Mouse", VK_RBUTTON },
        { "Left Mouse", VK_LBUTTON },
        { "Middle Mouse", VK_MBUTTON },
        { "Mouse 4", VK_XBUTTON1 },
        { "Mouse 5", VK_XBUTTON2 },
        { "Shift", VK_SHIFT },
        { "Control", VK_CONTROL },
        { "Alt", VK_MENU },
        { "Caps Lock", VK_CAPITAL },
        { "Space", VK_SPACE },
        { "E", 'E' },
        { "Q", 'Q' },
        { "F", 'F' },
        { "X", 'X' },
        { "C", 'C' },
        { "V", 'V' },
        { "F1", VK_F1 },
        { "F2", VK_F2 },
        { "F3", VK_F3 },
        { "F4", VK_F4 },
        { "F5", VK_F5 },
        { "F6", VK_F6 },
        { "F7", VK_F7 },
        { "F8", VK_F8 },
        { "F9", VK_F9 },
        { "F10", VK_F10 },
        { "F11", VK_F11 },
        { "F12", VK_F12 },
    };

    static const char* kThemeNames[] =
    {
        "Red",
        "Purple",
        "Blue",
        "Green",
        "Cyan",
        "Orange",
        "White",
    };

    static ImRect GetMenuBoundsRect()
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        while (window && window->ParentWindow)
            window = window->ParentWindow;

        if (!window)
            return ImGui::GetCurrentWindow()->ClipRect;

        return ImRect(window->Pos, ImVec2(window->Pos.x + window->Size.x, window->Pos.y + window->Size.y));
    }

    static float CalcComboPopupHeight(int visible_items)
    {
        const ImGuiStyle& style = ImGui::GetStyle();
        const float lineH = ImGui::GetTextLineHeightWithSpacing();
        if (visible_items <= 0)
            return lineH * 6.f + style.WindowPadding.y * 2.f;
        return lineH * visible_items + style.WindowPadding.y * 2.f;
    }

    static void ClampPopupToBounds(ImGuiWindow* popup, const ImRect& bounds)
    {
        if (!popup)
            return;

        ImVec2 pos = popup->Pos;
        const ImVec2 size = popup->SizeFull;

        if (pos.x + size.x > bounds.Max.x)
            pos.x = bounds.Max.x - size.x;
        if (pos.x < bounds.Min.x)
            pos.x = bounds.Min.x;
        if (pos.y + size.y > bounds.Max.y)
            pos.y = bounds.Max.y - size.y;
        if (pos.y < bounds.Min.y)
            pos.y = bounds.Min.y;

        popup->Pos = pos;
    }

    static bool DrawBoundedCombo(const char* id, int* current_item,
        bool(*getter)(void*, int, const char**), void* data, int item_count, int preferred_visible = 6)
    {
        const char* preview = "";
        getter(data, *current_item, &preview);

        const ImVec2 pos = ImGui::GetCursorScreenPos();
        const float frameH = ImGui::GetFrameHeight();
        const float lineH = ImGui::GetTextLineHeightWithSpacing();
        const ImGuiStyle& style = ImGui::GetStyle();
        const ImRect bounds = GetMenuBoundsRect();
        const float spaceBelow = bounds.Max.y - (pos.y + frameH);
        const float spaceAbove = pos.y - bounds.Min.y;
        const float popupPad = style.WindowPadding.y * 2.f;

        int maxVisible = preferred_visible;
        float popupH = CalcComboPopupHeight(maxVisible);
        bool openUpward = false;

        if (spaceBelow < popupH)
        {
            if (spaceAbove >= popupH)
                openUpward = true;
            else
            {
                const float bestSpace = ImMax(spaceBelow, spaceAbove);
                maxVisible = (int)((bestSpace - popupPad) / lineH);
                if (maxVisible < 2)
                    maxVisible = 2;
                popupH = CalcComboPopupHeight(maxVisible);
                openUpward = spaceAbove >= spaceBelow;
            }
        }

        ImGui::SetNextWindowSizeConstraints(
            ImVec2(0, 0),
            ImVec2(FLT_MAX, CalcComboPopupHeight(maxVisible)));

        if (openUpward)
            ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y - popupH - style.ItemSpacing.y), ImGuiCond_Always);

        if (!ImGui::BeginCombo(id, preview, ImGuiComboFlags_None))
            return false;

        bool value_changed = false;
        for (int i = 0; i < item_count; i++)
        {
            ImGui::PushID(i);
            const bool item_selected = (i == *current_item);
            const char* item_text;
            if (!getter(data, i, &item_text))
                item_text = "*Unknown item*";
            if (ImGui::Selectable(item_text, item_selected))
            {
                value_changed = true;
                *current_item = i;
            }
            if (item_selected)
                ImGui::SetItemDefaultFocus();
            ImGui::PopID();
        }

        ImGuiWindow* popup = ImGui::GetCurrentWindow();
        if (openUpward)
        {
            popup->AutoPosLastDirection = ImGuiDir_Up;
            popup->Pos = ImVec2(pos.x, pos.y - popup->SizeFull.y - style.ItemSpacing.y);
        }
        ClampPopupToBounds(popup, bounds);

        ImGui::EndCombo();
        return value_changed;
    }

    static bool AimKeyGetter(void* data, int idx, const char** out_text)
    {
        const AimKeyEntry* keys = static_cast<const AimKeyEntry*>(data);
        if (idx < 0 || idx >= IM_ARRAYSIZE(kAimKeys))
            return false;
        if (out_text)
            *out_text = keys[idx].label;
        return true;
    }

    static int FindKeyIndex(int vk)
    {
        for (int i = 0; i < IM_ARRAYSIZE(kAimKeys); ++i)
        {
            if (kAimKeys[i].vk == vk)
                return i;
        }
        return 0;
    }

    static void DrawKeyCombo(const char* label, const char* id, int* vk, float scale)
    {
        int keyIndex = FindKeyIndex(*vk);

        ImGui::Dummy(ImVec2(0, 5.0f * scale));
        ImGui::SetCursorPosX(15.0f * scale);
        ImGui::TextUnformatted(label);
        ImGui::SetCursorPosX(15.0f * scale);
        ImGui::PushItemWidth(200.0f * scale);
        if (DrawBoundedCombo(id, &keyIndex, AimKeyGetter, (void*)kAimKeys, IM_ARRAYSIZE(kAimKeys)))
            *vk = kAimKeys[keyIndex].vk;
        ImGui::PopItemWidth();
    }

    static void DrawAimKeyCombo(float scale)
    {
        DrawKeyCombo("Aim Key", "##aimkey", &aimbot::current_key, scale);
    }

    static void DrawThemeCombo(float scale)
    {
        ImGui::Dummy(ImVec2(0, 8.0f * scale));
        ImGui::SetCursorPosX(15.0f * scale);
        ImGui::TextUnformatted("Theme Color");
        ImGui::SetCursorPosX(15.0f * scale);
        ImGui::PushItemWidth(200.0f * scale);
        if (DrawBoundedCombo("##theme", &settings::theme_preset, [](void*, int idx, const char** out_text) -> bool {
            if (idx < 0 || idx >= IM_ARRAYSIZE(kThemeNames))
                return false;
            if (out_text)
                *out_text = kThemeNames[idx];
            return true;
        }, nullptr, IM_ARRAYSIZE(kThemeNames)))
            ApplyThemePreset(settings::theme_preset);
        ImGui::PopItemWidth();
    }

    static void DrawTabAimbot(float scale)
    {
        ImGui::Columns(2, "##AimbotCols", false);
        ImGui::SetColumnWidth(0, EchoStyle::COLUMN_WIDTH * scale);

        ImGui::BeginChild("##AimLeft", ImVec2(EchoStyle::PANEL_WIDTH * scale, EchoStyle::PANEL_HEIGHT * scale), false);
        PanelHeader("General", scale);
        ImGui::SetCursorPosX(15.0f * scale);
        RoundCheckbox("Memory Aimbot", &aimbot::memoryaim, scale);
        ImGui::SetCursorPosX(15.0f * scale);
        RoundCheckbox("Triggerbot", &triggerbot::enabled, scale);
        ImGui::SetCursorPosX(15.0f * scale);
        RoundCheckbox("Target Line", &features::targetline, scale);
        ImGui::SetCursorPosX(15.0f * scale);
        RoundCheckbox("Visible Check", &features::visiblecheck, scale);
        ImGui::SetCursorPosX(15.0f * scale);
        RoundCheckbox("Show FOV", &aimbot::show_fov, scale);
        ImGui::SetCursorPosX(15.0f * scale);
        RoundCheckbox("Fill FOV Circle", &aimbot::fill_fov, scale);
        ImGui::SetCursorPosX(15.0f * scale);
        RoundCheckbox("Controller Support", &aimbot::controller_support, scale);
        ImGui::EndChild();

        ImGui::NextColumn();
        ImGui::BeginChild("##AimRight", ImVec2(EchoStyle::PANEL_WIDTH * scale, EchoStyle::PANEL_HEIGHT * scale), false);
        PanelHeader("Aim Settings", scale);
        ImGui::SetCursorPosX(15.0f * scale);
        ImGui::TextUnformatted("Smoothness");
        ImGui::SetCursorPosX(15.0f * scale);
        DrawCustomSlider("##smooth", &aimbot::smooth, 1.0f, 25.0f, "%.1f", scale);
        ImGui::Dummy(ImVec2(0, 5.0f * scale));
        ImGui::SetCursorPosX(15.0f * scale);
        ImGui::TextUnformatted("FOV");
        ImGui::SetCursorPosX(15.0f * scale);
        DrawCustomSlider("##fov", &aimbot::fov, 10.0f, 500.0f, "%.0f", scale);
        DrawAimKeyCombo(scale);
        ImGui::EndChild();
        ImGui::Columns(1);
    }

    static void DrawTabVisuals(float scale)
    {
        ImGui::Columns(2, "##VisCols", false);
        ImGui::SetColumnWidth(0, EchoStyle::COLUMN_WIDTH * scale);

        ImGui::BeginChild("##VisLeft", ImVec2(EchoStyle::PANEL_WIDTH * scale, EchoStyle::PANEL_HEIGHT * scale), false);
        PanelHeader("ESP", scale);
        ImGui::SetCursorPosX(15.0f * scale);
        RoundCheckbox("Cornered Box", &visuals::CorneredBox, scale);
        ImGui::SetCursorPosX(15.0f * scale);
        RoundCheckbox("Box", &visuals::box, scale);
        ImGui::SetCursorPosX(15.0f * scale);
        RoundCheckbox("Skeleton", &visuals::skeleton, scale);
        ImGui::SetCursorPosX(15.0f * scale);
        RoundCheckbox("Head Circle", &visuals::headc, scale);
        ImGui::SetCursorPosX(15.0f * scale);
        RoundCheckbox("Distance ESP", &visuals::distance, scale);
        ImGui::EndChild();

        ImGui::NextColumn();
        ImGui::BeginChild("##VisRight", ImVec2(EchoStyle::PANEL_WIDTH * scale, EchoStyle::PANEL_HEIGHT * scale), false);
        PanelHeader("Overlay", scale);
        ImGui::SetCursorPosX(15.0f * scale);
        RoundCheckbox("Line", &visuals::line, scale);
        ImGui::SetCursorPosX(15.0f * scale);
        RoundCheckbox("Radar", &visuals::radar, scale);
        ImGui::EndChild();
        ImGui::Columns(1);
    }

    static void DrawTabSettings(float scale)
    {
        ImGui::BeginChild("##Settings", ImVec2(EchoStyle::CONTENT_WIDTH * scale, EchoStyle::PANEL_HEIGHT * scale), false);
        PanelHeader("Settings", scale);
        ImGui::SetCursorPosX(15.0f * scale);
        ImGui::TextUnformatted("Distance");
        ImGui::SetCursorPosX(15.0f * scale);
        DrawCustomSlider("##distance", &settings::render_distance, 50.0f, settings::unlimited_distance, "%.0f", scale);
        if (settings::render_distance >= settings::unlimited_distance - 1.0f)
        {
            ImGui::SetCursorPosX(15.0f * scale);
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + EchoStyle::PANEL_WIDTH * scale - 30.0f * scale);
            ImGui::TextDisabled("Max distance = map-wide ESP / aim");
            ImGui::PopTextWrapPos();
        }
        DrawThemeCombo(scale);
        ImGui::SetCursorPosX(15.0f * scale);
        ImGui::Dummy(ImVec2(0, 6.0f * scale));
        ImGui::TextDisabled("Menu: F2 | Aim: selected key / LT");
        ImGui::EndChild();
    }
}

void EchoMenu::Render(float main_scale)
{
    if (main_scale <= 0.0f)
        main_scale = 1.0f;

    ImGuiIO& io = ImGui::GetIO();
    EchoStyle::ApplyStyle();
    EchoStyle::PushColors();

    const float menuWidth = EchoStyle::MENU_WIDTH * main_scale;
    const float menuHeight = EchoStyle::MENU_HEIGHT * main_scale;
    const float sidebarWidth = EchoStyle::SIDEBAR_WIDTH * main_scale;
    const float headerHeight = EchoStyle::HEADER_HEIGHT * main_scale;

    ImGui::SetNextWindowSize(ImVec2(menuWidth, menuHeight), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);

    ImGui::Begin("##EasyLolPublic", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);

    ImDrawList* draw = ImGui::GetWindowDrawList();
    const ImVec2 pos = ImGui::GetWindowPos();
    const ImVec2 size = ImGui::GetWindowSize();

    draw->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), EchoStyle::BG_WINDOW_U32, EchoStyle::WINDOW_ROUNDING);

    if (currentTab != previousTab)
    {
        contentAlpha = 0.0f;
        previousTab = currentTab;
    }
    contentAlpha += io.DeltaTime * 6.0f;
    if (contentAlpha > 1.0f)
        contentAlpha = 1.0f;

    DrawSidebar(draw, pos, size, main_scale);
    DrawHeader(draw, pos, size, main_scale);

    const float contentBoxWidth = EchoStyle::CONTENT_WIDTH * main_scale;
    const float contentBoxHeight = EchoStyle::CONTENT_HEIGHT * main_scale;
    const float availW = size.x - sidebarWidth;
    const float availH = size.y - headerHeight;
    const float contentX = sidebarWidth + (availW - contentBoxWidth) * 0.5f;
    const float contentY = headerHeight + (availH - contentBoxHeight) * 0.5f;

    ImGui::SetCursorPos(ImVec2(contentX, contentY));
    ImGui::BeginChild("##Content", ImVec2(contentBoxWidth, contentBoxHeight), false);

    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, contentAlpha);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(18.0f * main_scale, 12.0f * main_scale));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(12.0f * main_scale, 10.0f * main_scale));

    switch (currentTab)
    {
    case 0: DrawTabAimbot(main_scale); break;
    case 1: DrawTabVisuals(main_scale); break;
    case 2: DrawTabSettings(main_scale); break;
    }

    ImGui::PopStyleVar(3);
    ImGui::EndChild();

    ImGui::GetForegroundDrawList()->AddRect(
        pos, ImVec2(pos.x + size.x, pos.y + size.y),
        EchoStyle::BORDER_U32, EchoStyle::WINDOW_ROUNDING, 0, 1.5f);

    ImGui::End();
    EchoStyle::PopColors();
}
