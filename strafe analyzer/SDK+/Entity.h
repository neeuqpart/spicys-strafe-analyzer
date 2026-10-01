#pragma once
#include <cstdint>
#include "fnv1.h"
#include "netvars.h"
#include "../math/vector3d.h"

enum entity_flags 
{
	fl_onground = (1 << 0),
	fl_ducking  = (1 << 1),
};

class CssPlayer 
{
public:
	vec3_t& origin() { return field<vec3_t>("DT_BasePlayer", "m_vecOrigin", 0x428); }
	vec3_t& velocity() { return field<vec3_t>("DT_BasePlayer", "m_vecVelocity[0]", 0x148); }
	int& flags() { return field<int>("DT_BasePlayer", "m_fFlags", 0x440); }

	bool is_in_air() { return (flags() & fl_onground) == 0; }

private:
	template <typename T>
	T& field(const char* table, const char* property, uintptr_t fallback_offset)
	{
		static T fallback_val{};
		if (!this) return fallback_val;

		uintptr_t offset = netvar_manager::get_net_var(fnv::hash("DT_CSPlayer"), fnv::hash(property));
		if (!offset) {
			offset = netvar_manager::get_net_var(fnv::hash(table), fnv::hash(property));
		}
		if (!offset) {
			offset = fallback_offset;
		}

		return *reinterpret_cast<T*>(reinterpret_cast<std::uintptr_t>(this) + offset);
	}
};
