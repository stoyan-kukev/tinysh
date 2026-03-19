#include "arena.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARENA_ALIGNMENT 16
#define ALIGN_UP(n, a) (((n) + (a) - 1) & ~((a) - 1))

static ArenaRegion *arena_region_create(size_t capacity) {
  size_t total_size = sizeof(ArenaRegion) + capacity;
  ArenaRegion *region = malloc(total_size);
  if (!region) {
    fprintf(stderr, "FATAL: Arena failed to allocate a memory region.\n");
    exit(EXIT_FAILURE);
  }

  region->next = NULL;
  region->capacity = capacity;
  region->offset = 0;

  return region;
}

void arena_init(Arena *arena, size_t default_size) {
  if (default_size == 0) {
    default_size = ARENA_DEFAULT_REGION_SIZE;
  }

  arena->default_region_size = default_size;
  arena->head = arena_region_create(default_size);
  arena->current = arena->head;
}

void *arena_alloc(Arena *arena, size_t size) {
  size_t aligned_size = ALIGN_UP(size, ARENA_ALIGNMENT);
  ArenaRegion *region = arena->current;

  if (region->offset + aligned_size > region->capacity) {
    if (region->next != NULL) {
      arena->current = region->next;
      region = arena->current;
    } else {
      size_t new_capacity = arena->default_region_size;
      if (aligned_size > new_capacity) {
        new_capacity = aligned_size;
      }

      ArenaRegion *new_region = arena_region_create(new_capacity);
      region->next = new_region;
      arena->current = new_region;
      region = new_region;
    }
  }

  void *ptr = (uint8_t *)region->data + region->offset;
  region->offset += aligned_size;

  return ptr;
}

void *arena_calloc(Arena *arena, size_t count, size_t size) {
  size_t total_size = count * size;
  void *ptr = arena_alloc(arena, total_size);
  if (ptr) {
    memset(ptr, 0, total_size);
  }

  return ptr;
}

void arena_reset(Arena *arena) {
  ArenaRegion *region = arena->head;
  while (region != NULL) {
    region->offset = 0;
    region = region->next;
  }

  arena->current = arena->head;
}

void arena_deinit(Arena *arena) {
  ArenaRegion *region = arena->head;
  while (region != NULL) {
    ArenaRegion *next = region->next;
    free(region);
    region = next;
  }

  arena->head = NULL;
  arena->current = NULL;
}
