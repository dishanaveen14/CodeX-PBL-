/**
 * @file semaphores.c
 * @brief Implementation of POSIX Semaphores with Race Condition Detection & Telemetry.
 *
 * Owned by: Student 3 (Semaphores and Mutex Locks)
 * Course: Operating Systems - POSIX IPC Project
 */

#include "semaphores.h"
#include "ipc_logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

#ifdef _WIN32
  #include <windows.h>
#else
  #include <unistd.h>
#endif

typedef struct {
    int id;
    int iterations;
    long *counter;
    sem_t *sem;
} sem_worker_arg_t;

/* Worker without synchronization: deliberately introduces race window */
static void *worker_unsafe(void *arg)
{
    sem_worker_arg_t *warg = (sem_worker_arg_t *)arg;
    for (int i = 0; i < warg->iterations; i++) {
        volatile long temp = *(warg->counter);
#ifdef _WIN32
        if ((i % 1000) == 0) Sleep(0);
#else
        if ((i % 1000) == 0) sched_yield();
#endif
        temp = temp + 1;
        *(warg->counter) = temp;
    }
    return NULL;
}

/* Worker with POSIX Semaphore synchronization */
static void *worker_safe(void *arg)
{
    sem_worker_arg_t *warg = (sem_worker_arg_t *)arg;
    for (int i = 0; i < warg->iterations; i++) {
        sem_wait(warg->sem);      /* Acquire / lock */
        *(warg->counter) += 1;    /* Protected critical section */
        sem_post(warg->sem);      /* Release / signal */
    }
    return NULL;
}

int execute_semaphores_demo(int worker_count, int iterations, char *output_buffer, size_t buffer_size)
{
    if (output_buffer && buffer_size > 0) {
        ipc_log_set_capture(output_buffer, buffer_size);
    }

    int workers = (worker_count >= 2 && worker_count <= 8) ? worker_count : 4;
    int iters = (iterations >= 1000 && iterations <= 100000) ? iterations : 25000;
    long expected_total = (long)workers * iters;

    ipc_log(LOG_LVL_INFO, "SEM", "========================================================");
    ipc_log(LOG_LVL_INFO, "SEM", "STARTING POSIX SEMAPHORES SYNCHRONIZATION DEMO");
    ipc_log(LOG_LVL_INFO, "SEM", "User Inputs: Workers=%d, Iterations/worker=%d (Target Sum=%ld)",
            workers, iters, expected_total);

    /* --- Experiment 1: Unsynchronized --- */
    ipc_log(LOG_LVL_INFO, "SEM", "Step 1: Running Experiment 1 (Unsynchronized Access)...");
    ipc_log(LOG_LVL_INFO, "SEM", "Spawning %d threads competing for shared counter WITHOUT semaphore...", workers);

    long unsafe_counter = 0;
    pthread_t threads[8];
    sem_worker_arg_t unsafe_args[8];

    for (int i = 0; i < workers; i++) {
        unsafe_args[i].id = i + 1;
        unsafe_args[i].iterations = iters;
        unsafe_args[i].counter = &unsafe_counter;
        unsafe_args[i].sem = NULL;
        pthread_create(&threads[i], NULL, worker_unsafe, &unsafe_args[i]);
    }

    for (int i = 0; i < workers; i++) {
        pthread_join(threads[i], NULL);
    }

    long lost_updates = expected_total - unsafe_counter;
    if (lost_updates != 0) {
        ipc_log(LOG_LVL_WARN, "SEM", "RACE CONDITION DETECTED! Unsafe Counter=%ld / %ld (Lost updates: %ld)",
                unsafe_counter, expected_total, lost_updates);
    } else {
        ipc_log(LOG_LVL_INFO, "SEM", "Unsafe Counter=%ld / %ld (High thread scheduler coincidence)",
                unsafe_counter, expected_total);
    }

    /* --- Experiment 2: Synchronized with POSIX Semaphore --- */
    ipc_log(LOG_LVL_INFO, "SEM", "Step 2: Running Experiment 2 (POSIX Semaphore Protected)...");

    long safe_counter = 0;
    sem_t sem;
    sem_init(&sem, 0, 1); /* Binary semaphore initialized to 1 */

    ipc_log(LOG_LVL_INFO, "SEM", "Initialized binary semaphore via sem_init(val=1).");
    ipc_log(LOG_LVL_INFO, "SEM", "Spawning %d threads using sem_wait() and sem_post()...", workers);

    sem_worker_arg_t safe_args[8];
    for (int i = 0; i < workers; i++) {
        safe_args[i].id = i + 1;
        safe_args[i].iterations = iters;
        safe_args[i].counter = &safe_counter;
        safe_args[i].sem = &sem;
        pthread_create(&threads[i], NULL, worker_safe, &safe_args[i]);
    }

    for (int i = 0; i < workers; i++) {
        pthread_join(threads[i], NULL);
    }

    sem_destroy(&sem);

    ipc_log(LOG_LVL_INFO, "SEM", "Step 3: All %d worker threads completed. Destroying semaphore (sem_destroy).", workers);
    ipc_log(LOG_LVL_INFO, "SEM", "Synchronized Counter = %ld / %ld (100%% ACCURATE, 0 Lost Updates)",
            safe_counter, expected_total);
    ipc_log(LOG_LVL_INFO, "SEM", "POSIX SEMAPHORE DEMONSTRATION COMPLETE: Mutual exclusion guaranteed.");
    ipc_log(LOG_LVL_INFO, "SEM", "========================================================");

    ipc_log_stop_capture();
    return 0;
}

void run_semaphores(void)
{
    printf("\n========================================================\n");
    printf("            STUDENT 3: POSIX SEMAPHORES MODULE          \n");
    printf("========================================================\n");

    int workers = 4;
    int iters = 25000;

    printf("Enter number of concurrent worker threads [2-8, default 4]: ");
    if (scanf("%d", &workers) != 1) { workers = 4; while (getchar() != '\n'); }

    printf("Enter number of increments per worker [1000-50000, default 25000]: ");
    if (scanf("%d", &iters) != 1) { iters = 25000; while (getchar() != '\n'); }

    printf("\n--- Executing POSIX Semaphores Synchronization & Streaming Logs ---\n");
    execute_semaphores_demo(workers, iters, NULL, 0);
}
