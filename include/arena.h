#ifndef TINYSH_ARENA_H
#define TINYSH_ARENA_H

#include <stddef.h>
#include <stdint.h>

#define ARENA_DEFAULT_REGION_SIZE (64 * 1024)

typedef struct ArenaRegion {
  struct ArenaRegion* next;
  size_t capacity;
  size_t offset;
  uintptr_t data[];
} ArenaRegion;

typedef struct Arena {
  ArenaRegion *head;
  ArenaRegion *current;
  size_t default_region_size;
} Arena;

// Initializes an arena by allocating a default_size chunk of memory
void arena_init(Arena* arena, size_t default_size);
// Allocates memory using an initialized arena. Can call malloc to expand the arena.
void* arena_alloc(Arena* arena, size_t size);
// Calls arena_alloc for N count items
void* arena_calloc(Arena* arena, size_t count, size_t size);
// Resets the arena. All pointers to previously allocated memory become invalid.
void arena_reset(Arena* arena);
// Frees the memory held by the arena. All pointers to previously allocated memory become invalid.
void arena_deinit(Arena* arena);

#endif
