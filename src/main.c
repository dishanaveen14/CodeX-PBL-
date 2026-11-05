/**
 * @file main.c
 * @brief Integrated Menu-Driven Application for POSIX IPC Mechanisms with Telemetry.
 *
 * Owned by: Student 4 (Integration and Main Application)
 * Team Members:
 *   - Student 1: Pipes and Message Queues
 *   - Student 2: Sockets and Shared Memory
 *   - Student 3: Semaphores and Mutex Locks
 *   - Student 4: Integration and Main Application
 *
 * Course: Operating Systems - POSIX IPC Project
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pipes.h"
#include "message_queue.h"
#include "sockets.h"
#include "shared_memory.h"
#include "semaphores.h"
#include "mutex.h"
#include "gui.h"
#include "ipc_logger.h"

static void print_banner(void)
{
    printf("\n========================================\n");
    printf("       IPC USING POSIX MECHANISMS       \n");
    printf("========================================\n\n");
    printf("1. Pipes\n");
    printf("2. Message Queues\n");
    printf("3. Sockets\n");
    printf("4. Shared Memory\n");
    printf("5. Semaphores\n");
    printf("6. Mutex Locks\n");
    printf("7. Graphical User Interface (Simple GUI in C)\n");
    printf("8. Exit\n\n");
    printf("Enter your choice: ");
    fflush(stdout);
}

static void run_all_mechanisms(void)
{
    ipc_log(LOG_LVL_INFO, "CORE", ">>> EXECUTING ALL 6 POSIX IPC MECHANISMS <<<");
    execute_pipes_demo(15, 25, "SUITE_PIPE", NULL, 0);
    execute_message_queue_demo("SUITE: Critical Reactor Temperature Alert", 10, NULL, 0);
    execute_sockets_demo("SUITE: Query Core Process Load", NULL, 0);
    execute_shared_memory_demo("SUITE: Shared Frame Telemetry Block #1024", 9001, NULL, 0);
    execute_semaphores_demo(4, 25000, NULL, 0);
    execute_mutex_demo(10000.0, 4, 10000, NULL, 0);
    ipc_log(LOG_LVL_INFO, "CORE", "[+] All 6 IPC demonstrations completed successfully.");
}

int main(int argc, char *argv[])
{
    if (argc > 1) {
        if (strcmp(argv[1], "--gui") == 0 || strcmp(argv[1], "-g") == 0) {
            return launch_gui(argc, argv);
        } else if (strcmp(argv[1], "--all") == 0 || strcmp(argv[1], "-a") == 0) {
            run_all_mechanisms();
            return 0;
        } else if (strcmp(argv[1], "--pipes") == 0 || strcmp(argv[1], "-1") == 0) {
            run_pipes();
            return 0;
        } else if (strcmp(argv[1], "--mq") == 0 || strcmp(argv[1], "-2") == 0) {
            run_message_queue();
            return 0;
        } else if (strcmp(argv[1], "--sockets") == 0 || strcmp(argv[1], "-3") == 0) {
            run_sockets();
            return 0;
        } else if (strcmp(argv[1], "--shm") == 0 || strcmp(argv[1], "-4") == 0) {
            run_shared_memory();
            return 0;
        } else if (strcmp(argv[1], "--sem") == 0 || strcmp(argv[1], "-5") == 0) {
            run_semaphores();
            return 0;
        } else if (strcmp(argv[1], "--mutex") == 0 || strcmp(argv[1], "-6") == 0) {
            run_mutex();
            return 0;
        } else if (strcmp(argv[1], "--cmd") == 0 && argc > 2) {
            /* Backend API bridge mode for web server */
            char buf[8192] = {0};
            if (strcmp(argv[2], "PIPES") == 0) {
                long long n1 = (argc > 3) ? atoll(argv[3]) : 20;
                long long n2 = (argc > 4) ? atoll(argv[4]) : 30;
                execute_pipes_demo(n1, n2, "WEB_PIPE", buf, sizeof(buf));
            } else if (strcmp(argv[2], "MQ") == 0) {
                const char *msg = (argc > 3 && strlen(argv[3])) ? argv[3] : "WEB: Critical Alert";
                int prio = (argc > 4) ? atoi(argv[4]) : 10;
                execute_message_queue_demo(msg, prio, buf, sizeof(buf));
            } else if (strcmp(argv[2], "SOCKETS") == 0) {
                const char *msg = (argc > 3 && strlen(argv[3])) ? argv[3] : "WEB: Socket Status Request";
                execute_sockets_demo(msg, buf, sizeof(buf));
            } else if (strcmp(argv[2], "SHM") == 0) {
                const char *msg = (argc > 3 && strlen(argv[3])) ? argv[3] : "WEB: Shared Matrix Payload";
                int tx = (argc > 4) ? atoi(argv[4]) : 9001;
                execute_shared_memory_demo(msg, tx, buf, sizeof(buf));
            } else if (strcmp(argv[2], "SEM") == 0) {
                int w = (argc > 3) ? atoi(argv[3]) : 4;
                int iter = (argc > 4) ? atoi(argv[4]) : 25000;
                execute_semaphores_demo(w, iter, buf, sizeof(buf));
            } else if (strcmp(argv[2], "MUTEX") == 0) {
                double b = (argc > 3) ? atof(argv[3]) : 10000.0;
                int ops = (argc > 4) ? atoi(argv[4]) : 10000;
                execute_mutex_demo(b, 4, ops, buf, sizeof(buf));
            } else if (strcmp(argv[2], "ALL") == 0) {
                run_all_mechanisms();
                return 0;
            }
            printf("%s", buf);
            return 0;
        }
    }

    int choice = 0;
    while (1) {
        print_banner();
        if (scanf("%d", &choice) != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            printf("\n[!] Invalid input. Please enter a number between 1 and 8.\n");
            continue;
        }

        switch (choice) {
        case 1:
            run_pipes();
            break;
        case 2:
            run_message_queue();
            break;
        case 3:
            run_sockets();
            break;
        case 4:
            run_shared_memory();
            break;
        case 5:
            run_semaphores();
            break;
        case 6:
            run_mutex();
            break;
        case 7:
            launch_gui(argc, argv);
            break;
        case 8:
            ipc_log(LOG_LVL_INFO, "CORE", "Exiting IPC POSIX application. Goodbye!");
            return 0;
        default:
            printf("\n[!] Invalid option. Please select between 1 and 8.\n");
            break;
        }

        printf("\nPress ENTER to continue...");
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF);
        getchar();
    }

    return 0;
}
