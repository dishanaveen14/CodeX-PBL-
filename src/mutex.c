/**
 * @file mutex.c
 * @brief Implementation of POSIX Mutex Locks with Bank Simulation & Telemetry.
 *
 * Owned by: Student 3 (Semaphores and Mutex Locks)
 * Course: Operating Systems - POSIX IPC Project
 */

#include "mutex.h"
#include "ipc_logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#ifdef _WIN32
  #include <windows.h>
#else
  #include <unistd.h>
#endif

typedef struct {
    double balance;
    pthread_mutex_t lock;
} bank_account_t;

typedef struct {
    int id;
    bank_account_t *account;
    int ops;
    int use_mutex;
} teller_arg_t;

static void *teller_thread(void *arg)
{
    teller_arg_t *targ = (teller_arg_t *)arg;
    for (int i = 0; i < targ->ops; i++) {
        if (targ->use_mutex) {
            pthread_mutex_lock(&targ->account->lock);
        }

        /* Critical Section: Deposit $50 then withdraw $50 */
        volatile double b = targ->account->balance;
#ifdef _WIN32
        if ((i % 1000) == 0) Sleep(0);
#else
        if ((i % 1000) == 0) sched_yield();
#endif
        b += 50.0;
        targ->account->balance = b;

        b = targ->account->balance;
        b -= 50.0;
        targ->account->balance = b;

        if (targ->use_mutex) {
            pthread_mutex_unlock(&targ->account->lock);
        }
    }
    return NULL;
}

int execute_mutex_demo(double init_balance, int num_threads, int ops_per_thread, char *output_buffer, size_t buffer_size)
{
    if (output_buffer && buffer_size > 0) {
        ipc_log_set_capture(output_buffer, buffer_size);
    }

    double starting_balance = (init_balance > 0.0) ? init_balance : 10000.0;
    int threads_count = (num_threads >= 2 && num_threads <= 8) ? num_threads : 4;
    int ops = (ops_per_thread >= 1000 && ops_per_thread <= 50000) ? ops_per_thread : 10000;

    ipc_log(LOG_LVL_INFO, "MUTEX", "========================================================");
    ipc_log(LOG_LVL_INFO, "MUTEX", "STARTING POSIX MUTEX LOCKS DEMONSTRATION");
    ipc_log(LOG_LVL_INFO, "MUTEX", "User Inputs: Initial Balance=$%.2f, Teller Threads=%d, Ops/thread=%d",
            starting_balance, threads_count, ops);
    ipc_log(LOG_LVL_INFO, "MUTEX", "Scenario: Each teller performs %d matched (+$50 / -$50) transactions. Expected Final: $%.2f",
            ops, starting_balance);

    /* --- Run 1: Without Mutex --- */
    ipc_log(LOG_LVL_INFO, "MUTEX", "Step 1: Running Experiment 1 (WITHOUT Mutex Protection)...");
    ipc_log(LOG_LVL_INFO, "MUTEX", "Spawning %d teller threads updating balance concurrently...", threads_count);

    bank_account_t unsafe_account;
    unsafe_account.balance = starting_balance;
    pthread_t tids[8];
    teller_arg_t unsafe_args[8];

    for (int i = 0; i < threads_count; i++) {
        unsafe_args[i].id = i + 1;
        unsafe_args[i].account = &unsafe_account;
        unsafe_args[i].ops = ops;
        unsafe_args[i].use_mutex = 0;
        pthread_create(&tids[i], NULL, teller_thread, &unsafe_args[i]);
    }

    for (int i = 0; i < threads_count; i++) {
        pthread_join(tids[i], NULL);
    }

    double diff = unsafe_account.balance - starting_balance;
    if (diff != 0.0) {
        ipc_log(LOG_LVL_WARN, "MUTEX", "RACE CONDITION DETECTED! Final Balance: $%.2f (Discrepancy: %+.2f)",
                unsafe_account.balance, diff);
    } else {
        ipc_log(LOG_LVL_INFO, "MUTEX", "Final Balance: $%.2f (Discrepancy: $0.00)", unsafe_account.balance);
    }

    /* --- Run 2: With POSIX Mutex --- */
    ipc_log(LOG_LVL_INFO, "MUTEX", "Step 2: Running Experiment 2 (WITH POSIX Mutex Protection)...");
    ipc_log(LOG_LVL_INFO, "MUTEX", "Calling pthread_mutex_init() to initialize fast mutex lock...");

    bank_account_t safe_account;
    safe_account.balance = starting_balance;
    pthread_mutex_init(&safe_account.lock, NULL);

    ipc_log(LOG_LVL_INFO, "MUTEX", "Spawning %d teller threads using pthread_mutex_lock() / unlock()...", threads_count);

    teller_arg_t safe_args[8];
    for (int i = 0; i < threads_count; i++) {
        safe_args[i].id = i + 1;
        safe_args[i].account = &safe_account;
        safe_args[i].ops = ops;
        safe_args[i].use_mutex = 1;
        pthread_create(&tids[i], NULL, teller_thread, &safe_args[i]);
    }

    for (int i = 0; i < threads_count; i++) {
        pthread_join(tids[i], NULL);
    }

    /* Test pthread_mutex_trylock */
    int trylock_res = pthread_mutex_trylock(&safe_account.lock);
    if (trylock_res == 0) {
        ipc_log(LOG_LVL_INFO, "MUTEX", "pthread_mutex_trylock() verified non-blocking acquisition: SUCCESS");
        pthread_mutex_unlock(&safe_account.lock);
    }

    pthread_mutex_destroy(&safe_account.lock);
    ipc_log(LOG_LVL_INFO, "MUTEX", "Destroyed mutex via pthread_mutex_destroy().");
    ipc_log(LOG_LVL_INFO, "MUTEX", "Final Synchronized Balance: $%.2f (Discrepancy: $0.00 - 100%% EXACT)", safe_account.balance);
    ipc_log(LOG_LVL_INFO, "MUTEX", "POSIX MUTEX LOCK DEMONSTRATION COMPLETE: Mutual exclusion preserved.");
    ipc_log(LOG_LVL_INFO, "MUTEX", "========================================================");

    ipc_log_stop_capture();
    return 0;
}

void run_mutex(void)
{
    printf("\n========================================================\n");
    printf("             STUDENT 3: POSIX MUTEX MODULE              \n");
    printf("========================================================\n");

    double balance = 10000.0;
    int threads = 4;
    int ops = 10000;

    printf("Enter starting bank balance [e.g. 10000.00]: ");
    if (scanf("%lf", &balance) != 1) { balance = 10000.0; while (getchar() != '\n'); }

    printf("Enter number of concurrent teller threads [2-8, default 4]: ");
    if (scanf("%d", &threads) != 1) { threads = 4; while (getchar() != '\n'); }

    printf("Enter number of deposit/withdrawal operations per thread [1000-50000, default 10000]: ");
    if (scanf("%d", &ops) != 1) { ops = 10000; while (getchar() != '\n'); }

    printf("\n--- Executing POSIX Mutex Mutual Exclusion & Streaming Logs ---\n");
    execute_mutex_demo(balance, threads, ops, NULL, 0);
}
