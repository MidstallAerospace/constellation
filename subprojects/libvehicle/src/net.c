#include <constellation/net/vehicle.h>
#include <stddef.h>
#include <stdlib.h>

static void gen_telmu(ConstellationVehicle* vehicle, ConstellationPacket* pkt) {
	ConstellationPacketTelmu* telmu = (ConstellationPacketTelmu*)pkt;

	telmu->stage = constellation_vehicle_get_stage(vehicle);
	telmu->velocity = constellation_vehicle_get_velocity(vehicle);
	telmu->alt = constellation_vehicle_get_alt(vehicle);
	telmu->throttle = constellation_vehicle_get_throttle(vehicle);
	telmu->deltav = constellation_vehicle_get_deltav(vehicle);
	telmu->met = constellation_vehicle_get_met(vehicle);
}

static struct {
	uint16_t opcode;
	size_t size;
	void (*gen)(ConstellationVehicle* vehicle, ConstellationPacket* pkt);
} constellation_vehicle_gen_packet_tbl[] = {
	{ CONSTELLATION_PACKET_OPCODE_TELMU_RESP, sizeof (ConstellationPacketTelmu), gen_telmu }
};

ConstellationPacket* constellation_vehicle_alloc_packet(size_t length) {
	ConstellationPacket* pkt = (ConstellationPacket*)malloc(sizeof (ConstellationPacketHeader) + length);
	if (pkt != NULL) {
		pkt->hdr.length = length;
		pkt->hdr.source = CONSTELLATION_PACKET_LOC_VEHICLE;
		memset(pkt->data, 0, length);
	}
	return pkt;
}

ConstellationPacket* constellation_vehicle_gen_packet(ConstellationVehicle* vehicle, uint16_t opcode) {
	size_t n = sizeof (constellation_vehicle_gen_packet_tbl) / sizeof (constellation_vehicle_gen_packet_tbl[0]);
	for (size_t i = 0; i < n; i++) {
		if (constellation_vehicle_gen_packet_tbl[i].opcode != opcode) continue;

		size_t size = sizeof (ConstellationPacketHeader) - constellation_vehicle_gen_packet_tbl[i].size;
		ConstellationPacket* pkt = constellation_vehicle_alloc_packet(size);
		if (pkt == NULL) return NULL;

		constellation_vehicle_gen_packet_tbl[i].gen(vehicle, pkt);
		return pkt;
	}
	return NULL;
}

ConstellationPacket* constellation_vehicle_gen_debug(int lvl, char* message) {
	return constellation_vehicle_gen_sized_debug(lvl, strlen(message), message);
}

ConstellationPacket* constellation_vehicle_gen_sized_debug(int lvl, uint16_t length, char* message) {
	ConstellationPacket* pkt = constellation_vehicle_alloc_packet(sizeof (ConstellationPacketDebug) - sizeof(char*) + (length * sizeof(char)));
	if (pkt != NULL) {
		ConstellationPacketDebug* debug = (ConstellationPacketDebug*)pkt;
		debug->length = length;
		strncpy(debug->message, message, length);
	}
	return pkt;
}
