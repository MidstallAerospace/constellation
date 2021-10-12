#include <constellation/net/packet.h>
#include <stddef.h>

static bool verify_debug(ConstellationPacket* pkt) {
	ConstellationPacketDebug* debug = (ConstellationPacketDebug*)pkt;

	if (debug->lvl > CONSTELLATION_PACKET_N_LVL) return false;

	uint16_t total_length = sizeof (ConstellationPacketDebug) + debug->length;
	if (debug->hdr.length != total_length) return false;
	return true;
}

static bool verify_telmu_resp(ConstellationPacket* pkt) {
	return pkt->hdr.source == CONSTELLATION_PACKET_LOC_VEHICLE;
}

static bool verify_prg(ConstellationPacket* pkt) {
	ConstellationPacketProg* prog = (ConstellationPacketProg*)pkt;
	return prog->prg < CONSTELLATION_PACKET_N_PRG;
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
	{ CONSTELLATION_PACKET_OPCODE_TELMU,		  NULL,							 sizeof (ConstellationPacketHeader), false },
	{ CONSTELLATION_PACKET_OPCODE_TELMU_RESP, verify_telmu_resp, sizeof (ConstellationPacketTelmu),  false },
	{ CONSTELLATION_PACKET_OPCODE_IGNITE, 		NULL,							 sizeof (ConstellationPacketHeader), false },
	{ CONSTELLATION_PACKET_OPCODE_ABORT, 			NULL,							 sizeof (ConstellationPacketHeader), false },
	{ CONSTELLATION_PACKET_OPCODE_PROG,				verify_prg, 			 sizeof (ConstellationPacketProg),		 false }
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
