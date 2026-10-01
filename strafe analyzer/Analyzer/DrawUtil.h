#pragma once
#include "AnalyzerEnums.h"
#include "AnalyzerMath.h"
#include "AnalyzerInlines.h"

#include "../SDK+/Drawing.h"
#include "../SDK+/SDK.h"
#include "../SDK+/color.h"

#include "../zgui/zgui.hh"
#include "../zgui/Menu.h"

namespace DrawUtil
{

#pragma region Coloring
    /// <summary>
    /// Changes colors if the player has lost speed, or if the player has been on 
    /// the ground for at least 2 concurrent ticks
    /// else, it uses the players current Sync State
    /// </summary>
    inline color GetTickColor(int index)
    {
        color curStateColor = { 0, 0, 0 };

        // Player has been on the ground for at least 2 ticks in a row (current)
        if (HasBeenOnGround(index))
        {
            curStateColor = zColorToColor(g_menu.strafetrainer.colors.synced);
        }
        // Lostspeed
        else if (History::tickHist[index].lostSpeed)
        {
            curStateColor = zColorToColor(g_menu.strafetrainer.colors.lostSpeed);
        }
        else
        {
            switch (History::tickHist[index].syncState)
            {
            case SyncType::COUNTERSTRAFE:
                curStateColor = zColorToColor(g_menu.strafetrainer.colors.counterStrafe);
                break;
            case SyncType::SYNCED:
                curStateColor = zColorToColor(g_menu.strafetrainer.colors.synced);
                break;
            case SyncType::NOT_SYNCED:
                curStateColor = zColorToColor(g_menu.strafetrainer.colors.notSynced);
                break;
            }
        }
        return curStateColor;
    }
#pragma endregion

#pragma region Graphs

    template <class IT, class F>
    inline void PaintFilledGraph(IT beginning, IT end, int xOffset, int yOffset,
        double maxAngle, bool colorWithSync, color defaultColor, F functor)
    {
        const int DATA_WIDTH = g_menu.strafetrainer.dataWidth;
        const int DATA_HEIGHT = g_menu.strafetrainer.dataHeight;

        // Create temp color
        color curStateColor{ 0, 0, 0 };
        int endI = std::distance(beginning, end);

        // Create temp iterator and set to the beginning
        IT itIndex = beginning;

        while (itIndex != end)
        {
            int xPos, yPos;
            int i = std::distance(beginning, itIndex);

            if (g_menu.strafetrainer.invertDir) xPos = i * DATA_WIDTH;
            else xPos = (endI - i) * DATA_WIDTH;
            const double value = functor(*itIndex);
            yPos = std::isfinite(value) && value > 0.0
                ? (std::max)(1, static_cast<int>(std::ceil((std::min)(value, maxAngle) * DATA_HEIGHT))) : 0;

            if (std::isfinite(value) && value >= 0.0 && value <= maxAngle)
            {
                if (!colorWithSync) curStateColor = defaultColor;
                else curStateColor = GetTickColor(i);

                render::filled_rect(xOffset + xPos, yOffset - yPos,
                    DATA_WIDTH, yPos, curStateColor);
            }

            itIndex++;
        }
    }


    template <class IT, class F>
    inline void PaintLineGraph(IT beginning, IT end, int xOffset, int yOffset, 
        double maxAngle, bool colorWithSync, color defaultColor, int linePaddingCount, F functor)
    {
        const int DATA_WIDTH = g_menu.strafetrainer.dataWidth;
        const int DATA_HEIGHT = g_menu.strafetrainer.dataHeight;

        // Create temp color
        color curStateColor{ 0, 0, 0 };

        // Create temp iterator and set to the beginning
        IT itIndex = beginning;
        if (beginning == end) return;
        IT nextIndex = std::next(itIndex, 1);

        int endI = std::distance(beginning, end);

        while (nextIndex != end)
        {
            int xPos1, xPos2;
            int i = std::distance(beginning, itIndex);

            if (g_menu.strafetrainer.invertDir)
            {
                xPos1 = i * DATA_WIDTH;
                xPos2 = (i + 1) * DATA_WIDTH;
            }
            else
            {
                xPos1 = (endI - i) * DATA_WIDTH;
                xPos2 = (endI - (i + 1)) * DATA_WIDTH;
            }

            const double value1 = functor(*itIndex);
            const double value2 = functor(*nextIndex);
            int yPos1 = std::isfinite(value1) && value1 > 0.0
                ? (std::max)(1, static_cast<int>(std::ceil((std::min)(value1, maxAngle) * DATA_HEIGHT))) : 0;
            int yPos2 = std::isfinite(value2) && value2 > 0.0
                ? (std::max)(1, static_cast<int>(std::ceil((std::min)(value2, maxAngle) * DATA_HEIGHT))) : 0;

            if (std::isfinite(value1) && std::isfinite(value2) &&
                value1 >= 0.0 && value2 >= 0.0 && value1 <= maxAngle && value2 <= maxAngle)
            {
                if (!colorWithSync) curStateColor = defaultColor;
                else curStateColor = GetTickColor(i);

                // Padding lines
                for (int j = 0; j < linePaddingCount; j++)
                {
                    // Expand target guides upwards so a small positive angle
                    // stays above the baseline even with a thick line.
                    const int padding = colorWithSync ? j : -j;
                    render::line(xOffset + xPos1, yOffset - yPos1 + padding,
                        xOffset + xPos2, yOffset - yPos2 + padding, curStateColor);
                }
            }

            itIndex++;
            nextIndex++;
        }
    }


    template <class T, class T2, class S>
    inline void PaintHorizontalGraph(T* vecPtr, int totalTicks, T2 S::* targetElem1Ptr, T2 S::* targetElem2Ptr,
        int desiredPixels, int xCenter, int yOffset, bool colorWithSync, color borderColor)
    {
        if (totalTicks <= 0) return;
        const int CUR_TICK = totalTicks - 1;
        color curStateColor = { 0,0,0 };

        double maxAngle = vecPtr[CUR_TICK].*targetElem1Ptr * 2;
        if (maxAngle <= 0.0001) maxAngle = 2.36;
        if (desiredPixels <= 0) return;
        const double anglePerPixel = maxAngle / desiredPixels;
        double viewangleX = min(vecPtr[CUR_TICK].*targetElem2Ptr, maxAngle) / anglePerPixel;

        if (!colorWithSync) curStateColor = zColorToColor(g_menu.strafetrainer.colors.notSynced);
        else curStateColor = GetTickColor(CUR_TICK);

        const int HEIGHT = 50;

        // Draw outer border
        render::rect(xCenter - (desiredPixels / 2) - 1, yOffset - 1, desiredPixels + 5, HEIGHT + 2, borderColor);

        // Draw Viewangle
        render::filled_rect(xCenter - (desiredPixels / 2) + static_cast<int>(viewangleX), yOffset, g_menu.strafetrainer.lineSize, HEIGHT, curStateColor);

        // Paint the guide last, extending past the player marker so it remains
        // visible even when the player matches the target exactly.
        render::filled_rect(xCenter, yOffset - 4, g_menu.strafetrainer.lineSize, HEIGHT + 8,
            zColorToColor(g_menu.strafetrainer.colors.perfColor));
    }


    template <class T, class T2, class S>
    inline void PaintVerticalGraph(T* vecPtr, int totalTicks, T2 S::* targetElem1Ptr, T2 S::* targetElem2Ptr,
        int desiredPixels, double maxAngle, int xOffset, int yOffset, bool colorWithSync, color borderColor)
    {
        if (totalTicks <= 0) return;
        const int CUR_TICK = totalTicks - 1;
        color curStateColor = { 0, 0, 0 };

        if (maxAngle <= 0.0001 || desiredPixels <= 0) return;

        // Find the ratio of deltaYaw to perfangle
        const double anglePerPixel = maxAngle / desiredPixels;

        // targetElem1Ptr is perfDeltaYaw, targetElem2Ptr is deltaYaw
        double yPosPerf = min(min(vecPtr[CUR_TICK].*targetElem1Ptr, maxAngle) / anglePerPixel, static_cast<double>(desiredPixels));
        double yPosDelta = min(min(vecPtr[CUR_TICK].*targetElem2Ptr, maxAngle) / anglePerPixel, static_cast<double>(desiredPixels));

        if (!colorWithSync) curStateColor = zColorToColor(g_menu.strafetrainer.colors.synced);
        else curStateColor = GetTickColor(CUR_TICK);

        const int WIDTH = 40;
        // Draw outer rectangle
        render::rect(xOffset - 1, yOffset - 1, WIDTH + 2, desiredPixels + 5, borderColor);

        // Draw player viewangle deltaYaw
        render::filled_rect(xOffset, yOffset + desiredPixels - static_cast<int>(yPosDelta), WIDTH, g_menu.strafetrainer.lineSize, curStateColor);

        // Keep the target visible over an overlapping player marker.
        render::filled_rect(xOffset - 4, yOffset + desiredPixels - static_cast<int>(yPosPerf), WIDTH + 8,
            g_menu.strafetrainer.lineSize, zColorToColor(g_menu.strafetrainer.colors.perfColor));
    }
#pragma endregion
}
