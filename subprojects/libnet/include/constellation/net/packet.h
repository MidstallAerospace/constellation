#pragma once

#include <stdbool.h>
#include <stdint.h>

#define CONSTELLATION_PACKET_MAGIC 0xCA01

#define CONSTELLATION_PACKET_LOC_NONE 0
#define CONSTELLATION_PACKET_LOC_VEHICLE 1
#define CONSTELLATION_PACKET_LOC_RELAY 2
#define CONSTELLATION_PACKET_LOC_CONTROL 3
#define CONSTELLATION_PACKET_N_LOC 4

#define CONSTELLATION_PACKET_OPCODE_NONE 0
#define CONSTELLATION_PACKET_OPCODE_INIT 1
#define CONSTELLATION_PACKET_OPCODE_DEBUG 2
#define CONSTELLATION_PACKET_OPCODE_TELMU 3
#define CONSTELLATION_PACKET_OPCODE_TELMU_RESP 4
#define CONSTELLATION_PACKET_OPCODE_IGNITE 5
#define CONSTELLATION_PACKET_OPCODE_ABORT 6
#define CONSTELLATION_PACKET_OPCODE_PROG 7
#define CONSTELLATION_PACKET_N_OPCODE 8

#define CONSTELLATION_PACKET_LVL_INFO 0
#define CONSTELLATION_PACKET_LVL_DEBU 1
#define CONSTELLATION_PACKET_LVL_WARN 2
#define CONSTELLATION_PACKET_LVL_ERRO 3
#define CONSTELLATION_PACKET_N_LVL 4

#define CONSTELLATION_PACKET_PRG_NONE 0
#define CONSTELLATION_PACKET_PRG_ORBIT 1
#define CONSTELLATION_PACKET_PRG_LAND 2
#define CONSTELLATION_PACKET_N_PRG 2

typedef struct {
	uint16_t magic;
	uint8_t source:4;
	uint8_t dest:4;
	uint16_t opcode;
	uint16_t length;
} ConstellationPacketHeader;

typedef struct {
	ConstellationPacketHeader hdr;
	void* data;
} ConstellationPacket;

typedef struct {
	ConstellationPacketHeader hdr;

	uint8_t num_stages;
	uint8_t curr_stage;
	double pos[3];
} ConstellationPacketInit;

typedef struct {
	ConstellationPacketHeader hdr;

	int lvl:4;
	uint16_t length;
	char* message;
} ConstellationPacketDebug;

typedef struct {
	ConstellationPacketHeader hdr;

	uint8_t stage;
	double velocity;
	double alt;
	double throttle;
} ConstellationPacketTelmu;

typedef struct {
	ConstellationPacketHeader hdr;

	uint8_t prg;

	uint16_t data_int[3];
	double data_dbl[3];
} ConstellationPacketProg;

bool constellation_packet_header_verify(ConstellationPacketHeader* hdr);
bool constellation_packet_verify(ConstellationPacket* pkt);