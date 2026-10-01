#pragma once
#include <cstdint>
#include <cstddef>
#include "../math/vector3d.h"

enum cmd_buttons {			// unique flag bit shift
	in_attack = (1 << 0),	// binary 0001
	in_jump = (1 << 1),		// binary 0010
	in_duck = (1 << 2),		// binary 0100
	in_forward = (1 << 3),
	in_back = (1 << 4),
	in_use = (1 << 5),
	in_cancel = (1 << 6),
	in_left = (1 << 7),
	in_right = (1 << 8),
	in_moveleft = (1 << 9),
	in_moveright = (1 << 10),
	in_attack2 = (1 << 11),
	in_run = (1 << 12),
	in_reload = (1 << 13),
	in_alt1 = (1 << 14),
	in_alt2 = (1 << 15),
	in_score = (1 << 16),
	in_speed = (1 << 17),
	in_walk = (1 << 18),
	in_zoom = (1 << 19),
	in_weapon1 = (1 << 20),
	in_weapon2 = (1 << 21),
	in_bullrush = (1 << 22),
	in_grenade1 = (1 << 23),
	in_grenade2 = (1 << 24),
	in_attack3 = (1 << 25)
};

class CUserCmd {
public:
	virtual ~CUserCmd() = default;
	int command_number;
	int tick_count;
	vec3_t viewangles;
	float forwardmove;
	float sidemove;
	float upmove;
	int buttons;
	std::uint8_t impulse;
	std::uint8_t padding[3];
	int weaponselect;
	int weaponsubtype;
	int random_seed;
	std::int16_t mousedx;
	std::int16_t mousedy;
	bool hasbeenpredicted;
};

static_assert(sizeof(void*) == 8, "CSS analyzer requires x64");
static_assert(offsetof(CUserCmd, command_number) == 0x08, "Unexpected CSS x64 CUserCmd layout: command_number");
static_assert(offsetof(CUserCmd, tick_count) == 0x0C, "Unexpected CSS x64 CUserCmd layout: tick_count");
static_assert(offsetof(CUserCmd, viewangles) == 0x10, "Unexpected CSS x64 CUserCmd layout: viewangles");
static_assert(offsetof(CUserCmd, forwardmove) == 0x1C, "Unexpected CSS x64 CUserCmd layout: forwardmove");
static_assert(offsetof(CUserCmd, sidemove) == 0x20, "Unexpected CSS x64 CUserCmd layout: sidemove");
static_assert(offsetof(CUserCmd, upmove) == 0x24, "Unexpected CSS x64 CUserCmd layout: upmove");
static_assert(offsetof(CUserCmd, buttons) == 0x28, "Unexpected CSS x64 CUserCmd layout: buttons");
static_assert(sizeof(CUserCmd) == 0x48, "Unexpected CSS x64 CUserCmd size");
static_assert(sizeof(CUserCmd::mousedx) == sizeof(std::int16_t), "Source mouse deltas are shorts");
