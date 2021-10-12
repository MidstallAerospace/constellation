#pragma once

#include <constellation/net/packet.h>
#include <constellation/vehicle.h>
#include <stddef.h>

typedef enum {
	CONSTELLATION_CONTROL_SOURCE_NONE = 0,
	CONSTELLATION_CONTROL_SOURCE_ENG,
	CONSTELLATION_CONTROL_SOURCE_RCS,
	CONSTELLATION_N_CONTROL_SOURCE
} ConstellationControlSource;

typedef struct {
	double impulse;
	double dry_mass;
	double mass;
} ConstellationStageConfiguration;

typedef struct _ConstellationVehicle ConstellationVehicle;

typedef struct {
	void (*stage)(ConstellationVehicle* vehicle, void* priv, uint8_t stage, ConstellationStageConfiguration* config);
	void (*ignite)(ConstellationVehicle* vehicle, void* priv);
  void (*throttle)(ConstellationVehicle* vehicle, void* priv);
  void (*abort)(ConstellationVehicle* vehicle, void* priv);
	void (*rotate)(ConstellationVehicle* vehicle, void* priv, double yaw, double pitch, double roll);
	void (*translate)(ConstellationVehicle* vehicle, void* priv, double x, double y, double z);

	size_t (*transmit)(ConstellationVehicle* vehicle, void* priv, ConstellationPacket* pkt);
	ConstellationPacket* (*receive)(ConstellationVehicle* vehicle, void* priv, size_t* size);
} ConstellationVehicleFuncs;

ConstellationVehicle* constellation_vehicle_new(ConstellationVehicleFuncs funcs, void* priv);

uint8_t constellation_vehicle_get_stage(ConstellationVehicle* vehicle);
double constellation_vehicle_get_velocity(ConstellationVehicle* vehicle);
double constellation_vehicle_get_alt(ConstellationVehicle* vehicle);
double constellation_vehicle_get_throttle(ConstellationVehicle* vehicle);
double constellation_vehicle_get_deltav(ConstellationVehicle* vehicle);

void constellation_vehicle_stage(ConstellationVehicle* vehicle);
void constellation_vehicle_ignite(ConstellationVehicle* vehicle);
void constellation_vehicle_throttle(ConstellationVehicle* vehicle, float value);
void constellation_vehicle_abort(ConstellationVehicle* vehicle);

size_t constellation_vehicle_transmit_packet_unqueued(ConstellationVehicle* vehicle, ConstellationPacket* pkt);
ConstellationPacket* constellation_vehicle_receive_packet_unqueued(ConstellationVehicle* vehicle, size_t* size);

void constellation_vehicle_transmit(ConstellationVehicle* vehicle, ConstellationPacket* pkt);

size_t constellation_vehicle_handle_transmit(ConstellationVehicle* vehicle);
size_t constellation_vehicle_handle_receive(ConstellationVehicle* vehicle);
