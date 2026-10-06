#ifndef MEMORY_H
#define MEMORY_H

#define MEMORY_SIZE 100

typedef struct {
    int values[MEMORY_SIZE];
    int used[MEMORY_SIZE];
} Memory;

void memory_init(Memory *memory);
int memory_write(Memory *memory, int address, int value);
int memory_read(const Memory *memory, int address, int *value);
void memory_display(const Memory *memory);

#endif
