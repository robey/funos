#include <stdbool.h>
#include <stdint.h>
#include "buffer.h"

void buffer_init(buffer_t *b, char *buffer, uint32_t size) {
  b->buffer = buffer;
  b->size = size;
  b->used = 0;
}

bool buffer_append(buffer_t *b, const char *append, uint32_t size) {
  uint32_t remaining = b->size - b->used;
  if (remaining < size) return false;
  for (uint32_t i = 0; i < size; i++) b->buffer[b->used + i] = append[i];
  b->used += size;
  return true;
}

uint32_t buffer_length(buffer_t *b) {
  return b->used;
}
