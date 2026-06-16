#pragma once
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "ui.h"

namespace custom
{
    inline bool TabButton(const char* label, ImVec2 pos, bool Toggled)
    {
        ImGui::PushFont(font::Tab_Poppins_SemiBold);
        ImGui::SetCursorPos(pos);

        ImVec4 textColor = Toggled ? color::Text_Blue : color::Text_Gray;
        ImGui::PushStyleColor(ImGuiCol_Text, textColor);

        ImVec2 textSize = ImGui::CalcTextSize(label);
        ImGui::InvisibleButton(label, textSize);
        bool clicked = ImGui::IsItemClicked();

        ImGui::SetCursorPos(pos);
        ImGui::Text("%s", label);

        if (Toggled)
        {
            ImVec2 min = ImGui::GetItemRectMin();
            ImVec2 max = ImGui::GetItemRectMax();
            ImDrawList* drawList = ImGui::GetWindowDrawList();

            float underlineY = max.y + 7.0f;
            drawList->AddLine(ImVec2(min.x, underlineY), ImVec2(max.x, underlineY), ImGui::GetColorU32(color::Text_Blue), 2.0f);

            float glowHeight = 16.0f;
            float glowAlpha[] = { 24, 14, 6 };
            for (int i = 0; i < 3; ++i)
            {
                ImU32 col = IM_COL32(179, 77, 255, (int)glowAlpha[i]);
                drawList->AddRectFilled(
                    ImVec2(min.x, underlineY - (i + 1) * (glowHeight / 3.0f)),
                    ImVec2(max.x, underlineY - i * (glowHeight / 3.0f)),
                    col, 0.0f);
            }
        }

        ImGui::PopStyleColor();
        ImGui::PopFont();
        return clicked;
    }

    inline void Begin()
    {
        ImGui::SetNextWindowSize(WindowSize);
        ImGui::Begin("Astryn Cheats Private", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);
    }

    inline void BeginSection(const std::string SectionName, ImVec2 Pos, ImVec2 Size)
    {
        ImGui::SetCursorPos(Pos);
        ImGui::BeginChild(SectionName.c_str(), Size, true, ImGuiWindowFlags_NoScrollbar);

        ImGui::PushStyleColor(ImGuiCol_ChildBg, color::Background_Gray);
        ImGui::PushStyleColor(ImGuiCol_Text, color::Text_Blue);

        float headerHeight = 36.0f;
        ImVec2 headerSize(Size.x, headerHeight);                    // Explicit constructor

        // Fixed: Explicit ImVec2 instead of braced init
        ImVec2 cursorPos(
            ImGui::GetCursorPosX() - 7.f,
            ImGui::GetCursorPosY() - 3.5f
        );
        ImGui::SetCursorPos(cursorPos);

        ImGui::BeginChild((SectionName + "_header").c_str(), headerSize, true, ImGuiWindowFlags_NoScrollbar);

        ImGui::SetCursorPos(ImVec2(5.0f, 9.0f));
        ImGui::PushFont(font::Tab_Poppins_SemiBold);
        ImGui::Text("%s", SectionName.c_str());
        ImGui::PopFont();

        ImGui::PopStyleColor(2);
        ImGui::EndChild();
    }

    inline bool CheckBox(const char* label, bool* v)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.1f);
        ImGui::PushStyleColor(ImGuiCol_Text, color::Text_Gray);
        ImGui::PushFont(font::Tab_Poppins_SemiBold);

        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
        {
            ImGui::PopFont(); ImGui::PopStyleVar(); ImGui::PopStyleColor();
            return false;
        }

        const ImGuiStyle& style = ImGui::GetStyle();
        const ImGuiID id = window->GetID(label);
        const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
        const float square_sz = ImGui::GetFrameHeight() - 15.0f;
        const ImVec2 pos = window->DC.CursorPos;

        const float total_width = square_sz + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f);
        const float total_height = label_size.y + style.FramePadding.y - 6.f;

        const ImRect total_bb(pos, ImVec2(pos.x + total_width, pos.y + total_height));

        ImGui::ItemSize(total_bb, style.FramePadding.y);
        if (!ImGui::ItemAdd(total_bb, id))
        {
            ImGui::PopFont(); ImGui::PopStyleVar(); ImGui::PopStyleColor();
            return false;
        }

        bool hovered, held;
        bool clicked = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);

        if (clicked)
        {
            *v = !*v;
            ImGui::MarkItemEdited(id);
        }

        const ImRect check_bb(pos, ImVec2(pos.x + square_sz, pos.y + square_sz));

        ImGui::RenderFrame(check_bb.Min, check_bb.Max,
            ImGui::GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg),
            true, style.FrameRounding);

        if (*v)
        {
            const float pad = ImMax(1.0f, (float)(int)(square_sz / 4.0f));
            window->DrawList->AddRectFilled(check_bb.Min, check_bb.Max, ImColor(179, 77, 255, 255), style.FrameRounding);
            ImGui::RenderCheckMark(window->DrawList, ImVec2(check_bb.Min.x + pad, check_bb.Min.y + pad),
                IM_COL32(255, 255, 255, 255), square_sz - pad * 2.0f);
        }

        const float label_offset_x = style.ItemInnerSpacing.x - 1.f;
        const ImVec2 label_pos(
            check_bb.Max.x + label_offset_x,
            check_bb.Min.y + (check_bb.GetHeight() - label_size.y + 2.5f) * 0.5f
        );

        ImGui::PushStyleColor(ImGuiCol_Text, *v ? custom::color::Text_White : custom::color::Text_Gray);
        if (label_size.x > 0.0f)
            ImGui::RenderText(label_pos, label);
        ImGui::PopStyleColor();

        ImGui::PopFont();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();

        return clicked;
    }

    inline bool SliderFloat(const char* label, float* v, float v_min, float v_max)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.1f);
        ImGui::PushStyleColor(ImGuiCol_Text, color::Text_Gray);
        ImGui::PushFont(font::Tab_Poppins_SemiBold);

        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
        {
            ImGui::PopFont(); ImGui::PopStyleVar(); ImGui::PopStyleColor();
            return false;
        }

        const ImGuiStyle& style = ImGui::GetStyle();
        const ImVec2 pos = window->DC.CursorPos;
        const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

        float slider_width = ImGui::GetContentRegionAvail().x;
        float slider_height = 6.0f;
        float spacing = 4.0f;

        ImGui::RenderText(pos, label);

        char value_buf[32];
        snprintf(value_buf, 32, "%.1f", *v);
        ImVec2 value_size = ImGui::CalcTextSize(value_buf);
        ImGui::RenderText(ImVec2(pos.x + slider_width - value_size.x, pos.y), value_buf);

        ImVec2 slider_pos(pos.x, pos.y + label_size.y + spacing);

        ImRect slider_bb(slider_pos, ImVec2(slider_pos.x + slider_width, slider_pos.y + slider_height));
        ImRect total_bb(pos, ImVec2(pos.x + slider_width, pos.y + label_size.y + spacing + slider_height));

        ImGui::ItemSize(total_bb, style.FramePadding.y);
        const ImGuiID id = window->GetID(label);
        if (!ImGui::ItemAdd(total_bb, id))
        {
            ImGui::PopFont(); ImGui::PopStyleVar(); ImGui::PopStyleColor();
            return false;
        }

        bool hovered, held;
        ImGui::ButtonBehavior(slider_bb, id, &hovered, &held);

        if (held && ImGui::IsMouseDragging(0))
        {
            float mouse_x = ImGui::GetIO().MousePos.x - slider_bb.Min.x;
            float t = ImClamp(mouse_x / slider_width, 0.0f, 1.0f);
            *v = v_min + (v_max - v_min) * t;
        }

        float t = (*v - v_min) / (v_max - v_min);
        ImU32 bg_col = ImGui::GetColorU32(hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
        ImU32 fill_col = IM_COL32(179, 77, 255, 255);

        window->DrawList->AddRectFilled(slider_bb.Min, slider_bb.Max, bg_col, 2.0f);
        window->DrawList->AddRectFilled(slider_bb.Min, ImVec2(slider_bb.Min.x + slider_width * t, slider_bb.Max.y), fill_col, 2.0f);
        window->DrawList->AddRect(slider_bb.Min, slider_bb.Max, ImGui::GetColorU32(color::Border_Color), 2.0f);

        float handle_x = slider_bb.Min.x + slider_width * t;
        float handle_y = slider_bb.Min.y + slider_height / 2.0f;
        window->DrawList->AddCircleFilled(ImVec2(handle_x, handle_y), 4.0f, IM_COL32(30, 30, 30, 255));

        ImGui::PopFont();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();

        return true; // You can return actual change state if needed
    }
}