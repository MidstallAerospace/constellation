#include <constellation/common.h>
#include <stdlib.h>

struct _ConstellationEvent {
	ConstellationEventCallback* callbacks;
	size_t max_events;
};

ConstellationEvent* constellation_event_new(size_t max) {
	ConstellationEvent* ev = (ConstellationEvent*)malloc(sizeof (ConstellationEvent));
	if (ev != NULL) {
		ev->max_events = max;
		ev->callbacks = (ConstellationEventCallback*)malloc(sizeof (ConstellationEventCallback) * ev->max_events);
		if (ev->callbacks == NULL) {
			free(ev);
			return NULL;
		}

		for (size_t i = 0; i < ev->max_events; i++) ev->callbacks[i] = NULL;
	}
	return ev;
}

void constellation_event_destroy(ConstellationEvent* ev) {
	free(ev->callbacks);
	free(ev);
}

void constellation_event_add(ConstellationEvent* ev, ConstellationEventCallback cb) {
	for (size_t i = 0; i < ev->max_events; i++) {
		if (ev->callbacks[i] == NULL) {
			ev->callbacks[i] = cb;
			break;
		}
	}
}

void constellation_event_remove(ConstellationEvent* ev, ConstellationEventCallback cb) {
	for (size_t i = 0; i < ev->max_events; i++) {
		if (ev->callbacks[i] == cb) ev->callbacks[i] = NULL;
	}
}

void constellation_event_fire(ConstellationEvent* ev, void* source) {
	for (size_t i = 0; i < ev->max_events; i++) {
		if (ev->callbacks[i] != NULL) ev->callbacks[i](source);
	}
}