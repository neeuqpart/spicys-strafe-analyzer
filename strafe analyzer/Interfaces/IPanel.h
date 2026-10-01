#pragma once
#include <cstdint>
#include "../Indices/Indices.h"

class IPanel {
public:
	const char* get_panel_name(std::uintptr_t panel_id) {
		using original_fn = const char* (__thiscall*)(IPanel*, std::uintptr_t);
		return (*(original_fn**)this)[Indices::get_panel_name](this, panel_id);
	}
};
