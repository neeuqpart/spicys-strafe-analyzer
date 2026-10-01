#pragma once
#include "RouteTool.h"

#include "../AnalyzerInlines.h"
#include "../History.h"
#include "../../zgui/Menu.h"

#include "../../SDK+/Drawing.h"
#include "../../SDK+/Color.h"

namespace RouteTool
{
	/// <summary>
	/// Handle updating recorded run
	/// </summary>
	void Update()
	{
		// Capture input if in menu
		if (g_menu.routetool.menu.enabled)
		{
			if (!g_menu.routetool.recording)
			{
				// F1 enable recording
				if (GetAsyncKeyState(VK_F1) & 1)
				{
					g_menu.routetool.recording = true;
					g_menu.routetool.menu.menuText = "Started recording!";
				}
			}
			else
			{
				// F2 disable recording
				if (GetAsyncKeyState(VK_F2) & 1)
				{
					g_menu.routetool.recording = false;
					g_menu.routetool.hasRunRecorded = true;
					g_menu.routetool.menu.menuText = "Stopped recording!";

					std::lock_guard<std::mutex> lock(History::mutex);
					if (History::recordedRun.size() > 1)
					{
						for (size_t i = 0; i < History::recordedRun.size() - 1; i++)
						{
							if (History::recordedRun[i].vecVel.z != History::recordedRun[0].vecVel.z)
							{
								g_menu.routetool.firstTickJumped = static_cast<int>(i);
								break;
							}
						}
					}
				}
			}
			// F3 clear run
			if (GetAsyncKeyState(VK_F3) & 1)
			{
				std::lock_guard<std::mutex> lock(History::mutex);
				History::recordedRun.clear();
				g_menu.routetool.hasRunRecorded = false;
				g_menu.routetool.menu.menuText = "Cleared recorded run!";
			}
		}

		if (g_menu.routetool.recording)
		{
			std::lock_guard<std::mutex> lock(History::mutex);
			if (History::tickHist.empty())
				return;
			const int CUR_TICK = static_cast<int>(History::tickHist.size()) - 1;
			History::recordedRun.push_back(History::tickHist[CUR_TICK]);
		}
	}

	color GetPathTickColor(const std::vector<TickData>& run, size_t index)
	{
		if (run[index].posType == PositionType::GROUND)
		{
			return g_menu.colors.pathOnGround;
		}
		else if (run[index].crouching)
		{
			return g_menu.colors.pathOnCrouch;
		}
		else if (g_menu.routetool.showSpeedLoss && run[index].lostSpeed)
		{
			return g_menu.colors.pathLostSpeed;
		}
		else return g_menu.colors.pathDefault;
	}

	void Paint()
	{
		if (g_menu.routetool.draw.enabled && player)
		{
			std::vector<TickData> run;
			{
				std::lock_guard<std::mutex> lock(History::mutex);
				run = History::recordedRun;
			}

			const vec3_t playerOrigin = player->origin();

			if (g_menu.routetool.draw.pathType == (int)RouteToolGraphType::GRAPH_LINE)
			{
				if (run.size() > 2)
				{
					for (size_t i = 0; i < run.size() - 2; i++)
					{
						double diffDist = fabs(std::hypot(run[i].vecPos.x - playerOrigin.x,
							run[i].vecPos.y - playerOrigin.y,
							run[i].vecPos.z - playerOrigin.z));

						if (diffDist < g_menu.routetool.drawDistance)
						{
							vec3_t screen, screen2;
							if (worldToScreen(run[i].vecPos, screen))
							{
								if (worldToScreen(run[i + 1].vecPos, screen2))
								{
									color tempColor = GetPathTickColor(run, i);
									render::line(screen.x, screen.y, screen2.x, screen2.y, tempColor);
								}
							}
						}
					}
				}
			}
			else if (g_menu.routetool.draw.pathType == (int)RouteToolGraphType::GRAPH_DOT)
			{
				if (run.size() > 1)
				{
					for (size_t i = 0; i < run.size() - 1; i++)
					{
						double diffDist = fabs(std::hypot(run[i].vecPos.x - playerOrigin.x,
							run[i].vecPos.y - playerOrigin.y,
							run[i].vecPos.z - playerOrigin.z));

						if (diffDist < g_menu.routetool.drawDistance)
						{
							vec3_t screen;
							if (worldToScreen(run[i].vecPos, screen))
							{
								color tempColor = GetPathTickColor(run, i);
								render::filled_rect(screen.x, screen.y, g_menu.routetool.lineSize, g_menu.routetool.lineSize, tempColor);
							}
						}
					}
				}
			}
		}
		if (g_menu.routetool.menu.enabled)
		{
			// Draw background
			render::filled_rect(30, 30, 145, 100, g_menu.colors.bg);
			// Draw text
			render::s_text(50, 40, color::white(255), render::moon_font, false, "F1 - Start recording");
			render::s_text(50, 60, color::white(255), render::moon_font, false, "F2 - Stop recording");
			render::s_text(50, 80, color::white(255), render::moon_font, false, "F3 - Clear recording");
			render::s_text(50, 100, color::green(255), render::moon_font, false, g_menu.routetool.menu.menuText);
		}
	}
}
