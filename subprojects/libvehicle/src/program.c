#include <constellation/common.h>
#include <stdlib.h>
#include "program-private.h"

static bool launch_is_done(ConstellationVehicle* vehicle, ConstellationProgramData data, time_t elapsed_time) {
	return constellation_vehicle_get_alt(vehicle) >= data.launch.altitude;
}

static void launch_exec(ConstellationVehicle* vehicle, ConstellationProgramData data, time_t elapsed_time) {
	constellation_vehicle_throttle(vehicle, 1.0);
}

static bool timer_is_done(ConstellationVehicle* vehicle, ConstellationProgramData data, time_t elapsed_time) {
	return elapsed_time >= data.timer.interval;
}

static struct {
	uint8_t prog;
	bool (*is_done)(ConstellationVehicle* vehicle, ConstellationProgramData data, time_t elapsed_time);
	void (*exec)(ConstellationVehicle* vehicle, ConstellationProgramData data, time_t elapsed_time);
} programs[] = {
	{ CONSTELLATION_PACKET_PRG_LAUNCH, launch_is_done, launch_exec },
	{ CONSTELLATION_PACKET_PRG_TIMER, timer_is_done, NULL }
};

struct program_queue* constellation_vehicle_execute_program(ConstellationVehicle* vehicle, struct program_queue* prog) {
	if (prog != NULL) {
		struct program_queue* prog_next = prog->next;
		if (prog->executor != pthread_self()) return prog_next;

		bool done = false;
		bool found = false;

		size_t n = sizeof (programs) / sizeof (programs[0]);
		for (size_t i = 0; i < n; i++) {
			if (programs[i].prog == prog->prog) {
				if (prog->time == 0) prog->time = time(NULL);
				time_t elapsed_time = time(NULL) - prog->time;

				done = programs[i].is_done(vehicle, prog->data, elapsed_time);
				found = true;

				if (!done && programs[i].exec != NULL) {
					programs[i].exec(vehicle, prog->data, elapsed_time);
				}
				break;
			}
		}

		if (done || !found) free(prog);
		return (done || !found) ? prog_next : prog;
	}
	return NULL;
}