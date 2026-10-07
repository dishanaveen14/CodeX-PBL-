#include "fork_demo.h"
#include <stdio.h>

#if defined(__unix__) || defined(__APPLE__)
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int run_fork_demo(void)
{
    pid_t child_pid;
    int child_status;
    int number = 100;

    /* Flush before fork so buffered output is not duplicated. */
    fflush(NULL);
    child_pid = fork();
    if (child_pid < 0) {
        perror("fork failed");
        return 0;
    }

    if (child_pid == 0) {
        number += 1;
        printf("[Child] PID=%ld, parent PID=%ld, copied number=%d\n",
               (long)getpid(), (long)getppid(), number);
        fflush(stdout);
        _exit(0);
    }

    printf("[Parent] PID=%ld, child PID=%ld, original number=%d\n",
           (long)getpid(), (long)child_pid, number);
    if (waitpid(child_pid, &child_status, 0) < 0) {
        perror("waitpid failed");
        return 0;
    }
    if (WIFEXITED(child_status))
        printf("[Parent] Child exited with status %d.\n", WEXITSTATUS(child_status));
    return 1;
}
#else
int run_fork_demo(void)
{
    puts("The real fork() system call is unavailable in this Windows build.");
    puts("Compile and run this project in Linux or WSL to demonstrate fork().");
    return 0;
}
#endif
