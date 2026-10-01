#include "../sdk+/console.h"
#include "hooks.h"

#include "../Analyzer/AnalyzerMath.h"
#include "../Analyzer/AnalyzerInlines.h"

#include "../Analyzer/Features/TickHistory.h"
#include "../Analyzer/Features/SyncTrainer.h"
#include "../Analyzer/Features/ScrollGraph.h"
#include "../Analyzer/Features/RouteTool.h"
#include "../Analyzer/Features/VelocityGraph.h"

#include "../zgui/menu.h"
#include "../zgui/zgui.hh"

void UpdateMenuColorsConverted()
{
	g_menu.colors.perf = zColorToColor(g_menu.strafetrainer.colors.perfColor);
	g_menu.colors.synced = zColorToColor(g_menu.strafetrainer.colors.synced);
	g_menu.colors.notSynced = zColorToColor(g_menu.strafetrainer.colors.notSynced);
	g_menu.colors.lostSpeed = zColorToColor(g_menu.strafetrainer.colors.lostSpeed);
	g_menu.colors.counterStrafe = zColorToColor(g_menu.strafetrainer.colors.counterStrafe);

	g_menu.colors.left = zColorToColor(g_menu.synctrainer.colors.leftDir);
	g_menu.colors.right = zColorToColor(g_menu.synctrainer.colors.rightDir);

	g_menu.colors.pathDefault = zColorToColor(g_menu.routetool.colors.pathDefault);
	g_menu.colors.pathLostSpeed = zColorToColor(g_menu.routetool.colors.pathLostSpeed);
	g_menu.colors.pathOnGround = zColorToColor(g_menu.routetool.colors.pathOnGround);
	g_menu.colors.pathOnCrouch = zColorToColor(g_menu.routetool.colors.pathOnCrouch);
}

bool Hooks::CreateMove::Hook(IClientMode* self, float input_sample_frame_time, CUserCmd* cmd) 
{
	const bool result = CreateMove_original(self, input_sample_frame_time, cmd);
	if (!Hooks::runtime_ready.load(std::memory_order_acquire))
		return result;

	if (!cmd || cmd->command_number <= 0)
		return result;

	if (!Interfaces::engine->is_connected() || !Interfaces::engine->is_in_game()) {
		std::lock_guard<std::mutex> lock(History::mutex);
		History::tickHist.clear();
		History::strafeHist.clear();
		return result;
	}

	const int local_player_index = Interfaces::engine->get_local_player();
	if (local_player_index <= 0)
		return result;
	player = reinterpret_cast<player_t*>(Interfaces::ent_list->get_client_entity(local_player_index));
	if (!player)
		return result;

	// Update colors
	UpdateMenuColorsConverted();

	if (menu_open)
	{
		cmd->buttons &= ~(in_attack | in_attack2);
	}

	if (!g_menu.strafetrainer.paused)
	{
		// Update tick history
		TickHistory::Update(cmd);
		SyncTrainer::Update();
		RouteTool::Update();
	}

	return result;
}
