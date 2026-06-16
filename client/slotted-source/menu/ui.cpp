#include "ui.h"
#include "echo_menu.h"
#include "echo_style.h"
#include "echo_fa_loader.h"
#include <windows.h>
#include "../main/features.h"
#include "../esp.h"

namespace custom::font
{
	ImFont* Poppins = nullptr;
	ImFont* Poppins_SemiBold = nullptr;
	ImFont* Poppins_SemiBold_txt = nullptr;
	ImFont* Tab_Poppins_SemiBold = nullptr;
}

void custom::font::LoadFonts()
{
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->Clear();
	Poppins_SemiBold = io.Fonts->AddFontFromMemoryTTF(f_Poppins_SemiBold, sizeof(f_Poppins_SemiBold), 18.0f);
	EchoFa::MergeIconFont();

	Poppins_SemiBold_txt = Poppins_SemiBold;
	Tab_Poppins_SemiBold = Poppins_SemiBold;
	Poppins = Poppins_SemiBold;
	io.Fonts->Build();
	io.FontDefault = Poppins_SemiBold;
}

void custom::InitUI()
{
	font::LoadFonts();
	ApplyThemePreset(settings::theme_preset);

	ImGuiStyle& style = ImGui::GetStyle();
	EchoStyle::ApplyStyle();
	style.WindowBorderSize = 0.0f;
	style.FrameBorderSize = 0.0f;
	style.Colors[ImGuiCol_Text] = EchoStyle::TEXT_BRIGHT;
	style.Colors[ImGuiCol_TextDisabled] = EchoStyle::TEXT_DIM;
}

void UI::UI()
{
	if (!get_menu)
		return;

	EchoMenu::Render(1.0f);
}
