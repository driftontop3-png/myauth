#ifndef UI_H
#define UI_H
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_impl_win32.h"
#include "font.h"
#include <string>

#pragma warning(push)
#pragma warning(disable: 4244)
#pragma warning(disable: 4267)

namespace custom
{
	enum Tabs
	{
		Aimbot,
		Visuals,
		Exploits,
		Settings
	};
	extern Tabs CurrentTab;

	namespace font
	{
		extern ImFont* Poppins;
		extern ImFont* Poppins_SemiBold;
		extern ImFont* Poppins_SemiBold_txt;
		extern ImFont* Tab_Poppins_SemiBold;

		void LoadFonts();
	}

	namespace color
	{
		inline ImVec4 Background_Gray = ImVec4(10.0f / 255.f, 10 / 255.f, 10 / 255.f, 1.0f);
		inline ImVec4 Background_Black = ImVec4(9.999899930335232e-07f, 9.999935173254926e-07f, 9.999999974752427e-07f, 1.0f);
		inline ImVec4 Border_Color = { 15 / 255.f, 15 / 255, 15 / 255, 1.0f };
		inline ImVec4 Text_Blue = { 0.7f, 0.3f, 1.0f, 1.0f };
		inline ImVec4 Text_Gray = { 80.0f / 255.f, 80.0f / 255.f, 80.0f / 255.f, 1.0f };
		inline ImVec4 Text_White = { 255.0f / 255.f, 255.0f / 255.f, 255.0f / 255.f, 1.0f };

		namespace TabButton
		{
			inline const ImVec4 ButtonColor = { 0.7f, 0.3f, 1.0f, 0.1f };
			inline const ImVec4 ButtonColorHovered = { 0.7f, 0.3f, 1.0f, 0.3f };
			inline const ImVec4 ButtonColorActive = { 0.7f, 0.3f, 1.0f, 0.2f };
		}
	}

	const ImVec2 WindowSize = { 700, 700 };
	const ImVec2 ButtonSize = { 50, 25 };

	void InitUI();
	bool CheckBox(const char* label, bool* v);
	bool TabButton(const char* label, ImVec2 pos, bool Toggled);
	void BeginSection(const std::string SectionName, ImVec2 Pos, ImVec2 Size);
	void Begin();
	bool SliderFloat(const char* label, float* v, float v_min, float v_max);
}

namespace UI
{
	void UI();
}

#pragma warning(pop)

#endif
