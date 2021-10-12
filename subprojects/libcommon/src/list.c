#include <constellation/common.h>
#include <stdlib.h>

ConstellationList* constellation_list_new(void* data, size_t max_children) {
  ConstellationList* self = (ConstellationList*)malloc(sizeof(ConstellationList));
  if (self != NULL) {
    self->data = data;
    self->max_children = max_children;
    self->destroy = NULL;
    self->next = NULL;
  }
  return self;
}

ConstellationList* constelllation_list_pop(ConstellationList* list) {
  ConstellationList* next = list->next;
  if (list->destroy != NULL) list->destroy(list);
  free(list);
  return next;
}

ConstellationList* constellation_list_append(ConstellationList* head, void* data) {
  return constellation_list_append_sized(head, data, head == NULL ? -1 : (head->max_children - 1));
}

ConstellationList* constellation_list_append_sized(ConstellationList* head, void* data, size_t max_children) {
  ConstellationList* child = constellation_list_new(data, max_children);
  if (head == NULL) return child;

  child->destroy = head->destroy;

  ConstellationList* tail = constellation_list_tail(head);
  if (tail == NULL) {
    if (child->destroy != NULL) child->destroy(child);
    free(child);
    return NULL;
  }

  if (tail->max_children == 0) {
    if (child->destroy != NULL) child->destroy(child);
    free(child);
    return NULL;
  }

  tail->next = child;
  return head;
}

ConstellationList* constellation_list_tail(ConstellationList* head) {
  ConstellationList* node = head;
  while (node != NULL) {
    if (node->next == NULL) return node;
    node = node->next;
  }
  return NULL;
}

size_t constellation_list_length(ConstellationList* list) {
  size_t c = 0;
  ConstellationList* node = list;
  while (node != NULL) {
    c++;
    node = list->next;
  }
  return c;
}
