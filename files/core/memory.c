/*
 * core/memory.c  –  simulated flat memory implementation.
 *
 * Business logic only; no logging here.
 */
#define _GNU_SOURCE
#include <string.h>
#include "memory.h"

void memory_init(memory_t *mem)
{
    memset(mem->data, 0, sizeof mem->data);
}

int memory_write(memory_t *mem, size_t addr, int64_t value)
{
    /* Must be within bounds and word-aligned. */
    if (addr + MEM_WORD_SIZE > MEM_SIZE_BYTES || (addr % MEM_WORD_SIZE) != 0)
        return -1;

    int64_t tmp = value;
    memcpy(mem->data + addr, &tmp, MEM_WORD_SIZE);
    return 0;
}

int memory_read(const memory_t *mem, size_t addr, int64_t *value)
{
    if (addr + MEM_WORD_SIZE > MEM_SIZE_BYTES || (addr % MEM_WORD_SIZE) != 0)
        return -1;

    memcpy(value, mem->data + addr, MEM_WORD_SIZE);
    return 0;
}
