#ifndef CPU_H
#define CPU_H

/* Execute ADD, SUB, MUL, or DIV. Returns 1 on success and 0 on error. */
int cpu_execute(const char *operation, int a, int b, int *result);

#endif
