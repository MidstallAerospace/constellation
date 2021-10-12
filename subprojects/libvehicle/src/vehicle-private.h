#pragma once

#include <constellation/vehicle.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>

struct packet_queue {
	ConstellationPacket* pkt;
	struct packet_queue* next;
};

struct program_queue {
	uint8_t prog;
	ConstellationProgramData data;
	pthread_t executor;
	time_t time;
	struct program_queue* next;
};

struct _ConstellationVehicle {
	ConstellationVehicleFuncs funcs;

	ConstellationEvent* ev_stage;
	ConstellationEvent* ev_ignite;
	ConstellationEvent* ev_abort;

	void* priv;

	struct packet_queue* send_queue;
	uint32_t recv_failed_count;
	uint32_t total_transmit;
	uint32_t total_recv;

	pthread_t thread_recv;
	pthread_t thread_transmit;
	pthread_t thread_prog;

	bool ignite:1;
	time_t ignite_time;

	struct program_queue* prog_queue;

	ConstellationStageConfiguration curr_stage;

	uint8_t num_stages;
	uint8_t stage;
	double start_pos[3];

	double vel;
	double alt;
	double throttle;
};