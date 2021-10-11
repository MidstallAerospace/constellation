#include <constellation/net/vehicle.h>
#include <constellation/vehicle.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

struct packet_queue {
	ConstellationPacket* pkt;
	struct packet_queue* next;
};

struct _ConstellationVehicle {
	ConstellationVehicleFuncs funcs;

	void* priv;
	size_t psize;

	struct packet_queue* send_queue;
	struct packet_queue* recv_queue;
	uint32_t recv_failed_count;
	uint32_t total_transmit;
	uint32_t total_recv;

	pthread_t thread_recv;
	pthread_t thread_queue_recv;
	pthread_t thread_transmit;
	pthread_t thread_rot;
	pthread_t thread_translation;

  ConstellationRotationList* rot_queue;
  ConstellationTranslationList* trans_queue;

	bool ignite:1;

	uint8_t num_stages;
	double mass;
	double start_pos[3];

	int stage;
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

static void* queue_recv_thread(void* data) {
	ConstellationVehicle* vehicle = (ConstellationVehicle*)data;
	vehicle->total_recv = 0;
	while (vehicle->ignite) vehicle->total_recv = constellation_vehicle_handle_receive(vehicle);
	return NULL;
}

static void* recv_thread(void* data) {
	ConstellationVehicle* vehicle = (ConstellationVehicle*)data;
	while (vehicle->ignite) constellation_vehicle_receive(vehicle);
	return NULL;
}

static void* rot_thread(void* data) {
	ConstellationVehicle* vehicle = (ConstellationVehicle*)data;
	while (vehicle->ignite) {
		while (vehicle->rot_queue != NULL) {	
			ConstellationRotationList* list = vehicle->rot_queue;
			vehicle->rot_queue = list->next;

			if (vehicle->funcs.rotate != NULL) vehicle->funcs.rotate(vehicle, vehicle->priv, list);

			free(list);
		}
	}
	return NULL;
}

static void* translation_thread(void* data) {
	ConstellationVehicle* vehicle = (ConstellationVehicle*)data;
	while (vehicle->ignite) {
		while (vehicle->rot_queue != NULL) {	
			ConstellationTranslationList* list = vehicle->trans_queue;
			vehicle->trans_queue = list->next;

			if (vehicle->funcs.translate != NULL) vehicle->funcs.translate(vehicle, vehicle->priv, list);

			free(list);
		}
	}
	return NULL;
}

static void insert_rotation(ConstellationVehicle* vehicle, ConstellationRotationList* current) {
  ConstellationRotationList** last_of_same = &vehicle->rot_queue;
  while ((*last_of_same) != NULL) {
    if ((*last_of_same)->next->source != (*last_of_same)->source) {
      if ((*last_of_same)->source == current->source) {
        current->next = (*last_of_same)->next;
        *last_of_same = current;
        return;
      }
    }
    last_of_same = &(*last_of_same)->next;
  }

	ConstellationRotationList* node = vehicle->rot_queue;
	while (node != NULL) {
		if (node->next == NULL) {
			node->next = current;
			break;
		}
		node = node->next;
	}
}

static void insert_translation(ConstellationVehicle* vehicle, ConstellationTranslationList* current) {
  ConstellationTranslationList** last_of_same = &vehicle->trans_queue;
  while ((*last_of_same) != NULL) {
    if ((*last_of_same)->next->source != (*last_of_same)->source) {
      if ((*last_of_same)->source == current->source) {
        current->next = (*last_of_same)->next;
        *last_of_same = current;
        return;
      }
    }
    last_of_same = &(*last_of_same)->next;
  }

	ConstellationTranslationList* node = vehicle->trans_queue;
	while (node != NULL) {
		if (node->next == NULL) {
			node->next = current;
			break;
		}
		node = node->next;
	}
}

ConstellationVehicle* constellation_vehicle_new(ConstellationVehicleFuncs funcs, size_t psize) {
	ConstellationVehicle* vehicle = (ConstellationVehicle*)malloc(sizeof(ConstellationVehicle));
	if (vehicle != NULL) {
		vehicle->psize = psize;
		vehicle->priv = malloc(psize);
		if (vehicle->priv == NULL) {
			free(vehicle);
			return NULL;
		}

		vehicle->funcs = funcs;

		vehicle->send_queue = vehicle->recv_queue = NULL;
		vehicle->recv_failed_count = 0;
		vehicle->total_transmit = 0;
		vehicle->total_recv = 0;

    vehicle->rot_queue = NULL;
    vehicle->trans_queue = NULL;

		vehicle->ignite = false;

		vehicle->num_stages = 0;
		vehicle->mass = 0.0;
		vehicle->start_pos[0] = vehicle->start_pos[1] = vehicle->start_pos[2] = 0.0;

		vehicle->stage = 0;
		vehicle->vel = 0.0;
		vehicle->alt = 0.0;
		vehicle->throttle = 0.0;

		pthread_create(&vehicle->thread_transmit, NULL, transmit_thread, vehicle);
		pthread_create(&vehicle->thread_recv, NULL, recv_thread, vehicle);
		pthread_create(&vehicle->thread_queue_recv, NULL, queue_recv_thread, vehicle);
		pthread_create(&vehicle->thread_rot, NULL, rot_thread, vehicle);
		pthread_create(&vehicle->thread_translation, NULL, translation_thread, vehicle);
	}
	return vehicle;
}

void constellation_vehicle_destroy(ConstellationVehicle* vehicle) {
	pthread_join(vehicle->thread_recv, NULL);
	pthread_join(vehicle->thread_queue_recv, NULL);
	pthread_join(vehicle->thread_transmit, NULL);
	pthread_join(vehicle->thread_rot, NULL);
	pthread_join(vehicle->thread_translation, NULL);

	free(vehicle->priv);
	free(vehicle);
}

int constellation_vehicle_get_stage(ConstellationVehicle* vehicle) {
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

void constellation_vehicle_stage(ConstellationVehicle* vehicle) {
	if (vehicle->stage < vehicle->num_stages && vehicle->funcs.stage != NULL) {
		vehicle->stage++;
		// TODO: signal event
		vehicle->funcs.stage(vehicle, vehicle->priv);
	}
}

void constellation_vehicle_ignite(ConstellationVehicle* vehicle) {
	if (vehicle->ignite == false) {
		vehicle->ignite = true;

		if (vehicle->funcs.ignite != NULL) vehicle->funcs.ignite(vehicle, vehicle->priv);

		// TODO: signal event
	}
}

void constellation_vehicle_throttle(ConstellationVehicle* vehicle, float value) {
	vehicle->throttle = value;
	if (vehicle->funcs.throttle != NULL) vehicle->funcs.throttle(vehicle, vehicle->priv);
	// TODO: signal event
}

void constellation_vehicle_abort(ConstellationVehicle* vehicle) {
	vehicle->ignite = false;
	// TODO: signal an abort
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

void constellation_vehicle_receive(ConstellationVehicle* vehicle) {
	size_t size = 0;
	ConstellationPacket* pkt = NULL;
	while ((pkt = constellation_vehicle_receive_packet_unqueued(vehicle, &size)) == NULL);

	struct packet_queue* queue = (struct packet_queue*)malloc(sizeof (struct packet_queue));
	queue->pkt = pkt;
	queue->next = vehicle->recv_queue;
	vehicle->recv_queue = queue;
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
	size_t count = 0;
	while (vehicle->recv_queue != NULL) {
		struct packet_queue* queue = vehicle->recv_queue;
		vehicle->recv_queue = queue->next;

		if (constellation_packet_verify(queue->pkt)) {
      count += sizeof (ConstellationPacketHeader) + queue->pkt->hdr.length;

			switch (queue->pkt->hdr.opcode) {
				case CONSTELLATION_PACKET_OPCODE_NONE: break;
				case CONSTELLATION_PACKET_OPCODE_INIT:
					{
						ConstellationPacketInit* init = (ConstellationPacketInit*)queue->pkt;

						vehicle->num_stages = init->stages;
						vehicle->mass = init->mass;
						vehicle->start_pos[0] = init->pos[0];
						vehicle->start_pos[1] = init->pos[1];
						vehicle->start_pos[2] = init->pos[2];
					}
					break;
				case CONSTELLATION_PACKET_OPCODE_ROT:
					{
						ConstellationPacketRot* rot = (ConstellationPacketRot*)queue->pkt;

            ConstellationRotationList* list = vehicle->rot_queue;
            if (list == NULL) {
              list = vehicle->rot_queue = malloc(sizeof (ConstellationRotationList));
              if (list != NULL) {
                list->enable[0] = rot->enable_x;
                list->enable[1] = rot->enable_y;
                list->enable[2] = rot->enable_z;

                list->lock[0] = rot->lock_x;
                list->lock[1] = rot->lock_y;
                list->lock[2] = rot->lock_z;

                list->cont[0] = rot->cont_x;
                list->cont[1] = rot->cont_y;
                list->cont[2] = rot->cont_z;

                list->source = rot->source;

                list->x = rot->x;
                list->y = rot->y;
                list->z = rot->z;
                list->next = NULL;
              }
            } else {
              list = malloc(sizeof (ConstellationRotationList));
              if (list != NULL) {
                list->enable[0] = rot->enable_x;
                list->enable[1] = rot->enable_y;
                list->enable[2] = rot->enable_z;

                list->source = rot->source;

                list->x = rot->x;
                list->y = rot->y;
                list->z = rot->z;
                list->next = NULL;

                insert_rotation(vehicle, list);
              }
            }
					}
					break;
				case CONSTELLATION_PACKET_OPCODE_TRANS:
					{
						ConstellationPacketTrans* trans = (ConstellationPacketTrans*)queue->pkt;

            ConstellationTranslationList* list = vehicle->trans_queue;
            if (list == NULL) {
              list = vehicle->trans_queue = malloc(sizeof (ConstellationTranslationList));
              if (list != NULL) {
                list->enable[0] = trans->enable_x;
                list->enable[1] = trans->enable_y;
                list->enable[2] = trans->enable_z;

                list->source = trans->source;

                list->x = trans->x;
                list->y = trans->y;
                list->z = trans->z;
                list->next = NULL;
              }
            } else {
              list = malloc(sizeof (ConstellationTranslationList));
              if (list != NULL) {
                list->enable[0] = trans->enable_x;
                list->enable[1] = trans->enable_y;
                list->enable[2] = trans->enable_z;

                list->source = trans->source;

                list->x = trans->x;
                list->y = trans->y;
                list->z = trans->z;
                list->next = NULL;

                insert_translation(vehicle, list);
              }
            }
					}
					break;
				case CONSTELLATION_PACKET_OPCODE_SENSE:
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

		free(queue->pkt);
		free(queue);
	}
	return count;
}
