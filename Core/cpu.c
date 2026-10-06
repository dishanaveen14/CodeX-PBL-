#include "cpu.h"
#include <string.h>

int cpu_execute(const char *operation, int a, int b, int *result)
{
    if (operation == NULL || result == NULL) return 0;
    if (strcmp(operation, "ADD") == 0) *result = a + b;
    else if (strcmp(operation, "SUB") == 0) *result = a - b;
    else if (strcmp(operation, "MUL") == 0) *result = a * b;
    else if (strcmp(operation, "DIV") == 0) {
        if (b == 0) return 0;
        *result = a / b;
    } else return 0;
    return 1;
}
