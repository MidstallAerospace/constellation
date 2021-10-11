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
#define CONSTELLATION_PACKET_OPCODE_ROT 3
#define CONSTELLATION_PACKET_OPCODE_TRANS 4
#define CONSTELLATION_PACKET_OPCODE_SENSE 5
#define CONSTELLATION_PACKET_OPCODE_SENSE_RESP 6
#define CONSTELLATION_PACKET_OPCODE_TELMU 7
#define CONSTELLATION_PACKET_OPCODE_TELMU_RESP 8
#define CONSTELLATION_PACKET_OPCODE_IGNITE 9
#define CONSTELLATION_PACKET_OPCODE_ABORT 10
#define CONSTELLATION_PACKET_N_OPCODE 11

#define CONSTELLATION_PACKET_LVL_INFO 0
#define CONSTELLATION_PACKET_LVL_DEBU 1
#define CONSTELLATION_PACKET_LVL_WARN 2
#define CONSTELLATION_PACKET_LVL_ERRO 3
#define CONSTELLATION_PACKET_N_LVL 4

#define CONSTELLATION_PACKET_CONTROL_SRC_NONE 0
#define CONSTELLATION_PACKET_CONTROL_SRC_ENG 1
#define CONSTELLATION_PACKET_CONTROL_SRC_RCS 2
#define CONSTELLATION_PACKET_N_CONTROL_SRC 3

#define CONSTELLATION_PACKET_SENSE_NONE 0
#define CONSTELLATION_PACKET_SENSE_CORDS 1
#define CONSTELLATION_PACKET_N_SENSE 2

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

	uint8_t stages;
	double pos[3];
	double mass;
} ConstellationPacketInit;

typedef struct {
	ConstellationPacketHeader hdr;

	int lvl:4;
	uint16_t length;
	char* message;
} ConstellationPacketDebug;

typedef struct {
	ConstellationPacketHeader hdr;

  uint8_t source:3;

	double x;
	double y;
	double z;

	bool lock_x:1;
	bool lock_y:1;
	bool lock_z:1;

	bool cont_x:1;
	bool cont_y:1;
	bool cont_z:1;

	bool enable_x:1;
	bool enable_y:1;
	bool enable_z:1;
} ConstellationPacketRot;

typedef struct {
	ConstellationPacketHeader hdr;

  uint8_t source:3;

	double x;
	double y;
	double z;

	bool enable_x:1;
	bool enable_y:1;
	bool enable_z:1;
} ConstellationPacketTrans;

typedef union {
  uint32_t integer;
  double number;
  bool boolean:1;
} ConstellationPacketSenseData;

typedef struct {
  ConstellationPacketHeader hdr;

  uint8_t index;
  uint8_t count;
  ConstellationPacketSenseData data[];
} ConstellationPacketSense;

typedef struct {
  ConstellationPacketHeader hdr;

  uint8_t stage;
  double velocity;
  double alt;
  double throttle;
} ConstellationPacketTelmu;

bool constellation_packet_header_verify(ConstellationPacketHeader* hdr);
bool constellation_packet_verify(ConstellationPacket* pkt);
