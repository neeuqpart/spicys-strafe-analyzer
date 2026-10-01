#include "Interfaces.h"
#include "../SDK+/SDK.h"
#include "../SDK+/Utils.hpp"

namespace {
// Validated against Counter-Strike: Source x64 client.dll dated 2026-09-16.
// The first signature anchors the ClientModeCSNormal singleton-return path;
// the second resolves the CGlobalVarsBase pointer retained by VClient017::Init.
constexpr char kClientModePattern[] =
	"48 8D 1D ? ? ? ? 48 8B CB 89 05 ? ? ? ? E8 ? ? ? ? "
	"48 8B 0D ? ? ? ? 48 8D 05 ? ? ? ? 48 89 05 ? ? ? ?";
constexpr char kGlobalVarsPattern[] = "48 89 1D ? ? ? ? 48 8D 4D 18";

bool readable_address(const void* address, std::size_t size)
{
	MEMORY_BASIC_INFORMATION information{};
	if (!address || VirtualQuery(address, &information, sizeof(information)) == 0)
		return false;
	return information.State == MEM_COMMIT &&
		information.Protect != PAGE_NOACCESS &&
		information.RegionSize >= size;
}

IClientMode* resolve_client_mode()
{
	const auto match = g_utils.find_signature("client.dll", kClientModePattern);
	if (!match)
		return nullptr;
	// The match loads the ClientMode vtable, then writes it into the static
	// CClientModeCSNormal object. Resolve that following RIP-relative store.
	const auto object = g_utils.resolve_rel32(match, 38, 42);
	if (!readable_address(reinterpret_cast<void*>(object), sizeof(void*)))
		return nullptr;
	const auto vtable = *reinterpret_cast<void***>(object);
	if (!vtable || !g_utils.is_executable_address(reinterpret_cast<std::uintptr_t>(vtable[Indices::create_move_idx])))
		return nullptr;
	return reinterpret_cast<IClientMode*>(object);
}

CGlobalVars* resolve_global_vars()
{
	const auto match = g_utils.find_signature("client.dll", kGlobalVarsPattern);
	if (!match)
		return nullptr;
	const auto storage = g_utils.resolve_rel32(match, 3, 7);
	if (!readable_address(reinterpret_cast<void*>(storage), sizeof(void*)))
		return nullptr;
	auto* value = *reinterpret_cast<CGlobalVars**>(storage);
	if (!readable_address(value, sizeof(CGlobalVars)) || value->interval_per_tick <= 0.0f || value->interval_per_tick > 0.1f)
		return nullptr;
	return value;
}

ConVar* resolve_air_accelerate()
{
	// Resolve the registered variable by name; the old broad constructor
	// signature could select an unrelated ConVar (and its fallback was stale).
	return Interfaces::convar ? Interfaces::convar->get_convar("sv_airaccelerate") : nullptr;
}
}

IBaseClientDLL* Interfaces::client = nullptr;
IVEngineClient* Interfaces::engine = nullptr;
IClientEntityList* Interfaces::ent_list = nullptr;
IPanel* Interfaces::panel = nullptr;
ISurface* Interfaces::surface = nullptr;
IClientMode* Interfaces::clientmode = nullptr;
CGlobalVars* Interfaces::globals = nullptr;
IConVar* Interfaces::convar = nullptr;
ConVar* Interfaces::sv_airaccelerate = nullptr;
IInputSystem* Interfaces::input = nullptr;
IVDebugOverlay* Interfaces::debugoverlay = nullptr;

bool Interfaces::Initialize()
{
	static_assert(sizeof(void*) == 8, "CSS analyzer requires x64");
	client = get_interface<IBaseClientDLL>("client.dll", "VClient017");
	ent_list = get_interface<IClientEntityList>("client.dll", "VClientEntityList003");
	engine = get_interface<IVEngineClient>("engine.dll", "VEngineClient014");
	panel = get_interface<IPanel>("vgui2.dll", "VGUI_Panel009");
	surface = get_interface<ISurface>("vguimatsurface.dll", "VGUI_Surface030");
	convar = get_interface<IConVar>("vstdlib.dll", "VEngineCvar004");
	input = get_interface<IInputSystem>("inputsystem.dll", "InputSystemVersion001");

	if (!client || !ent_list || !engine || !panel || !surface || !input)
		return false;

	clientmode = resolve_client_mode();
	globals = resolve_global_vars();
	sv_airaccelerate = resolve_air_accelerate();

	return clientmode && globals;
}
