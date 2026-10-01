#pragma once
#include <cstdint>
class CGlobalVars {
public:
	float realtime;
	int framecount;
	float absolute_frametime;
	float curtime;
	float frametime;
	int max_clients;
	int tickcount;
	float interval_per_tick;

	float frame_time() const { return frametime; }
};
