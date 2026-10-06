#include "stack.h"
#include <stdio.h>

void stack_init(Stack *stack) { stack->top = -1; }
int stack_push(Stack *stack, int value)
{
    if (stack == NULL || stack->top >= STACK_CAPACITY - 1) return 0;
    stack->items[++stack->top] = value;
    return 1;
}
int stack_pop(Stack *stack, int *value)
{
    if (stack == NULL || value == NULL || stack->top < 0) return 0;
    *value = stack->items[stack->top--];
    return 1;
}
int stack_peek(const Stack *stack, int *value)
{
    if (stack == NULL || value == NULL || stack->top < 0) return 0;
    *value = stack->items[stack->top];
    return 1;
}
void stack_display(const Stack *stack)
{
    int i;
    if (stack->top < 0) { printf("  (empty)\n"); return; }
    for (i = stack->top; i >= 0; --i) printf("  %d\n", stack->items[i]);
}
