/*
 * core/queue.h  –  fixed-capacity circular FIFO queue of int64_t values.
 */
#ifndef QUEUE_H
#define QUEUE_H

#include <stddef.h>
#include <stdint.h>

#define QUEUE_CAPACITY 64

typedef struct {
    int64_t data[QUEUE_CAPACITY];
    size_t  head;    /* dequeue from here */
    size_t  tail;    /* enqueue here */
    size_t  count;   /* number of elements */
} queue_t;

/* Initialise (empty) the queue. */
void queue_init(queue_t *q);

/*
 * Enqueue a value.  Returns 0 on success, -1 if the queue is full.
 */
int queue_enqueue(queue_t *q, int64_t value);

/*
 * Dequeue a value into *out.  Returns 0 on success, -1 if the queue is empty.
 */
int queue_dequeue(queue_t *q, int64_t *out);

int    queue_empty(const queue_t *q);
int    queue_full(const queue_t *q);
size_t queue_count(const queue_t *q);

#endif /* QUEUE_H */
