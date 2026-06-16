#include "../imgui/imgui.h"
#include "echo_icons.h"
#include "echo_fa_loader.h"

// Font byte array only — must NOT contain #include "imgui.h"
#include "echo_fontawesome.h"

void EchoFa::MergeIconFont()
{
	ImGuiIO& io = ImGui::GetIO();
	ImFontConfig icons_config;
	icons_config.MergeMode = true;
	icons_config.PixelSnapH = true;
	icons_config.GlyphMinAdvanceX = 22.0f;
	static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };

	io.Fonts->AddFontFromMemoryTTF(
		(void*)FontData::fontawesome_data,
		sizeof(FontData::fontawesome_data),
		18.0f,
		&icons_config,
		icons_ranges);
}
