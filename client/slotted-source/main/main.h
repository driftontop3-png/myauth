#pragma once
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_dx9.h"
#include "../imgui/imgui_impl_win32.h"
#include <windows.h>
#include <XInput.h>
#include <dwmapi.h>
#include <d3d9.h>
#include "features.h"
#include "../external/gui.h"
#include "../updates/sdk.h"
#include "../communication/memory.hpp"
#include "../updates/offsets.h"
#include "../esp.h"
#include "../menu/ui.h"
#include "../external/input.h"
#include <string>
#include <vector>
#include <algorithm>
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "xinput.lib")

inline std::vector<RadarBlip> g_radar_blips;

IDirect3D9Ex* p_object = NULL;
IDirect3DDevice9Ex* p_device = NULL;
D3DPRESENT_PARAMETERS p_params = { NULL };
MSG messager = { NULL };
HWND MyWnd = NULL;
HWND GameWnd = NULL;
DWORD ProcId;

inline bool aim_visible_ok(uintptr_t mesh)
{
	if (!features::visiblecheck)
		return true;
	return is_visible(mesh);
}

inline bool get_head_aim(uintptr_t mesh, Vector3& out_world, Vector2& out_screen)
{
	Vector3 head = get_entity_bone(mesh, 67);
	if (head.x == 0.0 && head.y == 0.0 && head.z == 0.0)
		head = get_entity_bone(mesh, 66);
	if (head.x == 0.0 && head.y == 0.0 && head.z == 0.0)
		return false;

	out_world = head;
	out_screen = project_world_to_screen(head);
	return true;
}

inline bool is_controller_aim_active()
{
	XINPUT_STATE state{};
	if (XInputGetState(0, &state) != ERROR_SUCCESS)
		return false;

	if (state.Gamepad.bLeftTrigger > 30)
		return true;
	if (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)
		return true;
	if (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)
		return true;
	return false;
}

inline bool is_aim_key_held()
{
	if (aimbot::controller_support && is_controller_aim_active())
		return true;
	return (GetAsyncKeyState(aimbot::current_key) & 0x8000) != 0;
}

inline void run_triggerbot()
{
	if (!triggerbot::enabled)
		return;

	if (!cache::closest_mesh)
		return;

	if (!aim_visible_ok(cache::closest_mesh))
		return;

	Vector3 head3d{};
	Vector2 head2d{};
	if (!get_head_aim(cache::closest_mesh, head3d, head2d))
		return;

	const float dx = static_cast<float>(head2d.x - screen_center_x);
	const float dy = static_cast<float>(head2d.y - screen_center_y);
	const float head_dist = sqrtf(dx * dx + dy * dy);
	if (head_dist > triggerbot::lock_distance)
		return;

	input::left_click_game(GameWnd);
}

void SetupImGuiStyle()
{
	ImGuiStyle& style = ImGui::GetStyle();

	style.WindowRounding = 4.0f;
	style.FrameRounding = 2.0f;
	style.ScrollbarRounding = 3.0f;
	style.GrabRounding = 2.0f;
	style.ItemSpacing = ImVec2(6, 4);
	style.WindowPadding = ImVec2(10, 10);
	style.ChildRounding = 4.0f;
	style.ScrollbarSize = 8.0f;

	ImVec4* colors = style.Colors;

	colors[ImGuiCol_Text] = ImVec4(0.86f, 0.93f, 0.89f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.14f, 0.17f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.16f, 0.17f, 0.21f, 1.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.92f, 0.18f, 0.29f, 0.78f);
	colors[ImGuiCol_Button] = ImVec4(0.47f, 0.77f, 0.83f, 0.14f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.92f, 0.18f, 0.29f, 0.86f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.92f, 0.18f, 0.29f, 0.76f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.92f, 0.18f, 0.29f, 0.86f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
	colors[ImGuiCol_Separator] = ImVec4(0.92f, 0.18f, 0.29f, 0.50f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
}

HRESULT DirectXInit()
{
	if (FAILED(Direct3DCreate9Ex(D3D_SDK_VERSION, &p_object))) exit(3);
	ZeroMemory(&p_params, sizeof(p_params));
	p_params.Windowed = TRUE;
	p_params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	p_params.hDeviceWindow = MyWnd;
	p_params.MultiSampleQuality = D3DMULTISAMPLE_NONE;
	p_params.BackBufferFormat = D3DFMT_A8R8G8B8;
	p_params.BackBufferWidth = width;
	p_params.BackBufferHeight = height;
	p_params.EnableAutoDepthStencil = TRUE;
	p_params.AutoDepthStencilFormat = D3DFMT_D16;
	p_params.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
	if (FAILED(p_object->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, MyWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &p_params, 0, &p_device)))
	{
		p_object->Release();
		exit(4);
	}
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(MyWnd);
	ImGui_ImplDX9_Init(p_device);
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.IniFilename = 0;
	ImGuiStyle* style = &ImGui::GetStyle();
	ImVec4* colors = style->Colors;
	style->WindowBorderSize = 1;
	style->FrameBorderSize = 1;

	custom::InitUI();
	ApplyThemePreset(settings::theme_preset);

	p_object->Release();
	return S_OK;
}

void CreateOverlay()
{
	WNDCLASSEXA wcex = {
		sizeof(WNDCLASSEXA),
		0,
		DefWindowProcA,
		0,
		0,
		0,
		LoadIcon(0, IDI_APPLICATION),
		LoadCursor(0, IDC_ARROW),
		0,
		0,
		"Astryn Cheats Private",
		LoadIcon(0, IDI_APPLICATION)
	};
	ATOM rce = RegisterClassExA(&wcex);
	RECT rect;
	GetWindowRect(GetDesktopWindow(), &rect);
	MyWnd = gui::create_window_in_band(0, rce, L"Astryn Cheats Private", WS_POPUP, rect.left, rect.top, rect.right, rect.bottom, 0, 0, wcex.hInstance, 0, gui::ZBID_UIACCESS);
	SetWindowLong(MyWnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW);
	SetLayeredWindowAttributes(MyWnd, RGB(0, 0, 0), 255, LWA_ALPHA);
	MARGINS margin = { -1 };
	DwmExtendFrameIntoClientArea(MyWnd, &margin);
	ShowWindow(MyWnd, SW_SHOW);
	UpdateWindow(MyWnd);
}

intptr_t NetConnection = 0x4a8;
uintptr_t RotationInput = NetConnection + 0x8;
bool aim2(Vector3 newpos)
{
	fortnite::communcations::write<Vector3>(cache::player_controller + RotationInput, newpos);
	return true;
}

void mouseaim(uintptr_t target_mesh)
{
	if (!target_mesh)
		return;

	if (!aim_visible_ok(target_mesh))
		return;

	Vector3 head3d{};
	Vector2 head2d{};
	if (!get_head_aim(target_mesh, head3d, head2d))
		return;

	Vector2 target = Vector2(0, 0);

	float delta_x = static_cast<float>(head2d.x - screen_center_x);
	if (delta_x != 0.f)
	{
		target.x = delta_x / aimbot::smooth;
		if (head2d.x > screen_center_x)
		{
			if (target.x + screen_center_x > screen_center_x * 2)
				target.x = 0;
		}
		else if (head2d.x < screen_center_x)
		{
			if (target.x + screen_center_x < 0)
				target.x = 0;
		}
	}

	float delta_y = static_cast<float>(head2d.y - screen_center_y);
	if (delta_y != 0.f)
	{
		target.y = delta_y / aimbot::smooth;
		if (head2d.y > screen_center_y)
		{
			if (target.y + screen_center_y > screen_center_y * 2)
				target.y = 0;
		}
		else if (head2d.y < screen_center_y)
		{
			if (target.y + screen_center_y < 0)
				target.y = 0;
		}
	}

	if (std::abs(target.x) < 0.01f && std::abs(target.y) < 0.01f)
		return;

	aim2(Vector3(-target.y / 5.f, target.x / 5.f, 0.f));
}

void LoopaGame()
{
	uintptr_t EncryptedUWorld = fortnite::communcations::read<uintptr_t>(Base + UWORLD);
	uintptr_t DecryptedUWorld = DecryptUworld(EncryptedUWorld);
	cache::uworld = DecryptedUWorld;
	cache::game_instance = fortnite::communcations::read<uintptr_t>(cache::uworld + GAME_INSTANCE);
	cache::local_players = fortnite::communcations::read<uintptr_t>(fortnite::communcations::read<uintptr_t>(cache::game_instance + LOCAL_PLAYERS));
	cache::player_controller = fortnite::communcations::read<uintptr_t>(cache::local_players + PLAYER_CONTROLLER);
	cache::local_pawn = fortnite::communcations::read<uintptr_t>(cache::player_controller + LOCAL_PAWN);
	cache::current_weapon = fortnite::communcations::read<uintptr_t>(cache::local_pawn + CURRENT_WEAPON);
	if (cache::local_pawn != 0)
	{
		cache::root_component = fortnite::communcations::read<uintptr_t>(cache::local_pawn + ROOT_COMPONENT);
		cache::relative_location = fortnite::communcations::read<Vector3>(cache::root_component + RELATIVE_LOCATION);
		cache::player_state = fortnite::communcations::read<uintptr_t>(cache::local_pawn + PLAYER_STATE);
		cache::my_team_id = fortnite::communcations::read<int>(cache::player_state + TEAM_INDEX);
	}
	cache::game_state = fortnite::communcations::read<uintptr_t>(cache::uworld + GAME_STATE);
	cache::player_array = fortnite::communcations::read<uintptr_t>(cache::game_state + PLAYER_ARRAY);
	cache::player_count = fortnite::communcations::read<int>(cache::game_state + (PLAYER_ARRAY + sizeof(uintptr_t)));
	cache::closest_distance = FLT_MAX;
	cache::closest_mesh = NULL;
	cache::closest_aactor = NULL;
	cache::closest_pawn = NULL;
	g_radar_blips.clear();

	const float radarYawRad = visuals::radar && cache::uworld
		? static_cast<float>(get_view_point().rotation.y * M_PI / 180.0)
		: 0.f;
	const float radarCosYaw = cosf(radarYawRad);
	const float radarSinYaw = sinf(radarYawRad);

	for (int i = 0; i < cache::player_count; i++)
	{
		uintptr_t player_state = fortnite::communcations::read<uintptr_t>(cache::player_array + (i * sizeof(uintptr_t)));
		if (!player_state) continue;
		int player_team_id = fortnite::communcations::read<int>(player_state + TEAM_INDEX);
		if (player_team_id == cache::my_team_id) continue;
		uintptr_t pawn_private = fortnite::communcations::read<uintptr_t>(player_state + PAWN_PRIVATE);
		if (!pawn_private) continue;
		if (pawn_private == cache::local_pawn) continue;
		uintptr_t mesh = fortnite::communcations::read<uintptr_t>(pawn_private + MESH);
		if (!mesh) continue;

		Vector3 aim3d{};
		Vector2 aim2d{};
		if (!get_head_aim(mesh, aim3d, aim2d))
			continue;

		Vector3 head3d = aim3d;
		Vector2 head2d = aim2d;
		Vector3 bottom3d = get_entity_bone(mesh, 0);
		Vector2 bottom2d = project_world_to_screen(bottom3d);
		float box_height = abs(head2d.y - bottom2d.y);
		float box_width = box_height * 0.50f;
		float distance = cache::relative_location.distance(bottom3d) / 100;

		if (!settings::IsWithinRenderDistance(distance))
			continue;

		if (visuals::radar && cache::uworld)
		{
			const float dx = static_cast<float>(bottom3d.x - cache::relative_location.x);
			const float dy = static_cast<float>(bottom3d.y - cache::relative_location.y);
			const float rotatedX = dx * radarCosYaw + dy * radarSinYaw;
			const float rotatedY = -dx * radarSinYaw + dy * radarCosYaw;

			constexpr float radarRange = 15000.f;
			constexpr float radarSize = 180.f;
			constexpr float radarX = 20.f;
			constexpr float radarY = 20.f;
			const float scale = (radarSize * 0.5f - 10.f) / radarRange;

			RadarBlip blip;
			blip.x = radarX + radarSize * 0.5f + rotatedX * scale;
			blip.y = radarY + radarSize * 0.5f - rotatedY * scale;
			blip.visible = is_visible(mesh);
			blip.x = (std::max)(radarX + 6.f, (std::min)(blip.x, radarX + radarSize - 6.f));
			blip.y = (std::max)(radarY + 6.f, (std::min)(blip.y, radarY + radarSize - 6.f));
			g_radar_blips.push_back(blip);
		}

		if (features::targetline)
		{
			if (is_visible(mesh))
			{
				float dx = static_cast<float>(aim2d.x - screen_center_x);
				float dy = static_cast<float>(aim2d.y - screen_center_y);
				float dist = sqrtf(dx * dx + dy * dy);

				if (dist <= aimbot::fov)
				{
					ImVec2 screen_center = ImVec2(screen_center_x, screen_center_y);
					ImVec2 head_position = ImVec2(static_cast<float>(aim2d.x), static_cast<float>(aim2d.y));
					targetline(screen_center, head_position, esp_colors::target_line);
				}
			}
		}

		if (visuals::enable)
		{
			if (visuals::skeleton) {
				std::vector<Vector3> bones = {
			get_entity_bone(mesh, 66),
			get_entity_bone(mesh, 9),
			get_entity_bone(mesh, 10),
			get_entity_bone(mesh, 11),
			get_entity_bone(mesh, 38),
			get_entity_bone(mesh, 39),
			get_entity_bone(mesh, 40),
			get_entity_bone(mesh, 2),
			get_entity_bone(mesh, 71),
			get_entity_bone(mesh, 72),
			get_entity_bone(mesh, 78),
			get_entity_bone(mesh, 79),
			get_entity_bone(mesh, 75),
			get_entity_bone(mesh, 82),
			get_entity_bone(mesh, 67)
				};

				std::vector<Vector2> screenPositions(bones.size());
				for (size_t j = 0; j < bones.size(); ++j) {
					screenPositions[j] = project_world_to_screen(bones[j]);
				}

				const ImColor skel_color = is_visible(mesh) ? esp_colors::visible : esp_colors::hidden;
				DrawLineSkeleton(screenPositions, skel_color, visuals::SkeletonThickness);
			}
		}

		if (visuals::enable)
		{
			if (visuals::headc)
			{
				std::vector<Vector3> bones = {
	   get_entity_bone(mesh, 66),
	   get_entity_bone(mesh, 9),
	   get_entity_bone(mesh, 10),
	   get_entity_bone(mesh, 11),
	   get_entity_bone(mesh, 38),
	   get_entity_bone(mesh, 39),
	   get_entity_bone(mesh, 40),
	   get_entity_bone(mesh, 2),
	   get_entity_bone(mesh, 71),
	   get_entity_bone(mesh, 72),
	   get_entity_bone(mesh, 78),
	   get_entity_bone(mesh, 79),
	   get_entity_bone(mesh, 75),
	   get_entity_bone(mesh, 82),
	   get_entity_bone(mesh, 67)
				};

				std::vector<Vector2> screenPositions(bones.size());
				for (size_t j = 0; j < bones.size(); ++j) {
					screenPositions[j] = project_world_to_screen(bones[j]);
				}
				float baseRadius = 130.f;
				float radius = baseRadius / distance;
				if (is_visible(mesh))
				{
					ImGui::GetForegroundDrawList()->AddCircle(ImVec2(screenPositions[14].x, screenPositions[14].y), radius, ImGui::ColorConvertFloat4ToU32(esp_colors::visible), 64, 1);
				}
				else
				{
					ImGui::GetForegroundDrawList()->AddCircle(ImVec2(screenPositions[14].x, screenPositions[14].y), radius, ImGui::ColorConvertFloat4ToU32(esp_colors::hidden), 64, 1);
				}
			}
		}

		if (visuals::enable)
		{
			if (visuals::CorneredBox)
			{
				if (is_visible(mesh))
				{
					CornerBox(head2d.x - (box_width / 2), head2d.y, box_width, box_height, esp_colors::visible, 1);
				}
				else
				{
					CornerBox(head2d.x - (box_width / 2), head2d.y, box_width, box_height, esp_colors::hidden, 1);
				}
				if (visuals::fill_box) FilledRect(head2d.x - (box_width / 2), head2d.y, box_width, box_height, ImColor(0, 0, 0, 50));
			}
			if (visuals::line)
			{
				if (is_visible(mesh))
					Line(head2d, esp_colors::visible);
				else
					Line(head2d, esp_colors::hidden);
			}
		}
		if (visuals::distance)
		{
			ImVec2 pos = ImVec2(bottom2d.x, bottom2d.y - 15.0f);
			std::string distText = std::to_string(static_cast<int>(distance)) + "m";

			ImDrawList* draw = ImGui::GetForegroundDrawList();
			float time = static_cast<float>(ImGui::GetTime());
			float scale = 1.0f;

			if (visuals::distance_bounce)
			{
				scale += 0.05f * std::sin(time * 4.0f);
			}

			ImVec2 textSize = ImGui::CalcTextSize(distText.c_str());
			ImVec2 centeredPos = ImVec2(pos.x - textSize.x / 2.0f * scale, pos.y - textSize.y / 2.0f * scale);

			draw->AddText(NULL, 13.0f * scale, ImVec2(centeredPos.x + 1, centeredPos.y + 1), ImColor(0, 0, 0, 200), distText.c_str());
			draw->AddText(NULL, 13.0f * scale, ImVec2(centeredPos.x - 1, centeredPos.y), ImColor(0, 0, 0, 255), distText.c_str());
			draw->AddText(NULL, 13.0f * scale, ImVec2(centeredPos.x + 1, centeredPos.y), ImColor(0, 0, 0, 255), distText.c_str());
			draw->AddText(NULL, 13.0f * scale, ImVec2(centeredPos.x, centeredPos.y - 1), ImColor(0, 0, 0, 255), distText.c_str());
			draw->AddText(NULL, 13.0f * scale, ImVec2(centeredPos.x, centeredPos.y + 1), ImColor(0, 0, 0, 255), distText.c_str());
			draw->AddText(NULL, 13.0f * scale, centeredPos, esp_colors::distance_text, distText.c_str());
		}
		if (visuals::box)
		{
			if (is_visible(mesh))
			{
				Box(head2d.x - (box_width / 2), head2d.y, box_width, box_height, esp_colors::visible, 1);
			}
			else
			{
				Box(head2d.x - (box_width / 2), head2d.y, box_width, box_height, esp_colors::hidden, 1);
			}

			if (visuals::fill_box)
			{
				FilledRect(head2d.x - (box_width / 2), head2d.y, box_width, box_height, ImColor(0, 0, 0, 50));
			}
		}

		double dx = aim2d.x - screen_center_x;
		double dy = aim2d.y - screen_center_y;
		float dist = sqrtf(static_cast<float>(dx * dx + dy * dy));
		if (dist <= aimbot::fov && dist < cache::closest_distance)
		{
			if (!aim_visible_ok(mesh))
				continue;

			cache::closest_distance = dist;
			cache::closest_mesh = mesh;
		}
	}
	if (aimbot::memoryaim)
	{
		if (is_aim_key_held()) mouseaim(cache::closest_mesh);
	}

	run_triggerbot();
}

void update_overlay_clickthrough()
{
	LONG ex_style = GetWindowLong(MyWnd, GWL_EXSTYLE);
	if (get_menu)
		ex_style &= ~WS_EX_TRANSPARENT;
	else
		ex_style |= WS_EX_TRANSPARENT;
	SetWindowLong(MyWnd, GWL_EXSTYLE, ex_style);
}

void show_menu()
{
	update_overlay_clickthrough();

	const ImVec2 fov_center(
		ImGui::GetIO().DisplaySize.x / 2,
		ImGui::GetIO().DisplaySize.y / 2);

	if (aimbot::show_fov)
	{
		ImDrawList* draw = ImGui::GetForegroundDrawList();
		if (aimbot::fill_fov)
		{
			ImColor fill = esp_colors::fov;
			fill.Value.w = 0.12f;
			draw->AddCircleFilled(fov_center, aimbot::fov, fill, 100);
		}
		draw->AddCircle(fov_center, aimbot::fov, esp_colors::fov, 100, 1.0f);
	}

	DrawRadar(g_radar_blips);

	if (GetAsyncKeyState(VK_F2) & 1)
		get_menu = !get_menu;

	UI::UI();
}

HWND GetProcWindow(uint32_t pid)
{
	std::pair<HWND, uint32_t> params = { 0, pid };
	BOOL bresult = EnumWindows([](HWND hwnd, LPARAM lparam) -> BOOL
		{
			auto pparams = (std::pair<HWND, uint32_t>*)(lparam);
			uint32_t processid = 0;
			if (GetWindowThreadProcessId(hwnd, reinterpret_cast<LPDWORD>(&processid)) && processid == pparams->second)
			{
				SetLastError((uint32_t)-1);
				pparams->first = hwnd;
				return FALSE;
			}
			return TRUE;
		}, (LPARAM)&params);
	if (!bresult && GetLastError() == -1 && params.first) return params.first;
	return 0;
}

WPARAM RenderLoop()
{
	static RECT old_rc;
	ZeroMemory(&messager, sizeof(MSG));
	while (messager.message != WM_QUIT)
	{
		if (PeekMessage(&messager, MyWnd, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&messager);
			DispatchMessage(&messager);
		}
		if (GameWnd == NULL) exit(0);
		HWND active_wnd = GetForegroundWindow();
		RECT rc;
		POINT xy;
		ZeroMemory(&rc, sizeof(RECT));
		ZeroMemory(&xy, sizeof(POINT));
		rc.left = xy.x;
		rc.top = xy.y;
		ImGuiIO& io = ImGui::GetIO();
		io.DeltaTime = 1.0f / 60.0f;
		POINT p;
		GetCursorPos(&p);
		io.MousePos.x = p.x - xy.x;
		io.MousePos.y = p.y - xy.y;
		if (GetAsyncKeyState(0x1))
		{
			io.MouseDown[0] = true;
			io.MouseClicked[0] = true;
			io.MouseClickedPos[0].x = io.MousePos.x;
			io.MouseClickedPos[0].x = io.MousePos.y;
		}
		else
		{
			io.MouseDown[0] = false;
		}
		if (rc.left != old_rc.left || rc.right != old_rc.right || rc.top != old_rc.top || rc.bottom != old_rc.bottom)
		{
			old_rc = rc;
			width = rc.right;
			height = rc.bottom;
			p_params.BackBufferWidth = width;
			p_params.BackBufferHeight = height;
			SetWindowPos(MyWnd, (HWND)0, xy.x, xy.y, width, height, SWP_NOREDRAW);
			p_device->Reset(&p_params);
		}
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		LoopaGame();
		show_menu();
		ImGui::EndFrame();
		p_device->SetRenderState(D3DRS_ZENABLE, false);
		p_device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
		p_device->SetRenderState(D3DRS_SCISSORTESTENABLE, false);
		p_device->Clear(0, 0, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
		if (p_device->BeginScene() >= 0)
		{
			ImGui::Render();
			ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
			p_device->EndScene();
		}
		HRESULT result = p_device->Present(0, 0, 0, 0);
		if (result == D3DERR_DEVICELOST && p_device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		{
			ImGui_ImplDX9_InvalidateDeviceObjects();
			p_device->Reset(&p_params);
			ImGui_ImplDX9_CreateDeviceObjects();
		}
	}

	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	if (p_device != 0)
	{
		p_device->EndScene();
		p_device->Release();
	}
	if (p_object != 0) p_object->Release();
	DestroyWindow(MyWnd);
	return messager.wParam;
}
