#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  char *buffer;
  uint32_t size;
  uint32_t used;
} buffer_t;

void buffer_init(buffer_t *b, char *buffer, uint32_t size);
bool buffer_append(buffer_t *b, const char *append, uint32_t size);
uint32_t buffer_length(buffer_t *b);
