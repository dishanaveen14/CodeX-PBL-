/*
 * core/cpu.h  –  CPU-simulation API exposed to core_main.c
 *
 * Simulates a tiny register-based CPU with four 64-bit registers (R0–R3)
 * and the four basic arithmetic operations.
 */
#ifndef CPU_H
#define CPU_H

#include <stdint.h>

/* Number of simulated registers. */
#define CPU_NREGS 4

/* Simulated CPU state. */
typedef struct {
    int64_t reg[CPU_NREGS];   /* R0 … R3 */
} cpu_t;

/* Supported ALU operations. */
typedef enum {
    CPU_OP_ADD = 0,
    CPU_OP_SUB,
    CPU_OP_MUL,
    CPU_OP_DIV,
} cpu_op_t;

/* Initialise (zero) all registers. */
void cpu_init(cpu_t *cpu);

/*
 * Execute one ALU instruction:  dst = src_a  op  src_b
 *
 * dst, src_a, src_b must be in [0, CPU_NREGS).
 * Returns  0 on success,
 *         -1 on invalid register index,
 *         -2 on division by zero.
 *
 * On error the destination register is left unchanged.
 */
int cpu_execute(cpu_t *cpu, cpu_op_t op,
                int dst, int src_a, int src_b);

/* Return a human-readable name for an operation ("ADD", "DIV", …). */
const char *cpu_op_name(cpu_op_t op);

#endif /* CPU_H */
