/*
 * core/stack.c  –  LIFO stack implementation.  No logging here.
 */
#include "stack.h"

void stack_init(sim_stack_t *s)
{
    s->top = 0;
}

int stack_push(sim_stack_t *s, int64_t value)
{
    if (s->top >= STACK_CAPACITY)
        return -1;   /* overflow */
    s->data[s->top++] = value;
    return 0;
}

int stack_pop(sim_stack_t *s, int64_t *out)
{
    if (s->top == 0)
        return -1;   /* underflow */
    *out = s->data[--s->top];
    return 0;
}

int stack_empty(const sim_stack_t *s)  { return s->top == 0; }
int stack_full(const sim_stack_t *s)   { return s->top >= STACK_CAPACITY; }
size_t stack_depth(const sim_stack_t *s) { return s->top; }
