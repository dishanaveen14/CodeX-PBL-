#include "cpu.h"
#include "memory.h"
#include "stack.h"
#include "queue.h"
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct { Memory memory; Stack stack; Queue queue; char last_operation[4]; int last_result; int has_cpu_result; } Core;
static int read_integer(const char *prompt, int *value)
{
    char line[128], *end; long parsed;
    printf("%s", prompt);
    if (fgets(line, sizeof line, stdin) == NULL) return 0;
    errno = 0; parsed = strtol(line, &end, 10);
    while (isspace((unsigned char)*end)) ++end;
    if (end == line || *end != '\0' || errno == ERANGE || parsed < INT_MIN || parsed > INT_MAX) {
        printf("Error: Please enter a valid integer.\n"); return -1;
    }
    *value = (int)parsed; return 1;
}
static int process_command(Core *core, int choice)
{
    int a, b, result, status; char operation[32];
    switch (choice) {
    case 1:
        printf("Enter operation (ADD/SUB/MUL/DIV): ");
        if (fgets(operation, sizeof operation, stdin) == NULL) return 0;
        operation[strcspn(operation, "\r\n")] = '\0';
        for (a = 0; operation[a]; ++a) operation[a] = (char)toupper((unsigned char)operation[a]);
        status = read_integer("Enter first value: ", &a); if (status <= 0) return status == 0 ? 0 : 1;
        status = read_integer("Enter second value: ", &b); if (status <= 0) return status == 0 ? 0 : 1;
        if (!cpu_execute(operation, a, b, &result)) {
            if (strcmp(operation, "DIV") == 0 && b == 0) printf("Error: Division by zero.\n");
            else printf("Error: Unsupported CPU operation.\n");
        } else {
            printf("CPU executing...\nResult = %d\n", result);
            strcpy(core->last_operation, operation); core->last_result = result; core->has_cpu_result = 1;
        }
        break;
    case 2:
        status = read_integer("Enter memory address (0-99): ", &a); if (status <= 0) return status == 0 ? 0 : 1;
        status = read_integer("Enter value: ", &b); if (status <= 0) return status == 0 ? 0 : 1;
        if (!memory_write(&core->memory, a, b)) printf("Error: Invalid memory address.\n");
        else printf("Memory[%d] = %d\n", a, b);
        break;
    case 3:
        status = read_integer("Enter memory address (0-99): ", &a); if (status <= 0) return status == 0 ? 0 : 1;
        if (!memory_read(&core->memory, a, &result)) printf("Error: Invalid address or memory location is empty.\n");
        else printf("Memory[%d] = %d\n", a, result);
        break;
    case 4:
        status = read_integer("Enter value: ", &a); if (status <= 0) return status == 0 ? 0 : 1;
        if (!stack_push(&core->stack, a)) printf("Error: Stack overflow.\n"); else printf("%d pushed into stack.\n", a);
        break;
    case 5:
        if (!stack_pop(&core->stack, &result)) printf("Error: Stack underflow.\n"); else printf("%d popped from stack.\n", result);
        break;
    case 6:
        status = read_integer("Enter value: ", &a); if (status <= 0) return status == 0 ? 0 : 1;
        if (!queue_enqueue(&core->queue, a)) printf("Error: Queue overflow.\n"); else printf("%d added to queue.\n", a);
        break;
    case 7:
        if (!queue_dequeue(&core->queue, &result)) printf("Error: Queue underflow.\n"); else printf("%d removed from queue.\n", result);
        break;
    case 8:
        printf("\n========== CORE STATE ==========\nCPU:\n");
        if (core->has_cpu_result) printf("  Last operation: %s\n  Last result: %d\n", core->last_operation, core->last_result);
        else printf("  No operation executed yet.\n");
        printf("Memory:\n"); memory_display(&core->memory);
        printf("Stack (top first):\n"); stack_display(&core->stack);
        printf("Queue (front first):\n"); queue_display(&core->queue);
        printf("================================\n"); break;
    case 9: return 0;
    default: printf("Error: Invalid menu option. Choose 1-9.\n");
    }
    return 1;
}
int main(void)
{
    Core core = {0}; int choice, status, running = 1;
    memory_init(&core.memory); stack_init(&core.stack); queue_init(&core.queue);
    while (running) {
        printf("\n=================================\n       STUDENT 2 CORE PROCESS\n=================================\n"
               "1. Execute CPU Operation\n2. Write to Memory\n3. Read from Memory\n"
               "4. Push to Stack\n5. Pop from Stack\n6. Enqueue\n7. Dequeue\n"
               "8. Display Core State\n9. Exit\n");
        status = read_integer("Enter your choice: ", &choice);
        if (status == 0) break;
        if (status < 0) continue;
        running = process_command(&core, choice);
    }
    printf("Core Process exiting. Resources cleaned up.\n"); return 0;
}
