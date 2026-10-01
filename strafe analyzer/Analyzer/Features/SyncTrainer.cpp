#include "SyncTrainer.h"

#include "../AnalyzerEnums.h"
#include "../AnalyzerInlines.h"
#include "../AnalyzerMath.h"
#include "../History.h"

#include "../../sdk+/sdk.h"
#include "../../zgui/menu.h"
#include "../../sdk+/drawing.h"
#include "../../sdk+/color.h"
#include "../../sdk+/console.h"

#pragma region Templates
template <typename T>
void pop_front(std::vector<T>& v)
{
	if (v.size() > 0)
		v.erase(v.begin());
}

template <typename T>
std::string to_string_with_precision(const T a_value, const int n = 6)
{
	std::ostringstream out;
	out.precision(n);
	out << std::fixed << a_value;
	return out.str();
}
#pragma endregion

namespace SyncTrainer
{
	void Update()
	{
		if (!g_menu.synctrainer.enabled) return;

		std::lock_guard<std::mutex> lock(History::mutex);
		if (History::tickHist.empty()) return;
		const int CUR_TICK_NUM = static_cast<int>(History::tickHist.size()) - 1;
		const TickData curTick = History::tickHist[CUR_TICK_NUM];

		if (curTick.posType == PositionType::AIR && curTick.latencyAmount != 999999)
		{
			StrafeData newStrafe;
			newStrafe.strafeNum = curTick.curStrafes;
			newStrafe.jumpNum = curTick.curJumps;
			newStrafe.latencyAmount = curTick.latencyAmount;

			History::strafeHist.push_back(newStrafe);

			while (History::strafeHist.size() > static_cast<std::size_t>(g_menu.synctrainer.strafeHistorySize))
				pop_front(History::strafeHist);
		}
	}

	void Paint()
	{
		if (!g_menu.synctrainer.enabled) return;

		int screenWidth, screenHeight;
		Interfaces::engine->get_screen_size(screenWidth, screenHeight);

		const int xPos = static_cast<int>(0.75 * screenWidth);

		std::vector<StrafeData> strafes;
		{
			std::lock_guard<std::mutex> lock(History::mutex);
			strafes = History::strafeHist;
		}

		if (!strafes.empty())
		{
			for (size_t i = 0; i < strafes.size(); i++)
			{
				color tempColor;
				if (strafes[i].jumpNum % 2 == 0) tempColor = g_menu.colors.left;
				else tempColor = g_menu.colors.right;

				const int itemY = 40 + static_cast<int>(i * 25);

				if (strafes[i].latencyAmount > 0)
				{
					render::s_text(xPos, itemY, tempColor, render::moon_font, false, "Jump #"
						+ std::to_string(strafes[i].jumpNum) + " Strafe #" + std::to_string(strafes[i].strafeNum)
						+ " Late by " + to_string_with_precision(std::fabs(static_cast<double>(strafes[i].latencyAmount)), 0) + "t");
				}
				else if (strafes[i].latencyAmount < 0)
				{
					render::s_text(xPos, itemY, tempColor, render::moon_font, false, "Jump #"
						+ std::to_string(strafes[i].jumpNum) + " Strafe #" + std::to_string(strafes[i].strafeNum)
						+ " Early by " + to_string_with_precision(std::fabs(static_cast<double>(strafes[i].latencyAmount)), 0) + "t");
				}
				else if (strafes[i].latencyAmount == 0)
				{
					render::s_text(xPos, itemY, tempColor, render::moon_font, false, "Jump #"
						+ std::to_string(strafes[i].jumpNum) + " Strafe #"
						+ std::to_string(strafes[i].strafeNum) + " Perf!:) ");
				}
			}

			if (g_menu.synctrainer.statistics)
			{
				const int total = static_cast<int>(strafes.size());
				int lateCount = 0;
				int earlyCount = 0;
				int perfCount = 0;
				double sumLateLatency = 0.0;
				double sumEarlyLatency = 0.0;

				for (const auto& s : strafes)
				{
					if (s.latencyAmount > 0)
					{
						lateCount++;
						sumLateLatency += s.latencyAmount;
					}
					else if (s.latencyAmount < 0)
					{
						earlyCount++;
						sumEarlyLatency += std::fabs(static_cast<double>(s.latencyAmount));
					}
					else
					{
						perfCount++;
					}
				}

				const double avgLate = total > 0 ? (static_cast<double>(lateCount) / total) * 100.0 : 0.0;
				const double avgEarly = total > 0 ? (static_cast<double>(earlyCount) / total) * 100.0 : 0.0;
				const double avgPerf = total > 0 ? (static_cast<double>(perfCount) / total) * 100.0 : 0.0;
				const double avgLateLatency = lateCount > 0 ? (sumLateLatency / lateCount) : 0.0;
				const double avgEarlyLatency = earlyCount > 0 ? (sumEarlyLatency / earlyCount) : 0.0;

				const int statsY = 40 + (total * 25) + 8;

				render::s_text(xPos, statsY, color::white(255), render::moon_font, false,
					"Late:  " + to_string_with_precision(avgLate, 1) + "%");
				render::s_text(xPos, statsY + 22, color::white(255), render::moon_font, false,
					"Early: " + to_string_with_precision(avgEarly, 1) + "%");
				render::s_text(xPos, statsY + 44, color::white(255), render::moon_font, false,
					"Perf:  " + to_string_with_precision(avgPerf, 1) + "%");

				double clLateVal = std::isfinite(avgLateLatency) ? avgLateLatency : 0.0;
				double clEarlyVal = std::isfinite(avgEarlyLatency) ? avgEarlyLatency : 0.0;

				color colorLate = color(
					static_cast<int>(std::fmin(clLateVal * 30.0, 255.0)),
					static_cast<int>(std::fmax(255.0 - (clLateVal * 30.0), 0.0)),
					0, 255);
				color colorEarly = color(
					static_cast<int>(std::fmin(clEarlyVal * 30.0, 255.0)),
					static_cast<int>(std::fmax(255.0 - (clEarlyVal * 30.0), 0.0)),
					0, 255);

				render::s_text(xPos + 120, statsY, colorLate, render::moon_font, false,
					"Avg Latency: " + to_string_with_precision(clLateVal, 2) + "t");
				render::s_text(xPos + 120, statsY + 22, colorEarly, render::moon_font, false,
					"Avg Latency: " + to_string_with_precision(clEarlyVal, 2) + "t");
			}
		}
		else if (g_menu.synctrainer.statistics)
		{
			render::s_text(xPos, 40, color::white(180), render::moon_font, false, "Sync Trainer: waiting for strafes...");
		}
	}
}
