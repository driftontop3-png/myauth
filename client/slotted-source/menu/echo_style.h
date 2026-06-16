#pragma once
#include "../imgui/imgui.h"

namespace EchoStyle
{
    constexpr float MENU_WIDTH = 650.0f;
    constexpr float MENU_HEIGHT = 450.0f;
    constexpr float SIDEBAR_WIDTH = 45.0f;
    constexpr float HEADER_HEIGHT = 55.0f;
    constexpr float CONTENT_WIDTH = 548.0f;
    constexpr float CONTENT_HEIGHT = 370.0f;
    constexpr float PANEL_WIDTH = 258.0f;
    constexpr float PANEL_HEIGHT = 340.0f;
    constexpr float COLUMN_WIDTH = 270.0f;
    constexpr float ICON_SIZE = 32.0f;
    constexpr float ICON_SPACING = 16.0f;
    constexpr float WINDOW_ROUNDING = 14.0f;

    inline bool LIGHT_MODE = false;

    inline ImVec4 BG_DARK = ImVec4(0.08f, 0.08f, 0.11f, 1.0f);
    inline ImVec4 BG_PANEL = ImVec4(0.10f, 0.10f, 0.14f, 1.0f);
    inline ImVec4 ACCENT = ImVec4(0.92f, 0.18f, 0.29f, 1.0f);
    inline ImVec4 ACCENT_HOVER = ImVec4(1.0f, 0.28f, 0.38f, 1.0f);
    inline ImVec4 TEXT_BRIGHT = ImVec4(0.95f, 0.95f, 0.97f, 1.0f);
    inline ImVec4 TEXT_DIM = ImVec4(0.55f, 0.55f, 0.60f, 1.0f);
    inline ImVec4 FRAME_BG = ImVec4(0.14f, 0.14f, 0.18f, 1.0f);
    inline ImVec4 FRAME_BG_HOVER = ImVec4(0.18f, 0.18f, 0.24f, 1.0f);
    inline ImVec4 FRAME_BG_ACTIVE = ImVec4(0.22f, 0.22f, 0.28f, 1.0f);
    inline ImVec4 BUTTON_BG = ImVec4(0.16f, 0.16f, 0.22f, 1.0f);
    inline ImVec4 BUTTON_BG_HOVER = ImVec4(0.22f, 0.22f, 0.30f, 1.0f);
    inline ImVec4 BUTTON_BG_ACTIVE = ImVec4(0.28f, 0.28f, 0.36f, 1.0f);
    inline ImVec4 HEADER_BG = ImVec4(0.16f, 0.16f, 0.22f, 1.0f);

    inline ImU32 ACCENT_U32 = IM_COL32(235, 46, 74, 255);
    inline ImU32 BG_WINDOW_U32 = IM_COL32(20, 20, 28, 255);
    inline ImU32 TRACK_BG_U32 = IM_COL32(30, 30, 40, 255);
    inline ImU32 CHECKBOX_OFF_U32 = IM_COL32(30, 30, 40, 255);
    inline ImU32 CHECKBOX_OFF_HOVER_U32 = IM_COL32(40, 40, 55, 255);
    inline ImU32 CHECKBOX_BORDER_U32 = IM_COL32(55, 55, 70, 255);
    inline ImU32 CHECKMARK_U32 = IM_COL32(255, 255, 255, 255);
    inline ImU32 SLIDER_GRAB_U32 = IM_COL32(255, 255, 255, 255);
    inline ImU32 BORDER_U32 = IM_COL32(75, 25, 35, 200);

    inline ImU32 WHITE_U32 = IM_COL32(255, 255, 255, 255);

    inline ImU32 AccentAlpha(int alpha)
    {
        return IM_COL32(
            (ACCENT_U32 >> IM_COL32_R_SHIFT) & 0xFF,
            (ACCENT_U32 >> IM_COL32_G_SHIFT) & 0xFF,
            (ACCENT_U32 >> IM_COL32_B_SHIFT) & 0xFF,
            alpha);
    }

    inline void SetAccentFromU32(ImU32 color)
    {
        ACCENT_U32 = color;
        const float r = ((color >> IM_COL32_R_SHIFT) & 0xFF) / 255.f;
        const float g = ((color >> IM_COL32_G_SHIFT) & 0xFF) / 255.f;
        const float b = ((color >> IM_COL32_B_SHIFT) & 0xFF) / 255.f;
        ACCENT = ImVec4(r, g, b, 1.0f);
        ACCENT_HOVER = ImVec4(
            (r + 0.08f > 1.f) ? 1.f : r + 0.08f,
            (g + 0.08f > 1.f) ? 1.f : g + 0.08f,
            (b + 0.08f > 1.f) ? 1.f : b + 0.08f,
            1.0f);
        BORDER_U32 = IM_COL32(
            (int)(((color >> IM_COL32_R_SHIFT) & 0xFF) * 0.35f),
            (int)(((color >> IM_COL32_G_SHIFT) & 0xFF) * 0.35f),
            (int)(((color >> IM_COL32_B_SHIFT) & 0xFF) * 0.35f),
            200);
    }

    inline void ApplyBasePalette(bool light)
    {
        LIGHT_MODE = light;
        if (light)
        {
            BG_DARK = ImVec4(0.96f, 0.96f, 0.98f, 1.0f);
            BG_PANEL = ImVec4(0.92f, 0.92f, 0.95f, 1.0f);
            TEXT_BRIGHT = ImVec4(0.12f, 0.12f, 0.15f, 1.0f);
            TEXT_DIM = ImVec4(0.45f, 0.45f, 0.52f, 1.0f);
            FRAME_BG = ImVec4(0.88f, 0.88f, 0.91f, 1.0f);
            FRAME_BG_HOVER = ImVec4(0.84f, 0.84f, 0.88f, 1.0f);
            FRAME_BG_ACTIVE = ImVec4(0.80f, 0.80f, 0.85f, 1.0f);
            BUTTON_BG = ImVec4(0.90f, 0.90f, 0.93f, 1.0f);
            BUTTON_BG_HOVER = ImVec4(0.86f, 0.86f, 0.90f, 1.0f);
            BUTTON_BG_ACTIVE = ImVec4(0.82f, 0.82f, 0.87f, 1.0f);
            HEADER_BG = ImVec4(0.90f, 0.90f, 0.93f, 1.0f);
            BG_WINDOW_U32 = IM_COL32(245, 245, 250, 255);
            TRACK_BG_U32 = IM_COL32(220, 220, 228, 255);
            CHECKBOX_OFF_U32 = IM_COL32(225, 225, 232, 255);
            CHECKBOX_OFF_HOVER_U32 = IM_COL32(210, 210, 220, 255);
            CHECKBOX_BORDER_U32 = IM_COL32(180, 180, 192, 255);
            CHECKMARK_U32 = IM_COL32(255, 255, 255, 255);
            SLIDER_GRAB_U32 = IM_COL32(255, 255, 255, 255);
        }
        else
        {
            BG_DARK = ImVec4(0.08f, 0.08f, 0.11f, 1.0f);
            BG_PANEL = ImVec4(0.10f, 0.10f, 0.14f, 1.0f);
            TEXT_BRIGHT = ImVec4(0.95f, 0.95f, 0.97f, 1.0f);
            TEXT_DIM = ImVec4(0.55f, 0.55f, 0.60f, 1.0f);
            FRAME_BG = ImVec4(0.14f, 0.14f, 0.18f, 1.0f);
            FRAME_BG_HOVER = ImVec4(0.18f, 0.18f, 0.24f, 1.0f);
            FRAME_BG_ACTIVE = ImVec4(0.22f, 0.22f, 0.28f, 1.0f);
            BUTTON_BG = ImVec4(0.16f, 0.16f, 0.22f, 1.0f);
            BUTTON_BG_HOVER = ImVec4(0.22f, 0.22f, 0.30f, 1.0f);
            BUTTON_BG_ACTIVE = ImVec4(0.28f, 0.28f, 0.36f, 1.0f);
            HEADER_BG = ImVec4(0.16f, 0.16f, 0.22f, 1.0f);
            BG_WINDOW_U32 = IM_COL32(20, 20, 28, 255);
            TRACK_BG_U32 = IM_COL32(30, 30, 40, 255);
            CHECKBOX_OFF_U32 = IM_COL32(30, 30, 40, 255);
            CHECKBOX_OFF_HOVER_U32 = IM_COL32(40, 40, 55, 255);
            CHECKBOX_BORDER_U32 = IM_COL32(55, 55, 70, 255);
            CHECKMARK_U32 = IM_COL32(255, 255, 255, 255);
            SLIDER_GRAB_U32 = IM_COL32(255, 255, 255, 255);
        }
    }

    inline void ApplyStyle()
    {
        ImGuiStyle& s = ImGui::GetStyle();
        s.WindowRounding = 14.0f;
        s.FrameRounding = 6.0f;
        s.ChildRounding = 10.0f;
        s.GrabRounding = 5.0f;
        s.WindowPadding = ImVec2(0, 0);
        s.FramePadding = ImVec2(8, 4);
        s.ItemSpacing = ImVec2(8, 6);
    }

    inline void PushColors()
    {
        ImGui::PushStyleColor(ImGuiCol_WindowBg, BG_DARK);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, BG_PANEL);
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(
            ((ACCENT_U32 >> IM_COL32_R_SHIFT) & 0xFF) / 255.f * 0.35f,
            ((ACCENT_U32 >> IM_COL32_G_SHIFT) & 0xFF) / 255.f * 0.35f,
            ((ACCENT_U32 >> IM_COL32_B_SHIFT) & 0xFF) / 255.f * 0.35f,
            1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, FRAME_BG);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, FRAME_BG_HOVER);
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, FRAME_BG_ACTIVE);
        ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_SliderGrab, ACCENT);
        ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ACCENT_HOVER);
        ImGui::PushStyleColor(ImGuiCol_Button, BUTTON_BG);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, BUTTON_BG_HOVER);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, BUTTON_BG_ACTIVE);
        ImGui::PushStyleColor(ImGuiCol_Header, HEADER_BG);
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ACCENT);
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ACCENT);
        ImGui::PushStyleColor(ImGuiCol_Text, TEXT_BRIGHT);
        ImGui::PushStyleColor(ImGuiCol_TextDisabled, TEXT_DIM);
        ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(
            ((ACCENT_U32 >> IM_COL32_R_SHIFT) & 0xFF) / 255.f * 0.35f,
            ((ACCENT_U32 >> IM_COL32_G_SHIFT) & 0xFF) / 255.f * 0.35f,
            ((ACCENT_U32 >> IM_COL32_B_SHIFT) & 0xFF) / 255.f * 0.35f,
            1.0f));
    }

    constexpr int COLOR_COUNT = 18;

    inline void PopColors() { ImGui::PopStyleColor(COLOR_COUNT); }
}
