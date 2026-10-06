#ifndef STACK_H
#define STACK_H

#define STACK_CAPACITY 10

typedef struct {
    int items[STACK_CAPACITY];
    int top;
} Stack;

void stack_init(Stack *stack);
int stack_push(Stack *stack, int value);
int stack_pop(Stack *stack, int *value);
int stack_peek(const Stack *stack, int *value);
void stack_display(const Stack *stack);

#endif
