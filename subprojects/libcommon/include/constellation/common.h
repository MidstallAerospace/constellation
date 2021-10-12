#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/** Linked list **/
struct _ConstellationList;

typedef void (*ConstellationListDestroyCallback)(struct _ConstellationList* self);

typedef struct _ConstellationList {
	void* data;
	size_t max_children;
	ConstellationListDestroyCallback destroy;
	struct _ConstellationList* next;
} ConstellationList;

#define constellation_list_is_full(list) ((list)->max_children == -1 ? false : constellation_list_length((list)) > (list)->max_children)

ConstellationList* constellation_list_new(void* data, size_t max_children);
ConstellationList* constelllation_queue_pop(ConstellationList* list);

ConstellationList* constellation_list_append(ConstellationList* head, void* data);
ConstellationList* constellation_list_append_sized(ConstellationList* head, void* data, size_t max_children);

ConstellationList* constellation_list_tail(ConstellationList* head);

size_t constellation_list_length(ConstellationList* list);

/** Event **/
typedef void (*ConstellationEventCallback)(void* source);
typedef struct _ConstellationEvent ConstellationEvent;

ConstellationEvent* constellation_event_new(size_t max);
void constellation_event_destroy(ConstellationEvent* ev);
void constellation_event_add(ConstellationEvent* ev, ConstellationEventCallback cb);
void constellation_event_remove(ConstellationEvent* ev, ConstellationEventCallback cb);
void constellation_event_fire(ConstellationEvent* ev, void* source);

/** Program **/
typedef union {
	struct {
		bool orbit:1;
		double altitude;
		double periapsis;
		double apoapsis;
	} launch;
	struct {
		uint16_t id;
		uint32_t interval;
	} timer;
} ConstellationProgramData;