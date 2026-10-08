/*
 * core/cpu.c  –  CPU simulation implementation.
 *
 * Business logic only; all logging is done by the caller (core_main.c)
 * using the core_log() façade so this file stays clean of IPC concerns.
 */
#define _GNU_SOURCE
#include <string.h>
#include "cpu.h"

void cpu_init(cpu_t *cpu)
{
    memset(cpu->reg, 0, sizeof cpu->reg);
}

int cpu_execute(cpu_t *cpu, cpu_op_t op, int dst, int src_a, int src_b)
{
    /* Validate register indices. */
    if (dst  < 0 || dst  >= CPU_NREGS ||
        src_a < 0 || src_a >= CPU_NREGS ||
        src_b < 0 || src_b >= CPU_NREGS)
        return -1;   /* invalid register */

    int64_t a = cpu->reg[src_a];
    int64_t b = cpu->reg[src_b];

    switch (op) {
    case CPU_OP_ADD: cpu->reg[dst] = a + b; break;
    case CPU_OP_SUB: cpu->reg[dst] = a - b; break;
    case CPU_OP_MUL: cpu->reg[dst] = a * b; break;
    case CPU_OP_DIV:
        if (b == 0) return -2;   /* division by zero */
        cpu->reg[dst] = a / b;
        break;
    default:
        return -1;   /* unknown operation */
    }
    return 0;
}

const char *cpu_op_name(cpu_op_t op)
{
    switch (op) {
    case CPU_OP_ADD: return "ADD";
    case CPU_OP_SUB: return "SUB";
    case CPU_OP_MUL: return "MUL";
    case CPU_OP_DIV: return "DIV";
    default:         return "???";
    }
}
