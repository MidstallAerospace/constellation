#include <constellation/net/vehicle.h>
#include <constellation/common.h>
#include <constellation/vehicle.h>
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

struct packet_queue {
	ConstellationPacket* pkt;
	struct packet_queue* next;
};

struct _ConstellationVehicle {
	ConstellationVehicleFuncs funcs;

	ConstellationEvent* ev_stage;
	ConstellationEvent* ev_ignite;
	ConstellationEvent* ev_throttle;
	ConstellationEvent* ev_abort;

	void* priv;

	struct packet_queue* send_queue;
	uint32_t recv_failed_count;
	uint32_t total_transmit;
	uint32_t total_recv;

	pthread_t thread_recv;
	pthread_t thread_transmit;

	bool ignite:1;

	ConstellationStageConfiguration curr_stage;

	uint8_t num_stages;
	uint8_t stage;
	double start_pos[3];

	double vel;
	double alt;
	double throttle;
};

static void* transmit_thread(void* data) {
	ConstellationVehicle* vehicle = (ConstellationVehicle*)data;
	vehicle->total_transmit = 0;
	while (vehicle->ignite) vehicle->total_transmit = constellation_vehicle_handle_transmit(vehicle);
	return NULL;
}

static void* recv_thread(void* data) {
	ConstellationVehicle* vehicle = (ConstellationVehicle*)data;
	vehicle->total_recv = 0;
	while (vehicle->ignite) vehicle->total_recv = constellation_vehicle_handle_receive(vehicle);
	return NULL;
}

ConstellationVehicle* constellation_vehicle_new(ConstellationVehicleFuncs funcs, void* priv) {
	ConstellationVehicle* vehicle = (ConstellationVehicle*)malloc(sizeof(ConstellationVehicle));
	if (vehicle != NULL) {
		vehicle->priv = priv;

		vehicle->ev_stage = constellation_event_new(5);
		if (vehicle->ev_stage == NULL) {
			free(vehicle);
			return NULL;
		}

		vehicle->ev_ignite = constellation_event_new(5);
		if (vehicle->ev_ignite == NULL) {
			constellation_event_destroy(vehicle->ev_stage);
			free(vehicle);
			return NULL;
		}

		vehicle->ev_abort = constellation_event_new(5);
		if (vehicle->ev_abort == NULL) {
			constellation_event_destroy(vehicle->ev_stage);
			constellation_event_destroy(vehicle->ev_ignite);
			free(vehicle);
			return NULL;
		}

		vehicle->funcs = funcs;

		vehicle->send_queue = NULL;
		vehicle->recv_failed_count = 0;
		vehicle->total_transmit = 0;
		vehicle->total_recv = 0;

		vehicle->ignite = false;

		vehicle->curr_stage.mass = 0.0;
		vehicle->curr_stage.dry_mass = 0.0;
		vehicle->curr_stage.impulse = 0.0;

		vehicle->num_stages = 0;
		vehicle->stage = 0;
		vehicle->start_pos[0] = vehicle->start_pos[1] = vehicle->start_pos[2] = 0.0;

		vehicle->vel = 0.0;
		vehicle->alt = 0.0;
		vehicle->throttle = 0.0;

		pthread_create(&vehicle->thread_transmit, NULL, transmit_thread, vehicle);
		pthread_create(&vehicle->thread_recv, NULL, recv_thread, vehicle);
	}
	return vehicle;
}

void constellation_vehicle_destroy(ConstellationVehicle* vehicle) {
	pthread_join(vehicle->thread_recv, NULL);
	pthread_join(vehicle->thread_transmit, NULL);

	constellation_event_destroy(vehicle->ev_stage);
	constellation_event_destroy(vehicle->ev_ignite);
	constellation_event_destroy(vehicle->ev_abort);

	free(vehicle->priv);
	free(vehicle);
}

uint8_t constellation_vehicle_get_stage(ConstellationVehicle* vehicle) {
	return vehicle->stage;
}

double constellation_vehicle_get_velocity(ConstellationVehicle* vehicle) {
	return vehicle->vel;
}

double constellation_vehicle_get_alt(ConstellationVehicle* vehicle) {
	return vehicle->alt;
}

double constellation_vehicle_get_throttle(ConstellationVehicle* vehicle) {
	return vehicle->throttle;
}

double constellation_vehicle_get_deltav(ConstellationVehicle* vehicle) {
	return vehicle->curr_stage.impulse * 9.80665 * log(vehicle->curr_stage.mass / vehicle->curr_stage.dry_mass);
}

void constellation_vehicle_stage(ConstellationVehicle* vehicle) {
	if (vehicle->stage < vehicle->num_stages && vehicle->funcs.stage != NULL) {
		vehicle->stage++;
		vehicle->funcs.stage(vehicle, vehicle->priv, vehicle->stage, &vehicle->curr_stage);

		constellation_event_fire(vehicle->ev_stage, vehicle);
	}
}

void constellation_vehicle_ignite(ConstellationVehicle* vehicle) {
	if (vehicle->ignite == false) {
		vehicle->ignite = true;

		if (vehicle->funcs.ignite != NULL) vehicle->funcs.ignite(vehicle, vehicle->priv);

		constellation_event_fire(vehicle->ev_ignite, vehicle);
	}
}

void constellation_vehicle_throttle(ConstellationVehicle* vehicle, float value) {
	vehicle->throttle = value;
	if (vehicle->funcs.throttle != NULL) vehicle->funcs.throttle(vehicle, vehicle->priv);
}

void constellation_vehicle_abort(ConstellationVehicle* vehicle) {
	vehicle->ignite = false;
	constellation_event_fire(vehicle->ev_abort, vehicle);
	if (vehicle->funcs.abort != NULL) vehicle->funcs.abort(vehicle, vehicle->priv);
}

size_t constellation_vehicle_transmit_packet_unqueued(ConstellationVehicle* vehicle, ConstellationPacket* pkt) {
	if (vehicle->funcs.transmit != NULL) {
		return vehicle->funcs.transmit(vehicle, vehicle->priv, pkt);
	}
	return 0;
}

ConstellationPacket* constellation_vehicle_receive_packet_unqueued(ConstellationVehicle* vehicle, size_t* size) {
	if (vehicle->funcs.receive != NULL) {
		return vehicle->funcs.receive(vehicle, vehicle->priv, size);
	}
	return NULL;
}

void constellation_vehicle_transmit(ConstellationVehicle* vehicle, ConstellationPacket* pkt) {
	struct packet_queue* queue = (struct packet_queue*)malloc(sizeof (struct packet_queue));
	queue->pkt = pkt;
	queue->next = vehicle->send_queue;
	vehicle->send_queue = queue;
}

size_t constellation_vehicle_handle_transmit(ConstellationVehicle* vehicle) {
	size_t total_size = 0;
	while (vehicle->send_queue != NULL) {
		struct packet_queue* queue = vehicle->send_queue;
		vehicle->send_queue = queue->next;

		total_size = constellation_vehicle_transmit_packet_unqueued(vehicle, queue->pkt);

		free(queue->pkt);
		free(queue);
	}
	return total_size;
}

size_t constellation_vehicle_handle_receive(ConstellationVehicle* vehicle) {
	size_t size = 0;
	ConstellationPacket* pkt = NULL;
	while ((pkt = constellation_vehicle_receive_packet_unqueued(vehicle, &size)) == NULL);

	if (constellation_packet_verify(pkt)) {
		switch (pkt->hdr.opcode) {
			case CONSTELLATION_PACKET_OPCODE_NONE: break;
			case CONSTELLATION_PACKET_OPCODE_INIT:
				{
					ConstellationPacketInit* init = (ConstellationPacketInit*)pkt;

					vehicle->num_stages = init->num_stages;
					vehicle->stage = init->curr_stage;
					vehicle->start_pos[0] = init->pos[0];
					vehicle->start_pos[1] = init->pos[1];
					vehicle->start_pos[2] = init->pos[2];
				}
				break;
			case CONSTELLATION_PACKET_OPCODE_TELMU:
				{
					ConstellationPacket* pkt = constellation_vehicle_gen_packet(vehicle, CONSTELLATION_PACKET_OPCODE_TELMU_RESP);
					if (pkt != NULL) constellation_vehicle_transmit(vehicle, pkt);
				}
				break;
			case CONSTELLATION_PACKET_OPCODE_IGNITE:
				constellation_vehicle_ignite(vehicle);
				break;
			case CONSTELLATION_PACKET_OPCODE_ABORT:
				constellation_vehicle_ignite(vehicle);
				break;
			default:
				vehicle->recv_failed_count++;
				break;
		}
	} else {
		vehicle->recv_failed_count++;
	}

	free(pkt);
	return size;
}
