#pragma once
#include <cstddef>
#include <string>
#include <Windows.h>

#include "IBaseClientDLL.h"
#include "IClientEntityList.h"
#include "IVEngineClient.h"
#include "IPanel.h"
#include "ISurface.h"
#include "IConVar.h"
#include "IInputSystem.h"
#include "IVDebugOverlay.h"
#include "CUserCmd.h"
#include "CGlobalVars.h"


namespace Interfaces 
{
	template< typename T >
	T* get_interface(const char* module, const char* name, bool print = false)
	{
		using CreateInterfaceFn = void* (*)(const char*, int*);
		auto module_handle = GetModuleHandleA(module);
		if (!module_handle)
			return nullptr;
		auto factory = reinterpret_cast<CreateInterfaceFn>(GetProcAddress(module_handle, "CreateInterface"));
		if (!factory)
			return nullptr;
		auto* result = static_cast<T*>(factory(name, nullptr));
		if (print)
			printf(" -- %s %s\n", name, result ? "OK" : "missing");
		return result;
	}

	extern IBaseClientDLL* client;
	extern IVEngineClient* engine;
	extern IClientEntityList* ent_list;
	extern IPanel* panel;
	extern ISurface* surface;
	extern IClientMode* clientmode;
	extern CGlobalVars* globals;
	extern IConVar* convar;
	extern ConVar* sv_airaccelerate;
	extern IInputSystem* input;
	extern IVDebugOverlay* debugoverlay;

	bool Initialize();
}
