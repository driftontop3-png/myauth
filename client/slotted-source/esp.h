#pragma once
#include "imgui/imgui.h"
#include "updates/sdk.h"
#include "main/features.h"
#include "menu/echo_style.h"
#include <vector>
#include <cmath>

namespace esp_colors
{
	inline ImColor visible = ImColor(235, 46, 74, 250);
	inline ImColor hidden = ImColor(180, 40, 60, 250);
	inline ImColor visible_fill = ImColor(235, 46, 74, 100);
	inline ImColor hidden_fill = ImColor(180, 40, 60, 100);
	inline ImColor fov = ImColor(235, 46, 74, 180);
	inline ImColor target_line = ImColor(255, 100, 120, 250);
	inline ImColor distance_text = ImColor(255, 180, 190, 255);
	inline ImColor crosshair = ImColor(235, 46, 74, 255);
	inline ImColor arrow = ImColor(235, 46, 74, 230);
	inline ImColor radar_local = ImColor(80, 255, 120, 255);
}

struct RadarBlip
{
	float x = 0.f;
	float y = 0.f;
	bool visible = false;
};

inline void ApplyThemePreset(int preset)
{
	settings::theme_preset = preset;

	struct ThemePreset
	{
		ImU32 accent;
		ImColor visible;
		ImColor hidden;
		ImColor visible_fill;
		ImColor hidden_fill;
		ImColor fov;
		ImColor target_line;
		ImColor distance_text;
	};

	static const ThemePreset presets[] =
	{
		{ IM_COL32(235, 46, 74, 255),  ImColor(235, 46, 74, 250),  ImColor(180, 40, 60, 250),  ImColor(235, 46, 74, 100),  ImColor(180, 40, 60, 100),  ImColor(235, 46, 74, 180),  ImColor(255, 100, 120, 250), ImColor(255, 180, 190, 255) },
		{ IM_COL32(179, 77, 255, 255), ImColor(179, 77, 255, 250), ImColor(130, 70, 210, 250), ImColor(179, 77, 255, 100), ImColor(130, 70, 210, 100), ImColor(179, 77, 255, 180), ImColor(200, 140, 255, 250), ImColor(220, 190, 255, 255) },
		{ IM_COL32(77, 140, 255, 255), ImColor(77, 140, 255, 250), ImColor(50, 90, 180, 250),  ImColor(77, 140, 255, 100), ImColor(50, 90, 180, 100),  ImColor(77, 140, 255, 180), ImColor(140, 180, 255, 250), ImColor(190, 210, 255, 255) },
		{ IM_COL32(77, 255, 140, 255), ImColor(77, 255, 140, 250), ImColor(50, 180, 90, 250),  ImColor(77, 255, 140, 100), ImColor(50, 180, 90, 100),  ImColor(77, 255, 140, 180), ImColor(140, 255, 180, 250), ImColor(190, 255, 210, 255) },
		{ IM_COL32(77, 220, 255, 255), ImColor(77, 220, 255, 250), ImColor(50, 160, 190, 250), ImColor(77, 220, 255, 100), ImColor(50, 160, 190, 100), ImColor(77, 220, 255, 180), ImColor(140, 230, 255, 250), ImColor(190, 240, 255, 255) },
		{ IM_COL32(255, 140, 77, 255), ImColor(255, 140, 77, 250), ImColor(200, 100, 50, 250), ImColor(255, 140, 77, 100), ImColor(200, 100, 50, 100), ImColor(255, 140, 77, 180), ImColor(255, 180, 140, 250), ImColor(255, 210, 190, 255) },
		{ IM_COL32(70, 110, 190, 255), ImColor(70, 110, 190, 250), ImColor(50, 80, 140, 250), ImColor(70, 110, 190, 100), ImColor(50, 80, 140, 100), ImColor(70, 110, 190, 180), ImColor(110, 150, 220, 250), ImColor(60, 70, 90, 255) },
	};

	constexpr int kWhiteThemePreset = 6;
	const int idx = preset < 0 ? 0 : (preset >= IM_ARRAYSIZE(presets) ? IM_ARRAYSIZE(presets) - 1 : preset);
	const ThemePreset& t = presets[idx];

	EchoStyle::ApplyBasePalette(preset == kWhiteThemePreset);

	esp_colors::visible = t.visible;
	esp_colors::hidden = t.hidden;
	esp_colors::visible_fill = t.visible_fill;
	esp_colors::hidden_fill = t.hidden_fill;
	esp_colors::fov = t.fov;
	esp_colors::target_line = t.target_line;
	esp_colors::distance_text = t.distance_text;
	esp_colors::crosshair = t.fov;
	esp_colors::arrow = t.fov;

	EchoStyle::SetAccentFromU32(t.accent);
}

inline void DrawRadar(const std::vector<RadarBlip>& blips)
{
	if (!visuals::radar)
		return;

	const float radarSize = 180.f;
	const float radarX = 20.f;
	const float radarY = 20.f;
	const float centerX = radarX + radarSize * 0.5f;
	const float centerY = radarY + radarSize * 0.5f;

	ImDrawList* draw = ImGui::GetForegroundDrawList();
	draw->AddRectFilled(
		ImVec2(radarX, radarY),
		ImVec2(radarX + radarSize, radarY + radarSize),
		IM_COL32(10, 10, 15, 190),
		8.f);
	draw->AddRect(
		ImVec2(radarX, radarY),
		ImVec2(radarX + radarSize, radarY + radarSize),
		static_cast<ImU32>(esp_colors::fov),
		8.f, 0, 1.5f);
	draw->AddLine(ImVec2(centerX, radarY + 8.f), ImVec2(centerX, radarY + radarSize - 8.f), IM_COL32(255, 255, 255, 30), 1.f);
	draw->AddLine(ImVec2(radarX + 8.f, centerY), ImVec2(radarX + radarSize - 8.f, centerY), IM_COL32(255, 255, 255, 30), 1.f);
	draw->AddCircleFilled(ImVec2(centerX, centerY), 4.f, static_cast<ImU32>(esp_colors::radar_local), 16);

	for (const RadarBlip& blip : blips)
	{
		const ImColor dotColor = blip.visible ? esp_colors::visible : esp_colors::hidden;
		draw->AddCircleFilled(ImVec2(blip.x, blip.y), 3.5f, dotColor, 12);
	}
}

inline void CornerBox(int x, int y, int w, int h, const ImColor color, int thickness)
{
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(x, y), ImVec2(x, y + (h / 3)), color, thickness);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(x, y), ImVec2(x + (w / 3), y), color, thickness);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(x + w - (w / 3), y), ImVec2(x + w, y), color, thickness);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(x + w, y), ImVec2(x + w, y + (h / 3)), color, thickness);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(x, y + h - (h / 3)), ImVec2(x, y + h), color, thickness);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(x, y + h), ImVec2(x + (w / 3), y + h), color, thickness);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(x + w - (w / 3), y + h), ImVec2(x + w, y + h), color, thickness);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(x + w, y + h - (h / 3)), ImVec2(x + w, y + h), color, thickness);
}
inline void Box(int X, int Y, int W, int H, const ImColor color, int thickness)
{
	ImGui::GetForegroundDrawList()->AddLine(ImVec2{ (float)X, (float)Y, }, ImVec2{ (float)(X + W), (float)Y }, color, thickness);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2{ (float)(X + W), (float)Y, }, ImVec2{ (float)(X + W), (float)(Y + H) }, color, thickness);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2{ (float)X, (float)(Y + H) }, ImVec2{ (float)(X + W), (float)(Y + H) }, color, thickness);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2{ (float)X, (float)Y, }, ImVec2{ (float)X, (float)(Y + H) }, color, thickness);

}

inline void FilledRect(int x, int y, int w, int h, const ImColor color)
{
	ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), color, 0, 0);
}

inline void Line(Vector2 target, const ImColor color)
{
	ImGui::GetForegroundDrawList()->AddLine(
		ImVec2(static_cast<float>(screen_center_x), 0.f),
		ImVec2(static_cast<float>(target.x), static_cast<float>(target.y)),
		color,
		1.f);
}
inline void Distance(Vector2 location, float distance, const ImColor color)
{
	char dist[64];
	sprintf_s(dist, "%.fm", distance);
	ImVec2 text_size = ImGui::CalcTextSize(dist);
	ImGui::GetForegroundDrawList()->AddText(ImVec2(location.x - text_size.x / 2, location.y - text_size.y / 2), color, dist);
}
inline void targetline(const ImVec2& start, const ImVec2& end, const ImColor& color)
{
	ImDrawList* draw_list = ImGui::GetForegroundDrawList();
	draw_list->AddLine(start, end, color);
}

inline void DrawLineSkeleton(const std::vector<Vector2>& points, const ImColor& color, float thickness = 2.f)
{
	if (points.size() < 15)
		return;

	ImDrawList* draw_list = ImGui::GetForegroundDrawList();

	static const int links[][2] = {
		{ 0, 14 }, { 1, 4 }, { 1, 2 }, { 2, 3 }, { 4, 5 }, { 5, 6 },
		{ 0, 7 }, { 7, 8 }, { 8, 9 }, { 9, 12 }, { 7, 10 }, { 10, 11 }, { 11, 13 }
	};

	for (const auto& link : links)
	{
		const int a = link[0];
		const int b = link[1];
		draw_list->AddLine(
			ImVec2(static_cast<float>(points[a].x), static_cast<float>(points[a].y)),
			ImVec2(static_cast<float>(points[b].x), static_cast<float>(points[b].y)),
			color,
			thickness);
	}
}
