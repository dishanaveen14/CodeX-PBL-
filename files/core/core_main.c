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
 * Command Loop (UI Bridge)
 * ========================================================================= */
static cpu_t    g_cpu;
static memory_t g_mem;
static sim_stack_t  g_st;
static queue_t  g_qu;

/* =========================================================================
 * Fork + POSIX-pipe IPC addition demo
 * ========================================================================= */
static void run_ipc_addition(long long number1, long long number2)
{
    core_log(LVL_INFO, "IPC addition: start (num1=%lld num2=%lld)", number1, number2);

    int channel[2];
    if (pipe(channel) < 0) {
        core_log(LVL_ERROR, "IPC addition: pipe() failed (%s)", strerror(errno));
        return;
    }

    pid_t child = fork();
    if (child < 0) {
        core_log(LVL_ERROR, "IPC addition: fork() failed (%s)", strerror(errno));
        close(channel[0]); close(channel[1]);
        return;
    }

    if (child == 0) {
        /* Child */
        close(channel[0]);
        core_log(LVL_INFO, "IPC addition: child PID=%ld sending num2=%lld via pipe", (long)getpid(), number2);
        if (write_all(channel[1], &number2, sizeof number2) < 0) {
            core_log(LVL_ERROR, "IPC addition: child pipe write failed (%s)", strerror(errno));
            close(channel[1]); _exit(EXIT_FAILURE);
        }
        core_log(LVL_INFO, "IPC addition: child PID=%ld pipe send OK, exiting", (long)getpid());
        close(channel[1]); _exit(EXIT_SUCCESS);
    }

    /* Parent */
    close(channel[1]);
    core_log(LVL_INFO, "IPC addition: parent PID=%ld waiting for child PID=%ld", (long)getpid(), (long)child);

    long long received = 0;
    if (read_all(channel[0], &received, sizeof received) < 0) {
        core_log(LVL_ERROR, "IPC addition: parent pipe read failed (%s)", strerror(errno));
        close(channel[0]); waitpid(child, NULL, 0); return;
    }
    close(channel[0]);

    long long sum = number1 + received;
    core_log(LVL_INFO, "IPC addition: sum = %lld + %lld = %lld", number1, received, sum);

    int status = 0;
    if (waitpid(child, &status, 0) < 0) {
        core_log(LVL_ERROR, "IPC addition: waitpid failed (%s)", strerror(errno));
        return;
    }
    if (WIFEXITED(status) && WEXITSTATUS(status) == EXIT_SUCCESS)
        core_log(LVL_INFO, "IPC addition: child exited cleanly");
    else
        core_log(LVL_WARN, "IPC addition: child did not exit cleanly (status=0x%x)", status);

    core_log(LVL_INFO, "IPC addition: complete");
}

static void process_command(const char *cmd_line)
{
    char cmd[32];
    if (sscanf(cmd_line, "%31s", cmd) != 1) return;

    if (strcmp(cmd, "PING") == 0) {
        printf("OK PONG\n");
    }
    else if (strcmp(cmd, "GET_STATE") == 0) {
        printf("OK R0=%lld R1=%lld R2=%lld R3=%lld STACK_DEPTH=%zu QUEUE_COUNT=%zu\n",
               (long long)g_cpu.reg[0], (long long)g_cpu.reg[1],
               (long long)g_cpu.reg[2], (long long)g_cpu.reg[3],
               stack_depth(&g_st), queue_count(&g_qu));
    }
    else if (strcmp(cmd, "SET_REG") == 0) {
        int reg;
        long long val;
        if (sscanf(cmd_line, "%*s %d %lld", &reg, &val) == 2 && reg >= 0 && reg < 4) {
            g_cpu.reg[reg] = val;
            core_log(LVL_INFO, "UI SET_REG R%d = %lld", reg, val);
            printf("OK R%d=%lld\n", reg, val);
        } else {
            printf("ERROR Invalid SET_REG args\n");
        }
    }
    else if (strcmp(cmd, "CPU") == 0) {
        char op_str[16];
        int dst, srcA, srcB;
        if (sscanf(cmd_line, "%*s %15s %d %d %d", op_str, &dst, &srcA, &srcB) == 4) {
            cpu_op_t op;
            if (strcmp(op_str, "ADD") == 0) op = CPU_OP_ADD;
            else if (strcmp(op_str, "SUB") == 0) op = CPU_OP_SUB;
            else if (strcmp(op_str, "MUL") == 0) op = CPU_OP_MUL;
            else if (strcmp(op_str, "DIV") == 0) op = CPU_OP_DIV;
            else { printf("ERROR Unknown CPU OP\n"); return; }
            
            int rc = cpu_execute(&g_cpu, op, dst, srcA, srcB);
            if (rc == 0) {
                core_log(LVL_INFO, "CPU %s R%d=R%d,R%d -> R%d=%lld",
                         op_str, dst, srcA, srcB, dst, (long long)g_cpu.reg[dst]);
                printf("OK R%d=%lld\n", dst, (long long)g_cpu.reg[dst]);
            } else if (rc == -2) {
                core_log(LVL_WARN, "CPU %s by zero rejected", op_str);
                printf("ERROR Division by zero\n");
            } else {
                core_log(LVL_WARN, "CPU %s invalid register", op_str);
                printf("ERROR Invalid register\n");
            }
        } else {
            printf("ERROR Invalid CPU args\n");
        }
    }
    else if (strcmp(cmd, "MEM_READ") == 0) {
        size_t addr;
        if (sscanf(cmd_line, "%*s %zu", &addr) == 1) {
            int64_t val;
            int rc = memory_read(&g_mem, addr, &val);
            if (rc == 0) {
                core_log(LVL_INFO, "MEM READ addr=0x%04zx val=%lld", addr, (long long)val);
                printf("OK %lld\n", (long long)val);
            } else {
                core_log(LVL_WARN, "MEM READ addr=0x%04zx failed", addr);
                printf("ERROR Bounds or alignment\n");
            }
        }
    }
    else if (strcmp(cmd, "MEM_WRITE") == 0) {
        size_t addr;
        long long val;
        if (sscanf(cmd_line, "%*s %zu %lld", &addr, &val) == 2) {
            int rc = memory_write(&g_mem, addr, val);
            if (rc == 0) {
                core_log(LVL_INFO, "MEM WRITE addr=0x%04zx val=%lld", addr, val);
                printf("OK\n");
            } else {
                core_log(LVL_WARN, "MEM WRITE addr=0x%04zx failed", addr);
                printf("ERROR Bounds or alignment\n");
            }
        }
    }
    else if (strcmp(cmd, "STACK_PUSH") == 0) {
        long long val;
        if (sscanf(cmd_line, "%*s %lld", &val) == 1) {
            if (stack_push(&g_st, val) == 0) {
                core_log(LVL_INFO, "STACK PUSH %lld", val);
                printf("OK\n");
            } else {
                core_log(LVL_WARN, "STACK PUSH overflow");
                printf("ERROR Stack overflow\n");
            }
        }
    }
    else if (strcmp(cmd, "STACK_POP") == 0) {
        int64_t val;
        if (stack_pop(&g_st, &val) == 0) {
            core_log(LVL_INFO, "STACK POP -> %lld", (long long)val);
            printf("OK %lld\n", (long long)val);
        } else {
            core_log(LVL_WARN, "STACK POP underflow");
            printf("ERROR Stack underflow\n");
        }
    }
    else if (strcmp(cmd, "QUEUE_ENQ") == 0) {
        long long val;
        if (sscanf(cmd_line, "%*s %lld", &val) == 1) {
            if (queue_enqueue(&g_qu, val) == 0) {
                core_log(LVL_INFO, "QUEUE ENQ %lld", val);
                printf("OK\n");
            } else {
                core_log(LVL_WARN, "QUEUE ENQ overflow");
                printf("ERROR Queue full\n");
            }
        }
    }
    else if (strcmp(cmd, "QUEUE_DEQ") == 0) {
        int64_t val;
        if (queue_dequeue(&g_qu, &val) == 0) {
            core_log(LVL_INFO, "QUEUE DEQ -> %lld", (long long)val);
            printf("OK %lld\n", (long long)val);
        } else {
            core_log(LVL_WARN, "QUEUE DEQ underflow");
            printf("ERROR Queue empty\n");
        }
    }
    else if (strcmp(cmd, "IPC_ADD") == 0) {
        long long num1, num2;
        if (sscanf(cmd_line, "%*s %lld %lld", &num1, &num2) == 2) {
            run_ipc_addition(num1, num2);
            /* run_ipc_addition logs it, we just tell UI it finished */
            printf("OK IPC Finished\n");
        }
    }
    else {
        printf("ERROR Unknown command\n");
    }
    fflush(stdout);
}

/* =========================================================================
 * main
 * ========================================================================= */
int main(void)
{
    core_log_init();   /* connect to Logger; no-op if absent */
    core_log(LVL_INFO, "Core startup");

    cpu_init(&g_cpu);
    memory_init(&g_mem);
    stack_init(&g_st);
    queue_init(&g_qu);
    core_log(LVL_INFO, "Core subsystems initialised");

    /* Default to interactive loop via stdin */
    char line[256];
    while (fgets(line, sizeof(line), stdin)) {
        process_command(line);
    }

    core_log(LVL_INFO, "Core shutdown");
    core_log_close();

    return EXIT_SUCCESS;
}
