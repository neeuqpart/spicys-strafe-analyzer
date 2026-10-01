//#include "../analyzer/config.h"
#include "../Interfaces/Interfaces.h"
#include "../sdk+/drawing.h"
#include "../sdk+/fnv1.h"
#include "../sdk+/utils.hpp"
#include "../zgui/zgui.hh"
#include "../zgui/menu.h"
#include "hooks.h"

#include "../sdk+/console.h"

#include "../Analyzer/Features/StrafeTrainer.h"
#include "../Analyzer/Features/SyncTrainer.h"
#include "../Analyzer/Features/ScrollGraph.h"
#include "../Analyzer/Features/RouteTool.h"
#include "../Analyzer/Features/VelocityGraph.h"

void Hooks::PaintTraverse::Hook(IPanel* self, std::uintptr_t panel, bool force_repaint, bool allow_force) 
{
	PaintTraverse_original(self, panel, force_repaint, allow_force);
	if (!Hooks::runtime_ready.load(std::memory_order_acquire))
		return;

	const char* panel_name = self->get_panel_name(panel);
	if (!panel_name)
		return;

	auto panel_to_draw = fnv::hash(panel_name);

	switch (panel_to_draw){
	case fnv::hash("MatSystemTopPanel"):
	{
		StrafeTrainer::Paint();
		SyncTrainer::Paint();
		ScrollGraph::Paint();
		VelocityGraph::Paint();
		RouteTool::Paint();
		break;
	}

	case fnv::hash("FocusOverlayPanel"):
	{
		g_menu.Render(game);
		break;
	}
	default:;
	}
}
