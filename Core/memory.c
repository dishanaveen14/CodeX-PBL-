#include "memory.h"
#include <stdio.h>

void memory_init(Memory *memory)
{
    int i;
    for (i = 0; i < MEMORY_SIZE; ++i) {
        memory->values[i] = 0;
        memory->used[i] = 0;
    }
}

int memory_write(Memory *memory, int address, int value)
{
    if (memory == NULL || address < 0 || address >= MEMORY_SIZE) return 0;
    memory->values[address] = value;
    memory->used[address] = 1;
    return 1;
}

int memory_read(const Memory *memory, int address, int *value)
{
    if (memory == NULL || value == NULL || address < 0 || address >= MEMORY_SIZE || !memory->used[address]) return 0;
    *value = memory->values[address];
    return 1;
}

void memory_display(const Memory *memory)
{
    int i, any = 0;
    for (i = 0; i < MEMORY_SIZE; ++i) {
        if (memory->used[i]) {
            printf("  Memory[%d] = %d\n", i, memory->values[i]);
            any = 1;
        }
    }
    if (!any) printf("  (empty)\n");
}
