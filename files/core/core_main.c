/*
 * core/core_main.c  –  Core process entry point.
 *
 * Handles:
 *   • CPU register operations (ADD / SUB / MUL / DIV)
 *   • Memory read / write
 *   • Stack push / pop
 *   • Queue enqueue / dequeue
 *   • Fork + POSIX-pipe IPC addition demo  (run_ipc_addition)
 *
 * Logger integration
 * ------------------
 * All Logger calls go through core_log() from core_log.h.
 * Business-logic modules (cpu, memory, stack, queue) are kept free of
 * logging so each event is recorded at exactly one site here.
 *
 * Build (from project root):
 *   gcc -Wall -Wextra -O2 -pthread \
 *       core/core_main.c core/cpu.c core/memory.c \
 *       core/stack.c core/queue.c \
 *       -o build/core -lrt
 */
#define _GNU_SOURCE
#define CORE_LOG_IMPL     /* define storage for g_log_q / g_log_ok */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "core_log.h"   /* Logger façade – must come before other core hdrs */
#include "cpu.h"
#include "memory.h"
#include "queue.h"
#include "stack.h"

/* =========================================================================
 * Helpers for the POSIX-pipe IPC addition demo (mirrors ipc_addition.c
 * from the CORE/ sub-tree but integrated with Logger).
 * ========================================================================= */

/* Write exactly `size` bytes, retrying on EINTR. */
static int write_all(int fd, const void *buf, size_t size)
{
    const char *p = buf;
    size_t done = 0;
    while (done < size) {
        ssize_t r = write(fd, p + done, size - done);
        if (r < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        done += (size_t)r;
    }
    return 0;
}

/* Read exactly `size` bytes, retrying on EINTR. */
static int read_all(int fd, void *buf, size_t size)
{
    char *p = buf;
    size_t done = 0;
    while (done < size) {
        ssize_t r = read(fd, p + done, size - done);
        if (r < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (r == 0) return -1;   /* unexpected EOF */
        done += (size_t)r;
    }
    return 0;
}

/* =========================================================================
 * CPU demo
 * ========================================================================= */
static void run_cpu_demo(cpu_t *cpu)
{
    core_log(LVL_INFO, "CPU demo: starting with R0=10 R1=3");

    cpu->reg[0] = 10;
    cpu->reg[1] = 3;

    /* ADD R2 = R0 + R1 */
    int rc = cpu_execute(cpu, CPU_OP_ADD, 2, 0, 1);
    if (rc == 0)
        core_log(LVL_INFO, "CPU ADD R2=R0+R1 -> R2=%lld",
                 (long long)cpu->reg[2]);
    else
        core_log(LVL_ERROR, "CPU ADD failed (rc=%d)", rc);

    /* SUB R3 = R0 - R1 */
    rc = cpu_execute(cpu, CPU_OP_SUB, 3, 0, 1);
    if (rc == 0)
        core_log(LVL_INFO, "CPU SUB R3=R0-R1 -> R3=%lld",
                 (long long)cpu->reg[3]);
    else
        core_log(LVL_ERROR, "CPU SUB failed (rc=%d)", rc);

    /* MUL R2 = R0 * R1 */
    rc = cpu_execute(cpu, CPU_OP_MUL, 2, 0, 1);
    if (rc == 0)
        core_log(LVL_INFO, "CPU MUL R2=R0*R1 -> R2=%lld",
                 (long long)cpu->reg[2]);
    else
        core_log(LVL_ERROR, "CPU MUL failed (rc=%d)", rc);

    /* DIV R3 = R0 / R1 */
    rc = cpu_execute(cpu, CPU_OP_DIV, 3, 0, 1);
    if (rc == 0)
        core_log(LVL_INFO, "CPU DIV R3=R0/R1 -> R3=%lld",
                 (long long)cpu->reg[3]);
    else
        core_log(LVL_ERROR, "CPU DIV failed (rc=%d)", rc);

    /* DIV by zero: R1=0, attempt R0/R1 */
    cpu->reg[1] = 0;
    rc = cpu_execute(cpu, CPU_OP_DIV, 2, 0, 1);
    if (rc == -2)
        core_log(LVL_WARN, "CPU DIV by zero rejected (R1=0)");
    else if (rc != 0)
        core_log(LVL_ERROR, "CPU DIV unexpected error rc=%d", rc);

    /* Invalid register */
    rc = cpu_execute(cpu, CPU_OP_ADD, 99, 0, 1);
    if (rc == -1)
        core_log(LVL_WARN, "CPU invalid register index rejected (dst=99)");
    else
        core_log(LVL_ERROR, "CPU bad-register check failed (rc=%d)", rc);

    core_log(LVL_INFO, "CPU demo: complete");
}

/* =========================================================================
 * Memory demo
 * ========================================================================= */
static void run_memory_demo(memory_t *mem)
{
    core_log(LVL_INFO, "Memory demo: starting");

    /* Valid write */
    size_t addr = 0;
    int64_t val = 0xDEAD;
    int rc = memory_write(mem, addr, val);
    if (rc == 0)
        core_log(LVL_INFO, "MEM WRITE addr=0x%04zx val=%lld OK",
                 addr, (long long)val);
    else
        core_log(LVL_ERROR, "MEM WRITE addr=0x%04zx FAILED", addr);

    /* Valid read-back */
    int64_t got = 0;
    rc = memory_read(mem, addr, &got);
    if (rc == 0)
        core_log(LVL_INFO, "MEM READ  addr=0x%04zx val=%lld OK",
                 addr, (long long)got);
    else
        core_log(LVL_ERROR, "MEM READ  addr=0x%04zx FAILED", addr);

    /* Out-of-bounds write */
    addr = MEM_SIZE_BYTES;   /* one byte past the end */
    rc = memory_write(mem, addr, 42);
    if (rc == -1)
        core_log(LVL_WARN, "MEM WRITE out-of-bounds addr=0x%04zx rejected", addr);
    else
        core_log(LVL_ERROR, "MEM WRITE bounds check failed for addr=0x%04zx", addr);

    /* Misaligned write */
    addr = 3;
    rc = memory_write(mem, addr, 7);
    if (rc == -1)
        core_log(LVL_WARN, "MEM WRITE misaligned addr=0x%04zx rejected", addr);
    else
        core_log(LVL_ERROR, "MEM WRITE alignment check failed for addr=0x%04zx", addr);

    core_log(LVL_INFO, "Memory demo: complete");
}

/* =========================================================================
 * Stack demo
 * ========================================================================= */
static void run_stack_demo(sim_stack_t *st)
{
    core_log(LVL_INFO, "Stack demo: starting (capacity=%d)", STACK_CAPACITY);

    /* Push three values */
    int64_t vals[] = { 100, 200, 300 };
    for (int i = 0; i < 3; i++) {
        int rc = stack_push(st, vals[i]);
        if (rc == 0)
            core_log(LVL_INFO, "STACK PUSH %lld OK (depth=%zu)",
                     (long long)vals[i], stack_depth(st));
        else
            core_log(LVL_ERROR, "STACK PUSH %lld FAILED (overflow)",
                     (long long)vals[i]);
    }

    /* Pop all three */
    for (int i = 0; i < 3; i++) {
        int64_t out = 0;
        int rc = stack_pop(st, &out);
        if (rc == 0)
            core_log(LVL_INFO, "STACK POP  -> %lld OK (depth=%zu)",
                     (long long)out, stack_depth(st));
        else
            core_log(LVL_ERROR, "STACK POP  FAILED (underflow)");
    }

    /* Underflow */
    int64_t dummy = 0;
    int rc = stack_pop(st, &dummy);
    if (rc == -1)
        core_log(LVL_WARN, "STACK POP  underflow rejected (stack empty)");
    else
        core_log(LVL_ERROR, "STACK underflow check failed");

    core_log(LVL_INFO, "Stack demo: complete");
}

/* =========================================================================
 * Queue demo
 * ========================================================================= */
static void run_queue_demo(queue_t *qu)
{
    core_log(LVL_INFO, "Queue demo: starting (capacity=%d)", QUEUE_CAPACITY);

    /* Enqueue three values */
    int64_t vals[] = { 11, 22, 33 };
    for (int i = 0; i < 3; i++) {
        int rc = queue_enqueue(qu, vals[i]);
        if (rc == 0)
            core_log(LVL_INFO, "QUEUE ENQ  %lld OK (count=%zu)",
                     (long long)vals[i], queue_count(qu));
        else
            core_log(LVL_ERROR, "QUEUE ENQ  %lld FAILED (full)",
                     (long long)vals[i]);
    }

    /* Dequeue all three */
    for (int i = 0; i < 3; i++) {
        int64_t out = 0;
        int rc = queue_dequeue(qu, &out);
        if (rc == 0)
            core_log(LVL_INFO, "QUEUE DEQ  -> %lld OK (count=%zu)",
                     (long long)out, queue_count(qu));
        else
            core_log(LVL_ERROR, "QUEUE DEQ  FAILED (empty)");
    }

    /* Underflow */
    int64_t dummy = 0;
    int rc = queue_dequeue(qu, &dummy);
    if (rc == -1)
        core_log(LVL_WARN, "QUEUE DEQ  underflow rejected (queue empty)");
    else
        core_log(LVL_ERROR, "QUEUE underflow check failed");

    core_log(LVL_INFO, "Queue demo: complete");
}

/* =========================================================================
 * Fork + POSIX-pipe IPC addition demo
 *
 * Parent (Process 1) supplies number1.
 * Child  (Process 2) supplies number2 and sends it to the parent via a pipe.
 * Parent computes and logs the sum.
 * ========================================================================= */
static void run_ipc_addition(long long number1, long long number2)
{
    core_log(LVL_INFO,
             "IPC addition: start (num1=%lld num2=%lld)",
             number1, number2);

    int channel[2];
    if (pipe(channel) < 0) {
        core_log(LVL_ERROR, "IPC addition: pipe() failed (%s)", strerror(errno));
        return;
    }

    pid_t child = fork();
    if (child < 0) {
        core_log(LVL_ERROR, "IPC addition: fork() failed (%s)", strerror(errno));
        close(channel[0]);
        close(channel[1]);
        return;
    }

    if (child == 0) {
        /* ---- Child process (Process 2) ---- */
        close(channel[0]);   /* close unused read end */

        core_log(LVL_INFO,
                 "IPC addition: child PID=%ld sending num2=%lld via pipe",
                 (long)getpid(), number2);

        if (write_all(channel[1], &number2, sizeof number2) < 0) {
            core_log(LVL_ERROR,
                     "IPC addition: child pipe write failed (%s)",
                     strerror(errno));
            close(channel[1]);
            _exit(EXIT_FAILURE);
        }

        core_log(LVL_INFO,
                 "IPC addition: child PID=%ld pipe send OK, exiting",
                 (long)getpid());
        close(channel[1]);
        _exit(EXIT_SUCCESS);
    }

    /* ---- Parent process (Process 1) ---- */
    close(channel[1]);   /* close unused write end */

    core_log(LVL_INFO,
             "IPC addition: parent PID=%ld waiting to receive num2 from child PID=%ld",
             (long)getpid(), (long)child);

    long long received = 0;
    if (read_all(channel[0], &received, sizeof received) < 0) {
        core_log(LVL_ERROR,
                 "IPC addition: parent pipe read failed (%s)", strerror(errno));
        close(channel[0]);
        waitpid(child, NULL, 0);
        return;
    }
    close(channel[0]);

    core_log(LVL_INFO,
             "IPC addition: parent received num2=%lld from pipe", received);

    long long sum = number1 + received;
    core_log(LVL_INFO,
             "IPC addition: sum = %lld + %lld = %lld",
             number1, received, sum);

    /* Reap child. */
    int status = 0;
    if (waitpid(child, &status, 0) < 0) {
        core_log(LVL_ERROR, "IPC addition: waitpid failed (%s)", strerror(errno));
        return;
    }
    if (WIFEXITED(status) && WEXITSTATUS(status) == EXIT_SUCCESS)
        core_log(LVL_INFO, "IPC addition: child exited cleanly");
    else
        core_log(LVL_WARN, "IPC addition: child did not exit cleanly (status=0x%x)",
                 status);

    core_log(LVL_INFO, "IPC addition: complete");
}

/* =========================================================================
 * main
 * ========================================================================= */
int main(void)
{
    /* ---- Logger connection ---- */
    core_log_init();   /* graceful no-op if Logger is not running */
    core_log(LVL_INFO, "Core startup");

    /* ---- Subsystem initialisation ---- */
    cpu_t    cpu;
    memory_t mem;
    sim_stack_t  st;
    queue_t  qu;

    cpu_init(&cpu);
    memory_init(&mem);
    stack_init(&st);
    queue_init(&qu);
    core_log(LVL_INFO, "Core subsystems initialised (CPU, memory, stack, queue)");

    /* ---- Run demos ---- */
    run_cpu_demo(&cpu);
    run_memory_demo(&mem);
    run_stack_demo(&st);
    run_queue_demo(&qu);
    run_ipc_addition(7, 5);

    /* ---- Shutdown ---- */
    core_log(LVL_INFO, "Core shutdown");
    core_log_close();

    return EXIT_SUCCESS;
}
