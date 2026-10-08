/*
 * core/queue.c  –  circular FIFO queue implementation.  No logging here.
 */
#include "queue.h"

void queue_init(queue_t *q)
{
    q->head  = 0;
    q->tail  = 0;
    q->count = 0;
}

int queue_enqueue(queue_t *q, int64_t value)
{
    if (q->count >= QUEUE_CAPACITY)
        return -1;
    q->data[q->tail] = value;
    q->tail = (q->tail + 1) % QUEUE_CAPACITY;
    q->count++;
    return 0;
}

int queue_dequeue(queue_t *q, int64_t *out)
{
    if (q->count == 0)
        return -1;
    *out    = q->data[q->head];
    q->head = (q->head + 1) % QUEUE_CAPACITY;
    q->count--;
    return 0;
}

int    queue_empty(const queue_t *q) { return q->count == 0; }
int    queue_full(const queue_t *q)  { return q->count >= QUEUE_CAPACITY; }
size_t queue_count(const queue_t *q) { return q->count; }
