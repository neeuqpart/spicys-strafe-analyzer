#include "Strafetrainer.h"
#include "../AnalyzerInlines.h"
#include "../DrawUtil.h"

#include "../../zgui/Menu.h"
#include "../../SDK+/SDK.h"

namespace StrafeTrainer
{
    /// <summary>
    /// Draws a graph onto the players screen using the History vector
    /// </summary>
    void Paint()
    {
        if (!g_menu.strafetrainer.enabled) return;

        std::vector<TickData> ticks;
        {
            std::lock_guard<std::mutex> lock(History::mutex);
            ticks = History::tickHist;
        }
        if (ticks.size() <= 1) return;

        const int DATA_WIDTH = g_menu.strafetrainer.dataWidth;
        const int HISTORY_SIZE = static_cast<int>(ticks.size());
        const float MAX_ANGLE = 20.0f;

        int screenWidth, screenHeight;
        Interfaces::engine->get_screen_size(screenWidth, screenHeight);

        // Find the correct graph Offset based on menuoptions
        int xOffset = g_menu.strafetrainer.xOffset;
        int yOffset = g_menu.strafetrainer.yOffset;

        int xCenter = (screenWidth / 2);

        if (g_menu.strafetrainer.centered)
        {
            xOffset = xCenter - ((HISTORY_SIZE * DATA_WIDTH) / 2);
        }

        StrafeTrainerGraphType graph = static_cast<StrafeTrainerGraphType>(g_menu.strafetrainer.graphType);
        if (graph == StrafeTrainerGraphType::GRAPH_FILLED)
        {
            // Airborne targets form the background; player strafes stay on top.
            DrawUtil::PaintFilledGraph(ticks.begin(), ticks.end(),
                xOffset, yOffset, MAX_ANGLE, false, zColorToColor(g_menu.strafetrainer.colors.perfColor),
                [MAX_ANGLE](const auto& i) {
                    return i.posType == PositionType::AIR
                        ? (std::min)(i.perfDeltaYaw, static_cast<double>(MAX_ANGLE)) : -1.0;
                });

            DrawUtil::PaintFilledGraph(ticks.begin(), ticks.end(),
                xOffset, yOffset, MAX_ANGLE, true, zColorToColor(g_menu.strafetrainer.colors.synced),
                [](const auto& i) { return i.deltaYaw; });
        }
        else if (graph == StrafeTrainerGraphType::GRAPH_LINE)
        {
            // and deltaYaw
            DrawUtil::PaintLineGraph(ticks.begin(), ticks.end(), xOffset, yOffset, MAX_ANGLE,
                true, zColorToColor(g_menu.strafetrainer.colors.synced), g_menu.strafetrainer.lineSize,
                [](const auto& i) { return i.deltaYaw; });
            // Draw the perfect guide last so matching yaw cannot erase it.
            DrawUtil::PaintLineGraph(ticks.begin(), ticks.end(), xOffset, yOffset, MAX_ANGLE,
                false, zColorToColor(g_menu.strafetrainer.colors.perfColor), g_menu.strafetrainer.lineSize,
                [MAX_ANGLE](const auto& i) { return (std::min)(i.perfDeltaYaw, static_cast<double>(MAX_ANGLE)); });
        }
        else if (graph == StrafeTrainerGraphType::GRAPH_HORIZONTAL)
        {
            int yOff = (std::min)(yOffset, (std::max)(0, screenHeight - 60));
            DrawUtil::PaintHorizontalGraph(ticks.data(), static_cast<int>(ticks.size()), &TickData::perfDeltaYaw, &TickData::deltaYaw,
                                            500, xCenter, yOff, true, color::white(200));
        }
        else if (graph == StrafeTrainerGraphType::GRAPH_VERTICAL)
        {
            int xOff = (screenWidth / 2) - 20;
            int yOff = (std::min)(yOffset, (std::max)(0, screenHeight - 510));
            DrawUtil::PaintVerticalGraph(ticks.data(), static_cast<int>(ticks.size()), &TickData::perfDeltaYaw, &TickData::deltaYaw,
                                        500, 6, xOff, yOff, true, color::white(200));
        }
    }
}
