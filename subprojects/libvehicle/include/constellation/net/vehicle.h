#pragma once

#include <constellation/net/packet.h>
#include <constellation/vehicle.h>
#include <string.h>

ConstellationPacket* constellation_vehicle_alloc_packet(size_t length);
ConstellationPacket* constellation_vehicle_gen_packet(ConstellationVehicle* vehicle, uint16_t opcode);
ConstellationPacket* constellation_vehicle_gen_debug(int lvl, char* message);
ConstellationPacket* constellation_vehicle_gen_sized_debug(int lvl, uint16_t length, char* message);
