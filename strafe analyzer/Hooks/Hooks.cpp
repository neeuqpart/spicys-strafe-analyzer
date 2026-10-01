#include <stdexcept>
#include "hooks.h"
#include "../sdk+/drawing.h"
#include "../sdk+/utils.hpp"

namespace {
template <typename T = void*>
T get_virtual(void* instance, std::size_t index)
{
	auto table = *reinterpret_cast<void***>(instance);
	return reinterpret_cast<T>(table[index]);
}

bool is_in_module(void* address, const char* module)
{
	const auto base = reinterpret_cast<std::uintptr_t>(GetModuleHandleA(module));
	if (!base)
		return false;
	MODULEINFO info{};
	if (!GetModuleInformation(GetCurrentProcess(), reinterpret_cast<HMODULE>(base), &info, sizeof(info)))
		return false;
	const auto target = reinterpret_cast<std::uintptr_t>(address);
	return target >= base && target < base + info.SizeOfImage && g_utils.is_executable_address(target);
}

bool replace_vtable_entry(void** table, std::size_t index, void* replacement)
{
	DWORD old_protection{};
	if (!VirtualProtect(&table[index], sizeof(void*), PAGE_EXECUTE_READWRITE, &old_protection))
		return false;
	table[index] = replacement;
	DWORD ignored{};
	VirtualProtect(&table[index], sizeof(void*), old_protection, &ignored);
	FlushInstructionCache(GetCurrentProcess(), &table[index], sizeof(void*));
	return true;
}
}

bool Hooks::Initialize() 
{
	if (!Interfaces::clientmode || !Interfaces::panel || !Interfaces::surface)
		return false;

	const auto CreateMove_target = get_virtual<void*>(Interfaces::clientmode, Indices::create_move_idx);
	const auto PaintTraverse_target = get_virtual<void*>(Interfaces::panel, Indices::paint_traverse_idx);
	const auto LockCursor_target = get_virtual<void*>(Interfaces::surface, Indices::lock_cursor);

	if (!is_in_module(CreateMove_target, "client.dll") || !is_in_module(PaintTraverse_target, "vgui2.dll") || !is_in_module(LockCursor_target, "vguimatsurface.dll"))
		return false;

	create_move_vtable = *reinterpret_cast<void***>(Interfaces::clientmode);
	create_move_target = CreateMove_target;
	CreateMove::CreateMove_original = reinterpret_cast<CreateMove::CreateMove_fn>(CreateMove_target);
	if (!replace_vtable_entry(create_move_vtable, Indices::create_move_idx, reinterpret_cast<void*>(&CreateMove::Hook))) {
		create_move_vtable = nullptr;
		create_move_target = nullptr;
		return false;
	}

	paint_traverse_vtable = *reinterpret_cast<void***>(Interfaces::panel);
	paint_traverse_target = PaintTraverse_target;
	PaintTraverse::PaintTraverse_original = reinterpret_cast<PaintTraverse::PaintTraverse_fn>(PaintTraverse_target);
	if (!replace_vtable_entry(paint_traverse_vtable, Indices::paint_traverse_idx, reinterpret_cast<void*>(&PaintTraverse::Hook))) {
		replace_vtable_entry(create_move_vtable, Indices::create_move_idx, create_move_target);
		create_move_vtable = nullptr;
		create_move_target = nullptr;
		paint_traverse_vtable = nullptr;
		paint_traverse_target = nullptr;
		return false;
	}

	lock_cursor_vtable = *reinterpret_cast<void***>(Interfaces::surface);
	lock_cursor_target = LockCursor_target;
	LockCursor::LockCursor_original = reinterpret_cast<LockCursor::LockCursor_fn>(LockCursor_target);
	if (!replace_vtable_entry(lock_cursor_vtable, Indices::lock_cursor, reinterpret_cast<void*>(&LockCursor::Hook))) {
		replace_vtable_entry(create_move_vtable, Indices::create_move_idx, create_move_target);
		replace_vtable_entry(paint_traverse_vtable, Indices::paint_traverse_idx, paint_traverse_target);
		create_move_vtable = nullptr;
		create_move_target = nullptr;
		paint_traverse_vtable = nullptr;
		paint_traverse_target = nullptr;
		lock_cursor_vtable = nullptr;
		lock_cursor_target = nullptr;
		return false;
	}

	return true;
}

void Hooks::Release() {
	if (create_move_vtable && create_move_target) {
		replace_vtable_entry(create_move_vtable, Indices::create_move_idx, create_move_target);
		create_move_vtable = nullptr;
		create_move_target = nullptr;
	}
	if (paint_traverse_vtable && paint_traverse_target) {
		replace_vtable_entry(paint_traverse_vtable, Indices::paint_traverse_idx, paint_traverse_target);
		paint_traverse_vtable = nullptr;
		paint_traverse_target = nullptr;
	}
	if (lock_cursor_vtable && lock_cursor_target) {
		replace_vtable_entry(lock_cursor_vtable, Indices::lock_cursor, lock_cursor_target);
		lock_cursor_vtable = nullptr;
		lock_cursor_target = nullptr;
	}
}
