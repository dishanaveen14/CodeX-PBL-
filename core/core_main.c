#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdint.h>

#define CORE_LOG_IMPL
#include "core_log.h"

/* -------------------------------------------------------------------------
 * Utility functions for reliable pipe I/O
 * ------------------------------------------------------------------------- */
static int write_all(int fd, const void *buf, size_t size)
{
    const char *p = buf;
    size_t written = 0;
    while (written < size) {
        ssize_t res = write(fd, p + written, size - written);
        if (res < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        written += (size_t)res;
    }
    return 0;
}

static int read_all(int fd, void *buf, size_t size)
{
    char *p = buf;
    size_t read_bytes = 0;
    while (read_bytes < size) {
        ssize_t res = read(fd, p + read_bytes, size - read_bytes);
        if (res < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (res == 0) return -1; /* EOF before full size */
        read_bytes += (size_t)res;
    }
    return 0;
}

/* =========================================================================
 * Fork + POSIX-pipe IPC addition demo
 * ========================================================================= */
static long long run_ipc_addition(long long number1, long long number2, int *err)
{
    core_log(LVL_INFO, "IPC addition: start (num1=%lld num2=%lld)", number1, number2);

    int channel[2];
    if (pipe(channel) < 0) {
        core_log(LVL_ERROR, "IPC addition: pipe() failed (%s)", strerror(errno));
        *err = 1;
        return 0;
    }

    pid_t child = fork();
    if (child < 0) {
        core_log(LVL_ERROR, "IPC addition: fork() failed (%s)", strerror(errno));
        close(channel[0]); close(channel[1]);
        *err = 1;
        return 0;
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
        close(channel[0]); waitpid(child, NULL, 0); 
        *err = 1;
        return 0;
    }
    close(channel[0]);

    long long sum = number1 + received;
    core_log(LVL_INFO, "IPC addition: sum = %lld + %lld = %lld", number1, received, sum);

    int status = 0;
    if (waitpid(child, &status, 0) < 0) {
        core_log(LVL_ERROR, "IPC addition: waitpid failed (%s)", strerror(errno));
        *err = 1;
        return 0;
    }
    
    if (WIFEXITED(status) && WEXITSTATUS(status) == EXIT_SUCCESS) {
        core_log(LVL_INFO, "IPC addition: child exited cleanly");
    } else {
        core_log(LVL_WARN, "IPC addition: child did not exit cleanly (status=0x%x)", status);
    }

    core_log(LVL_INFO, "IPC addition: complete");
    *err = 0;
    return sum;
}

/* =========================================================================
 * Command Loop (UI Bridge)
 * ========================================================================= */
static void process_command(const char *cmd_line)
{
    char cmd[32];
    if (sscanf(cmd_line, "%31s", cmd) != 1) return;

    if (strcmp(cmd, "PING") == 0) {
        printf("OK PONG\n");
    }
    else if (strcmp(cmd, "IPC_ADD") == 0) {
        long long num1, num2;
        if (sscanf(cmd_line, "%*s %lld %lld", &num1, &num2) == 2) {
            int err = 0;
            long long sum = run_ipc_addition(num1, num2, &err);
            if (!err) {
                printf("OK %lld\n", sum);
            } else {
                printf("ERROR IPC failure\n");
            }
        } else {
            printf("ERROR Invalid arguments\n");
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

    /* Interactive loop via stdin */
    char line[256];
    while (fgets(line, sizeof(line), stdin)) {
        process_command(line);
    }

    core_log(LVL_INFO, "Core shutdown");
    core_log_close();

    return EXIT_SUCCESS;
}
