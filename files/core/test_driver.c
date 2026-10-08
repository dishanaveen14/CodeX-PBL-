/*
 * core/test_driver.c  –  unit tests for Core subsystems with Logger output.
 *
 * Each test exercises one subsystem and logs PASS / FAIL results.
 * The driver connects to the Logger (if running) and emits events for
 * every tested operation so the log doubles as a test audit trail.
 *
 * Build (from project root):
 *   gcc -Wall -Wextra -O2 \
 *       core/test_driver.c core/cpu.c core/memory.c \
 *       core/stack.c core/queue.c \
 *       -o build/test_driver -lrt
 *
 * Run:
 *   ./build/test_driver
 */
#define _GNU_SOURCE
#define CORE_LOG_IMPL    /* storage for g_log_q / g_log_ok */

#include <stdio.h>
#include <stdlib.h>

#include "core_log.h"
#include "cpu.h"
#include "memory.h"
#include "queue.h"
#include "stack.h"

/* ---- Simple PASS/FAIL counters ---- */
static int g_pass = 0, g_fail = 0;

static void check(const char *name, int cond)
{
    if (cond) {
        g_pass++;
        printf("  PASS  %s\n", name);
        core_log(LVL_INFO, "TEST PASS: %s", name);
    } else {
        g_fail++;
        printf("  FAIL  %s\n", name);
        core_log(LVL_ERROR, "TEST FAIL: %s", name);
    }
}

/* =========================================================================
 * CPU tests
 * ========================================================================= */
static void test_cpu(void)
{
    printf("\n[CPU tests]\n");
    core_log(LVL_INFO, "CPU tests: begin");

    cpu_t cpu;
    cpu_init(&cpu);
    cpu.reg[0] = 10;
    cpu.reg[1] = 3;

    /* ADD */
    int rc = cpu_execute(&cpu, CPU_OP_ADD, 2, 0, 1);
    core_log(LVL_INFO, "CPU ADD R2=R0+R1 rc=%d R2=%lld",
             rc, (long long)cpu.reg[2]);
    check("cpu_execute ADD result == 13", rc == 0 && cpu.reg[2] == 13);

    /* SUB */
    rc = cpu_execute(&cpu, CPU_OP_SUB, 3, 0, 1);
    core_log(LVL_INFO, "CPU SUB R3=R0-R1 rc=%d R3=%lld",
             rc, (long long)cpu.reg[3]);
    check("cpu_execute SUB result == 7", rc == 0 && cpu.reg[3] == 7);

    /* MUL */
    rc = cpu_execute(&cpu, CPU_OP_MUL, 2, 0, 1);
    core_log(LVL_INFO, "CPU MUL R2=R0*R1 rc=%d R2=%lld",
             rc, (long long)cpu.reg[2]);
    check("cpu_execute MUL result == 30", rc == 0 && cpu.reg[2] == 30);

    /* DIV */
    rc = cpu_execute(&cpu, CPU_OP_DIV, 3, 0, 1);
    core_log(LVL_INFO, "CPU DIV R3=R0/R1 rc=%d R3=%lld",
             rc, (long long)cpu.reg[3]);
    check("cpu_execute DIV result == 3", rc == 0 && cpu.reg[3] == 3);

    /* DIV by zero */
    cpu.reg[1] = 0;
    rc = cpu_execute(&cpu, CPU_OP_DIV, 2, 0, 1);
    core_log(LVL_WARN, "CPU DIV by zero rc=%d", rc);
    check("cpu_execute DIV by zero returns -2", rc == -2);

    /* Invalid register */
    rc = cpu_execute(&cpu, CPU_OP_ADD, 99, 0, 1);
    core_log(LVL_WARN, "CPU bad register rc=%d", rc);
    check("cpu_execute bad register returns -1", rc == -1);

    core_log(LVL_INFO, "CPU tests: end");
}

/* =========================================================================
 * Memory tests
 * ========================================================================= */
static void test_memory(void)
{
    printf("\n[Memory tests]\n");
    core_log(LVL_INFO, "Memory tests: begin");

    memory_t mem;
    memory_init(&mem);

    /* Valid write + read-back */
    int rc = memory_write(&mem, 0, 0xCAFE);
    core_log(LVL_INFO, "MEM WRITE addr=0 val=0xCAFE rc=%d", rc);
    check("memory_write valid returns 0", rc == 0);

    int64_t got = 0;
    rc = memory_read(&mem, 0, &got);
    core_log(LVL_INFO, "MEM READ  addr=0 val=%lld rc=%d", (long long)got, rc);
    check("memory_read valid returns 0xCAFE", rc == 0 && got == 0xCAFE);

    /* Out-of-bounds */
    rc = memory_write(&mem, MEM_SIZE_BYTES, 1);
    core_log(LVL_WARN, "MEM WRITE OOB addr=%zu rc=%d", (size_t)MEM_SIZE_BYTES, rc);
    check("memory_write OOB returns -1", rc == -1);

    /* Misaligned */
    rc = memory_write(&mem, 3, 1);
    core_log(LVL_WARN, "MEM WRITE misaligned addr=3 rc=%d", rc);
    check("memory_write misaligned returns -1", rc == -1);

    core_log(LVL_INFO, "Memory tests: end");
}

/* =========================================================================
 * Stack tests
 * ========================================================================= */
static void test_stack(void)
{
    printf("\n[Stack tests]\n");
    core_log(LVL_INFO, "Stack tests: begin");

    sim_stack_t st;
    stack_init(&st);

    check("stack_empty after init", stack_empty(&st));

    int rc = stack_push(&st, 42);
    core_log(LVL_INFO, "STACK PUSH 42 rc=%d depth=%zu", rc, stack_depth(&st));
    check("stack_push 42 returns 0", rc == 0);

    rc = stack_push(&st, 99);
    core_log(LVL_INFO, "STACK PUSH 99 rc=%d depth=%zu", rc, stack_depth(&st));
    check("stack_push 99 returns 0", rc == 0);

    int64_t out = 0;
    rc = stack_pop(&st, &out);
    core_log(LVL_INFO, "STACK POP  -> %lld rc=%d depth=%zu",
             (long long)out, rc, stack_depth(&st));
    check("stack_pop returns 99 (LIFO)", rc == 0 && out == 99);

    rc = stack_pop(&st, &out);
    core_log(LVL_INFO, "STACK POP  -> %lld rc=%d depth=%zu",
             (long long)out, rc, stack_depth(&st));
    check("stack_pop returns 42", rc == 0 && out == 42);

    /* Underflow */
    rc = stack_pop(&st, &out);
    core_log(LVL_WARN, "STACK POP  underflow rc=%d", rc);
    check("stack_pop underflow returns -1", rc == -1);

    core_log(LVL_INFO, "Stack tests: end");
}

/* =========================================================================
 * Queue tests
 * ========================================================================= */
static void test_queue(void)
{
    printf("\n[Queue tests]\n");
    core_log(LVL_INFO, "Queue tests: begin");

    queue_t qu;
    queue_init(&qu);

    check("queue_empty after init", queue_empty(&qu));

    int rc = queue_enqueue(&qu, 10);
    core_log(LVL_INFO, "QUEUE ENQ  10 rc=%d count=%zu", rc, queue_count(&qu));
    check("queue_enqueue 10 returns 0", rc == 0);

    rc = queue_enqueue(&qu, 20);
    core_log(LVL_INFO, "QUEUE ENQ  20 rc=%d count=%zu", rc, queue_count(&qu));
    check("queue_enqueue 20 returns 0", rc == 0);

    int64_t out = 0;
    rc = queue_dequeue(&qu, &out);
    core_log(LVL_INFO, "QUEUE DEQ  -> %lld rc=%d count=%zu",
             (long long)out, rc, queue_count(&qu));
    check("queue_dequeue returns 10 (FIFO)", rc == 0 && out == 10);

    rc = queue_dequeue(&qu, &out);
    core_log(LVL_INFO, "QUEUE DEQ  -> %lld rc=%d count=%zu",
             (long long)out, rc, queue_count(&qu));
    check("queue_dequeue returns 20", rc == 0 && out == 20);

    /* Underflow */
    rc = queue_dequeue(&qu, &out);
    core_log(LVL_WARN, "QUEUE DEQ  underflow rc=%d", rc);
    check("queue_dequeue empty returns -1", rc == -1);

    core_log(LVL_INFO, "Queue tests: end");
}

/* =========================================================================
 * main
 * ========================================================================= */
int main(void)
{
    printf("=== Core unit tests ===\n");

    core_log_init();   /* connect to Logger; no-op if absent */
    core_log(LVL_INFO, "test_driver: start");

    test_cpu();
    test_memory();
    test_stack();
    test_queue();

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    core_log(LVL_INFO, "test_driver: complete pass=%d fail=%d", g_pass, g_fail);

    core_log_close();
    return (g_fail > 0) ? 1 : 0;
}
