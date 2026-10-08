/*
 * core/stack.h  –  fixed-capacity LIFO stack of int64_t values.
 */
#ifndef STACK_H
#define STACK_H

#include <stddef.h>
#include <stdint.h>

#define STACK_CAPACITY 64

typedef struct {
    int64_t data[STACK_CAPACITY];
    size_t  top;   /* index of next free slot; 0 = empty */
} sim_stack_t;

/* Initialise (empty) the stack. */
void stack_init(sim_stack_t *s);

/*
 * Push a value.  Returns 0 on success, -1 if the stack is full (overflow).
 */
int stack_push(sim_stack_t *s, int64_t value);

/*
 * Pop a value into *out.  Returns 0 on success, -1 if the stack is empty
 * (underflow).  On error *out is left unchanged.
 */
int stack_pop(sim_stack_t *s, int64_t *out);

/* Return 1 if the stack is empty, 0 otherwise. */
int stack_empty(const sim_stack_t *s);

/* Return 1 if the stack is full, 0 otherwise. */
int stack_full(const sim_stack_t *s);

/* Current depth (number of elements). */
size_t stack_depth(const sim_stack_t *s);

#endif /* STACK_H */
