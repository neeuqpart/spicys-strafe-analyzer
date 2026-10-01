#pragma once
#include "../Interfaces/Interfaces.h"
#include "../minhook/minhook.h"
#include "../sdk+/Entity.h"
#include "../sdk+/sdk.h"
#include <atomic>
#include <cstdint>

namespace Hooks 
{
	// Detours can run on the game threads as soon as MinHook enables them.  Do
	// not touch analyzer state until the loader thread has finished setting up
	// fonts and rendering callbacks.
	inline std::atomic_bool runtime_ready{ false };
	inline void** create_move_vtable = nullptr;
	inline void* create_move_target = nullptr;
	inline void** paint_traverse_vtable = nullptr;
	inline void* paint_traverse_target = nullptr;
	inline void** lock_cursor_vtable = nullptr;
	inline void* lock_cursor_target = nullptr;

	bool Initialize();
	void Release();

	namespace CreateMove 
	{
		using CreateMove_fn = bool(*)(IClientMode*, float, CUserCmd*);
		inline CreateMove_fn CreateMove_original;
		bool Hook(IClientMode* self, float input_sample_frame_time, CUserCmd* cmd);
	};

	namespace PaintTraverse
	{
		using PaintTraverse_fn = void(*)(IPanel*, std::uintptr_t, bool, bool);
		inline PaintTraverse_fn PaintTraverse_original;
		void Hook(IPanel* self, std::uintptr_t panel, bool force_repaint, bool allow_force);
	};

	namespace LockCursor 
	{
		using LockCursor_fn = void(*)(ISurface*);
		inline LockCursor_fn LockCursor_original;
		void Hook(ISurface* self);
	}
}
