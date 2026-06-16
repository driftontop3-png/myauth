#pragma once
#include <Windows.h>

inline bool get_menu = true;
inline int tab = 0;
namespace features
{
	inline bool visiblecheck = false;
	inline bool targetline = false;
}

namespace settings
{
	inline constexpr float unlimited_distance = 2500.f;
	inline float render_distance = 500.f;
	inline int theme_preset = 0;

	inline bool IsWithinRenderDistance(float distance_m)
	{
		if (render_distance >= unlimited_distance - 1.f)
			return true;
		return distance_m <= render_distance;
	}
}

namespace visuals
{
	inline bool enable = true;
	inline bool CorneredBox = false;
	inline bool fill_box = false;
	inline bool line = false;
	inline bool headc = false;
	inline bool distance = false;
	inline bool skeleton = false;
	inline float SkeletonThickness = 2.85f;
	inline bool box = false;
	inline bool distance_bounce = false;
	inline bool radar = false;
}

namespace aimbot
{
	inline bool memoryaim = false;
	inline float smooth = 5;
	inline bool show_fov = false;
	inline bool fill_fov = false;
	inline bool controller_support = false;
	inline float fov = 100;
	inline static int current_key = VK_RBUTTON;
}

namespace triggerbot
{
	inline bool enabled = false;
	inline float lock_distance = 25.f;
}
