#pragma once
#include "VelocityGraph.h"

#include "../History.h"
#include "../AnalyzerEnums.h"
#include "../DrawUtil.h"

#include "../../sdk+/sdk.h"
#include "../../zgui/menu.h"
#include "../../sdk+/drawing.h"
#include "../../sdk+/color.h"

namespace VelocityGraph
{
    /// <summary>
    /// Draws the players velocity graph
    /// </summary>
    void Paint()
    {
        if (!g_menu.velocitygraph.enabled) return;

        std::vector<TickData> ticks;
        {
            std::lock_guard<std::mutex> lock(History::mutex);
            ticks = History::tickHist;
        }
        if (ticks.size() <= 1) return;

        const int DATA_WIDTH = g_menu.strafetrainer.dataWidth;
        const int HISTORY_SIZE = static_cast<int>(ticks.size());
        int X_OFF = g_menu.strafetrainer.xOffset;
        const int Y_OFF = g_menu.strafetrainer.yOffset;

        int screenWidth, screenHeight;
        Interfaces::engine->get_screen_size(screenWidth, screenHeight);

        int xCenter = (screenWidth / 2);

        if (g_menu.velocitygraph.centered)
        {
            X_OFF = xCenter - ((HISTORY_SIZE * DATA_WIDTH) / 2);
        }

		constexpr float MAX_SPEED = 3500.0f;
		const float pixels_per_unit = (screenHeight * 0.5f) / MAX_SPEED;
        DrawUtil::PaintLineGraph(ticks.begin(), ticks.end(),
            X_OFF, Y_OFF, 13.0f, true, g_menu.colors.perf, 3,
			[pixels_per_unit](const auto& tick) { return tick.speed * pixels_per_unit; });
    }
}
