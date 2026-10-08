/*
 * core/memory.h  –  simulated flat memory region API.
 *
 * Provides a fixed-size byte-addressable memory bank with aligned word
 * (int64_t) read/write operations and bounds checking.
 */
#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>

/* Total simulated memory in bytes (must be a multiple of sizeof(int64_t)). */
#define MEM_SIZE_BYTES  4096
#define MEM_WORD_SIZE   ((size_t)sizeof(int64_t))

typedef struct {
    uint8_t data[MEM_SIZE_BYTES];
} memory_t;

/* Zero-initialise the memory bank. */
void memory_init(memory_t *mem);

/*
 * Write a 64-bit word at byte-offset addr.
 * addr must be word-aligned and within [0, MEM_SIZE_BYTES – MEM_WORD_SIZE].
 * Returns  0 on success, -1 on bad address.
 */
int memory_write(memory_t *mem, size_t addr, int64_t value);

/*
 * Read a 64-bit word from byte-offset addr.
 * Returns  0 on success, -1 on bad address.
 * On error *value is left unchanged.
 */
int memory_read(const memory_t *mem, size_t addr, int64_t *value);

#endif /* MEMORY_H */
