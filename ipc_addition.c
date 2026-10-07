#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static int write_all(int fd, const void *buffer, size_t size) {
    const char *bytes = buffer;
    size_t written = 0;

    while (written < size) {
        ssize_t result = write(fd, bytes + written, size - written);
        if (result < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        written += (size_t)result;
    }
    return 0;
}

static int read_all(int fd, void *buffer, size_t size) {
    char *bytes = buffer;
    size_t received = 0;

    while (received < size) {
        ssize_t result = read(fd, bytes + received, size - received);
        if (result < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (result == 0) return -1;
        received += (size_t)result;
    }
    return 0;
}

static int read_number(const char *prompt, long long *number) {
    printf("%s", prompt);
    fflush(stdout);
    if (scanf("%lld", number) != 1) {
        fprintf(stderr, "Please enter a valid integer.\n");
        return -1;
    }
    return 0;
}

int main(void) {
    int channel[2];
    long long number1;

    setvbuf(stdout, NULL, _IONBF, 0);

    if (read_number("Process 1: enter number 1: ", &number1) < 0) return EXIT_FAILURE;
    if (pipe(channel) < 0) {
        perror("pipe");
        return EXIT_FAILURE;
    }

    pid_t child = fork();
    if (child < 0) {
        perror("fork");
        close(channel[0]);
        close(channel[1]);
        return EXIT_FAILURE;
    }

    if (child == 0) {
        long long number2;
        close(channel[0]);
        printf("Process 2 (PID %ld) was created.\n", (long)getpid());

        if (read_number("Process 2: enter number 2: ", &number2) < 0) {
            close(channel[1]);
            _exit(EXIT_FAILURE);
        }
        if (write_all(channel[1], &number2, sizeof(number2)) < 0) {
            perror("Process 2: write to pipe");
            close(channel[1]);
            _exit(EXIT_FAILURE);
        }

        printf("Process 2 sent %lld to Process 1 through the pipe.\n", number2);
        close(channel[1]);
        _exit(EXIT_SUCCESS);
    }

    close(channel[1]);
    printf("Process 1 (PID %ld) is waiting for number 2 from Process 2 (PID %ld).\n",
           (long)getpid(), (long)child);

    long long number2;
    if (read_all(channel[0], &number2, sizeof(number2)) < 0) {
        fprintf(stderr, "Process 1: could not receive number 2 from Process 2.\n");
        close(channel[0]);
        waitpid(child, NULL, 0);
        return EXIT_FAILURE;
    }
    close(channel[0]);

    long long sum = number1 + number2;
    printf("Process 1 received %lld.\n", number2);
    printf("Process 1 calculated and stored sum = %lld + %lld = %lld.\n",
           number1, number2, sum);

    int status;
    if (waitpid(child, &status, 0) < 0) {
        perror("waitpid");
        return EXIT_FAILURE;
    }
    if (!WIFEXITED(status) || WEXITSTATUS(status) != EXIT_SUCCESS) {
        fprintf(stderr, "Process 2 did not complete successfully.\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
