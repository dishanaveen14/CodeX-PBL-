#include "queue.h"
#include <stdio.h>

void queue_init(Queue *queue) { queue->front = 0; queue->count = 0; }
int queue_enqueue(Queue *queue, int value)
{
    int tail;
    if (queue == NULL || queue->count >= QUEUE_CAPACITY) return 0;
    tail = (queue->front + queue->count) % QUEUE_CAPACITY;
    queue->items[tail] = value;
    ++queue->count;
    return 1;
}
int queue_dequeue(Queue *queue, int *value)
{
    if (queue == NULL || value == NULL || queue->count == 0) return 0;
    *value = queue->items[queue->front];
    queue->front = (queue->front + 1) % QUEUE_CAPACITY;
    --queue->count;
    return 1;
}
int queue_peek(const Queue *queue, int *value)
{
    if (queue == NULL || value == NULL || queue->count == 0) return 0;
    *value = queue->items[queue->front];
    return 1;
}
void queue_display(const Queue *queue)
{
    int i;
    if (queue->count == 0) { printf("  (empty)\n"); return; }
    for (i = 0; i < queue->count; ++i)
        printf("  %d\n", queue->items[(queue->front + i) % QUEUE_CAPACITY]);
}
