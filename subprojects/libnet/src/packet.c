#include <constellation/net/packet.h>
#include <stddef.h>

static bool verify_debug(ConstellationPacket* pkt) {
	ConstellationPacketDebug* debug = (ConstellationPacketDebug*)pkt;

	if (debug->lvl > CONSTELLATION_PACKET_N_LVL) return false;

	uint16_t total_length = sizeof (ConstellationPacketDebug) + debug->length;
	if (debug->hdr.length != total_length) return false;
	return true;
}

static bool verify_rot(ConstellationPacket* pkt) {
	ConstellationPacketRot* rot = (ConstellationPacketRot*)pkt;

	if (rot->source > CONSTELLATION_PACKET_N_CONTROL_SRC) return false;

	if (rot->enable_x == false && (rot->lock_x || rot->cont_x || rot->x != 0)) return false;
	if (rot->enable_y == false && (rot->lock_y || rot->cont_y || rot->y != 0)) return false;
	if (rot->enable_z == false && (rot->lock_z || rot->cont_z || rot->z != 0)) return false;
	return true;
}

static bool verify_trans(ConstellationPacket* pkt) {
	ConstellationPacketTrans* rot = (ConstellationPacketTrans*)pkt;

	if (rot->source > CONSTELLATION_PACKET_N_CONTROL_SRC) return false;

	if (rot->enable_x == false && rot->x != 0) return false;
	if (rot->enable_y == false && rot->y != 0) return false;
	if (rot->enable_z == false && rot->z != 0) return false;
	return true;
}

static bool verify_sense(ConstellationPacket* pkt) {
	ConstellationPacketSense* sense = (ConstellationPacketSense*)pkt;

	if (sense->index > CONSTELLATION_PACKET_N_SENSE) return false;

	uint16_t total_length = sizeof (ConstellationPacketSense) + (sizeof (ConstellationPacketSenseData) * sense->count);
	if (sense->hdr.length != total_length) return false;
	return true;
}

static bool verify_telmu_resp(ConstellationPacket* pkt) {
	return pkt->hdr.source == CONSTELLATION_PACKET_LOC_VEHICLE;
}

static struct {
	int opcode;
	bool (*verify)(ConstellationPacket* pkt);
	uint32_t length;
	bool extra:1;
} constellation_packet_verify_tbl[] = {
	{ CONSTELLATION_PACKET_OPCODE_NONE, 			NULL,							 sizeof (ConstellationPacketHeader), false },
	{ CONSTELLATION_PACKET_OPCODE_INIT,  			NULL,							 sizeof (ConstellationPacketInit),	 false },
	{ CONSTELLATION_PACKET_OPCODE_DEBUG,		  verify_debug,			 sizeof (ConstellationPacketDebug),  true  },
	{ CONSTELLATION_PACKET_OPCODE_ROT,   			verify_rot, 		   sizeof (ConstellationPacketRot),		 false },
	{ CONSTELLATION_PACKET_OPCODE_TRANS,			verify_trans,			 sizeof (ConstellationPacketTrans),	 false },
	{ CONSTELLATION_PACKET_OPCODE_SENSE,			verify_sense,			 sizeof (ConstellationPacketSense),	 true  },
	{ CONSTELLATION_PACKET_OPCODE_TELMU,		  NULL,							 sizeof (ConstellationPacketHeader), false },
	{ CONSTELLATION_PACKET_OPCODE_TELMU_RESP, verify_telmu_resp, sizeof (ConstellationPacketTelmu),  false }
};

bool constellation_packet_header_verify(ConstellationPacketHeader* hdr) {
	if (hdr->magic != CONSTELLATION_PACKET_MAGIC) return false;
	if (hdr->opcode > CONSTELLATION_PACKET_N_OPCODE) return false;
	if (hdr->source > CONSTELLATION_PACKET_N_LOC) return false;
	if (hdr->dest > CONSTELLATION_PACKET_N_LOC) return false;

	int n = sizeof(constellation_packet_verify_tbl) / sizeof(constellation_packet_verify_tbl[0]);
	for (int i = 0; i < n; i++) {
		if (constellation_packet_verify_tbl[i].opcode != hdr->opcode) continue;

		uint32_t expect_len = sizeof(ConstellationPacketHeader) - constellation_packet_verify_tbl[i].length;
		if (constellation_packet_verify_tbl[i].extra) {
			if (hdr->length < expect_len) return false;
		} else {
			if (hdr->length != expect_len) return false;
		}
	}
	return false;
}

bool constellation_packet_verify(ConstellationPacket* pkt) {
	if (!constellation_packet_header_verify(&pkt->hdr)) {
		return false;
	}

	int n = sizeof(constellation_packet_verify_tbl) / sizeof(constellation_packet_verify_tbl[0]);
	for (int i = 0; i < n; i++) {
		if (constellation_packet_verify_tbl[i].opcode != pkt->hdr.opcode) continue;
		if (constellation_packet_verify_tbl[i].verify == NULL) return true;
		return constellation_packet_verify_tbl[i].verify(pkt);
	}
	return false;
}
