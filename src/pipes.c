/**
 * @file pipes.c
 * @brief Implementation of POSIX Anonymous Pipe IPC Mechanism with Detailed Telemetry.
 *
 * Owned by: Student 1 (Pipes and Message Queues)
 * Course: Operating Systems - POSIX IPC Project
 */

#include "pipes.h"
#include "ipc_logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
  #include <windows.h>
  #include <io.h>
  #include <fcntl.h>
  #include <pthread.h>
#else
  #include <unistd.h>
  #include <sys/types.h>
  #include <sys/wait.h>
#endif

#define PIPE_BUF_LEN 512

#ifdef _WIN32
typedef struct {
    int write_fd;
    long long num2;
    char tag[64];
} pipe_win_child_arg_t;

static void *win_pipe_child_thread(void *arg)
{
    pipe_win_child_arg_t *carg = (pipe_win_child_arg_t *)arg;
    ipc_log(LOG_LVL_INFO, "PIPES", "Child Context (Thread %lu): Initiating pipe write...", (unsigned long)GetCurrentThreadId());
    ipc_log(LOG_LVL_INFO, "PIPES", "Child Context: Writing payload (num2=%lld, tag='%s') into Write FD %d",
            carg->num2, carg->tag, carg->write_fd);

    _write(carg->write_fd, &carg->num2, sizeof(carg->num2));
    _close(carg->write_fd);

    ipc_log(LOG_LVL_INFO, "PIPES", "Child Context: Write complete, closing Write FD and terminating.");
    free(carg);
    return NULL;
}
#endif

int execute_pipes_demo(long long num1, long long num2, const char *custom_tag, char *output_buffer, size_t buffer_size)
{
    if (output_buffer && buffer_size > 0) {
        ipc_log_set_capture(output_buffer, buffer_size);
    }

    const char *tag = (custom_tag && strlen(custom_tag) > 0) ? custom_tag : "DEFAULT_TRANSACTION";

    ipc_log(LOG_LVL_INFO, "PIPES", "========================================================");
    ipc_log(LOG_LVL_INFO, "PIPES", "STARTING POSIX PIPES INTER-PROCESS COMMUNICATION DEMO");
    ipc_log(LOG_LVL_INFO, "PIPES", "User Inputs: num1=%lld (Parent), num2=%lld (Child), tag='%s'", num1, num2, tag);
    ipc_log(LOG_LVL_INFO, "PIPES", "Step 1: Invoking pipe() system call to allocate kernel FIFO ring buffer...");

#ifndef _WIN32
    int fds[2];
    if (pipe(fds) == -1) {
        ipc_log(LOG_LVL_ERROR, "PIPES", "pipe() system call failed: %s", strerror(errno));
        ipc_log_stop_capture();
        return -1;
    }

    ipc_log(LOG_LVL_INFO, "PIPES", "Pipe created successfully. Read FD: %d, Write FD: %d (Capacity: 64 KB)", fds[0], fds[1]);
    ipc_log(LOG_LVL_INFO, "PIPES", "Step 2: Spawning child worker process via fork() system call...");

    pid_t parent_pid = getpid();
    pid_t child_pid = fork();

    if (child_pid < 0) {
        ipc_log(LOG_LVL_ERROR, "PIPES", "fork() system call failed: %s", strerror(errno));
        close(fds[0]);
        close(fds[1]);
        ipc_log_stop_capture();
        return -1;
    }

    if (child_pid == 0) {
        /* Child Process */
        pid_t my_pid = getpid();
        close(fds[0]); /* Enforce unidirectional flow by closing unused read end */

        ipc_log(LOG_LVL_INFO, "PIPES", "Child Process PID=%d: Closed unused Read FD %d", (int)my_pid, fds[0]);
        ipc_log(LOG_LVL_INFO, "PIPES", "Child Process PID=%d: Writing num2=%lld (%zu bytes) into pipe...",
                (int)my_pid, num2, sizeof(num2));

        ssize_t written = write(fds[1], &num2, sizeof(num2));
        close(fds[1]);

        if (written < 0) {
            ipc_log(LOG_LVL_ERROR, "PIPES", "Child PID=%d: write() failed: %s", (int)my_pid, strerror(errno));
            _exit(EXIT_FAILURE);
        }

        ipc_log(LOG_LVL_INFO, "PIPES", "Child Process PID=%d: Transmitted %zd bytes, closed Write FD, exiting.",
                (int)my_pid, written);
        _exit(EXIT_SUCCESS);
    }

    /* Parent Process */
    close(fds[1]); /* Close unused write end */
    ipc_log(LOG_LVL_INFO, "PIPES", "Parent Process PID=%d: Closed unused Write FD %d", (int)parent_pid, fds[1]);
    ipc_log(LOG_LVL_INFO, "PIPES", "Parent Process PID=%d: Waiting for Child PID=%d via blocking read() and waitpid()...",
            (int)parent_pid, (int)child_pid);

    long long received_num2 = 0;
    ssize_t bytes_read = read(fds[0], &received_num2, sizeof(received_num2));
    close(fds[0]);

    if (bytes_read <= 0) {
        ipc_log(LOG_LVL_ERROR, "PIPES", "Parent Process: read() returned error or premature EOF (%s)", strerror(errno));
        waitpid(child_pid, NULL, 0);
        ipc_log_stop_capture();
        return -1;
    }

    ipc_log(LOG_LVL_INFO, "PIPES", "Parent Process PID=%d: Read %zd bytes from kernel pipe. Received num2=%lld",
            (int)parent_pid, bytes_read, received_num2);

    long long sum = num1 + received_num2;
    ipc_log(LOG_LVL_INFO, "PIPES", "Parent Process: IPC Computation: %lld (Parent) + %lld (Child) = %lld",
            num1, received_num2, sum);

    int status = 0;
    waitpid(child_pid, &status, 0);
    ipc_log(LOG_LVL_INFO, "PIPES", "Parent Process: Reaped Child PID=%d cleanly with exit code %d.",
            (int)child_pid, WEXITSTATUS(status));
    ipc_log(LOG_LVL_INFO, "PIPES", "POSIX PIPES DEMONSTRATION COMPLETE: Final Sum = %lld", sum);
    ipc_log(LOG_LVL_INFO, "PIPES", "========================================================");

#else
    /* Windows MinGW Cross-Platform Implementation using _pipe() */
    int fds[2];
    if (_pipe(fds, PIPE_BUF_LEN, O_BINARY) != 0) {
        ipc_log(LOG_LVL_ERROR, "PIPES", "_pipe() failed: %s", strerror(errno));
        ipc_log_stop_capture();
        return -1;
    }

    ipc_log(LOG_LVL_INFO, "PIPES", "Kernel pipe created. Read FD: %d, Write FD: %d", fds[0], fds[1]);
    ipc_log(LOG_LVL_INFO, "PIPES", "Step 2: Spawning independent worker context for Child Process simulation...");

    pipe_win_child_arg_t *carg = (pipe_win_child_arg_t *)malloc(sizeof(pipe_win_child_arg_t));
    carg->write_fd = fds[1];
    carg->num2 = num2;
    strncpy(carg->tag, tag, sizeof(carg->tag) - 1);

    pthread_t child_tid;
    pthread_create(&child_tid, NULL, win_pipe_child_thread, carg);

    ipc_log(LOG_LVL_INFO, "PIPES", "Parent Context: Waiting for data on Read FD %d...", fds[0]);

    long long received_num2 = 0;
    int bytes_read = _read(fds[0], &received_num2, sizeof(received_num2));
    _close(fds[0]);

    pthread_join(child_tid, NULL);

    ipc_log(LOG_LVL_INFO, "PIPES", "Parent Context: Received %d bytes. num2=%lld", bytes_read, received_num2);
    long long sum = num1 + received_num2;
    ipc_log(LOG_LVL_INFO, "PIPES", "Parent Context: IPC Addition Complete: %lld + %lld = %lld", num1, received_num2, sum);
    ipc_log(LOG_LVL_INFO, "PIPES", "POSIX PIPES DEMONSTRATION COMPLETE: Final Sum = %lld", sum);
    ipc_log(LOG_LVL_INFO, "PIPES", "========================================================");
#endif

    ipc_log_stop_capture();
    return 0;
}

void run_pipes(void)
{
    printf("\n========================================================\n");
    printf("             STUDENT 1: POSIX PIPES MODULE              \n");
    printf("========================================================\n");

    long long n1 = 0, n2 = 0;
    char tag[64] = {0};

    printf("Enter Number 1 (held by Parent Process) [e.g. 15]: ");
    if (scanf("%lld", &n1) != 1) { n1 = 15; while (getchar() != '\n'); }

    printf("Enter Number 2 (held by Child Process)  [e.g. 25]: ");
    if (scanf("%lld", &n2) != 1) { n2 = 25; while (getchar() != '\n'); }

    printf("Enter transaction label tag (or press Enter) [e.g. SENSOR_A]: ");
    while (getchar() != '\n');
    if (fgets(tag, sizeof(tag), stdin)) {
        tag[strcspn(tag, "\r\n")] = 0;
    }
    if (strlen(tag) == 0) strcpy(tag, "PIPE_IPC_OP");

    printf("\n--- Executing POSIX Pipe IPC & Streaming Logs ---\n");
    execute_pipes_demo(n1, n2, tag, NULL, 0);
}
