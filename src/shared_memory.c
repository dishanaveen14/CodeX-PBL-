/**
 * @file shared_memory.c
 * @brief Implementation of POSIX Shared Memory IPC with Zero-Copy Telemetry.
 *
 * Owned by: Student 2 (Sockets and Shared Memory)
 * Course: Operating Systems - POSIX IPC Project
 */

#include "shared_memory.h"
#include "ipc_logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifndef _WIN32
  #include <unistd.h>
  #include <fcntl.h>
  #include <sys/mman.h>
  #include <sys/stat.h>
  #include <sys/time.h>
#else
  #include <windows.h>
#endif

int execute_shared_memory_demo(const char *shared_data, int tx_id, char *output_buffer, size_t buffer_size)
{
    if (output_buffer && buffer_size > 0) {
        ipc_log_set_capture(output_buffer, buffer_size);
    }

    const char *payload = (shared_data && strlen(shared_data) > 0)
                          ? shared_data
                          : "High-Speed Memory Segment: Telemetry Buffer Frame #0084";
    int transaction = (tx_id > 0) ? tx_id : 9001;

    ipc_log(LOG_LVL_INFO, "SHM", "========================================================");
    ipc_log(LOG_LVL_INFO, "SHM", "STARTING POSIX SHARED MEMORY IPC DEMONSTRATION");
    ipc_log(LOG_LVL_INFO, "SHM", "User Inputs: Data='%s', TxID=%d", payload, transaction);
    ipc_log(LOG_LVL_INFO, "SHM", "Step 1: Creating POSIX shared memory object: %s", SHM_DEMO_NAME);

#ifndef _WIN32
    shm_unlink(SHM_DEMO_NAME);

    ipc_log(LOG_LVL_INFO, "SHM", "Step 2: Invoking shm_open(O_CREAT | O_RDWR, 0666)...");
    int shm_fd = shm_open(SHM_DEMO_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        ipc_log(LOG_LVL_ERROR, "SHM", "shm_open failed: %s", strerror(errno));
        ipc_log_stop_capture();
        return -1;
    }

    ipc_log(LOG_LVL_INFO, "SHM", "Step 3: Setting memory segment size to %zu bytes via ftruncate()...", sizeof(shm_payload_t));
    if (ftruncate(shm_fd, sizeof(shm_payload_t)) == -1) {
        ipc_log(LOG_LVL_ERROR, "SHM", "ftruncate failed: %s", strerror(errno));
        close(shm_fd);
        shm_unlink(SHM_DEMO_NAME);
        ipc_log_stop_capture();
        return -1;
    }

    ipc_log(LOG_LVL_INFO, "SHM", "Step 4: Mapping shared memory page into virtual address space via mmap()...");
    shm_payload_t *shm_ptr = (shm_payload_t *)mmap(
        NULL, sizeof(shm_payload_t),
        PROT_READ | PROT_WRITE, MAP_SHARED,
        shm_fd, 0
    );

    if (shm_ptr == MAP_FAILED) {
        ipc_log(LOG_LVL_ERROR, "SHM", "mmap failed: %s", strerror(errno));
        close(shm_fd);
        shm_unlink(SHM_DEMO_NAME);
        ipc_log_stop_capture();
        return -1;
    }

    ipc_log(LOG_LVL_INFO, "SHM", "Shared segment successfully mapped at virtual address %p", (void *)shm_ptr);
    ipc_log(LOG_LVL_INFO, "SHM", "Step 5: Writer process writing structured payload directly into physical RAM...");

    shm_ptr->transaction_id = transaction;
    shm_ptr->timestamp_usec = 1718290000;
    strncpy(shm_ptr->author, "Process #1 (Core Engine)", sizeof(shm_ptr->author) - 1);
    strncpy(shm_ptr->data, payload, sizeof(shm_ptr->data) - 1);
    shm_ptr->is_ready = 1;

    ipc_log(LOG_LVL_INFO, "SHM", "    [Written to RAM] TxID: %d | Author: \"%s\"", shm_ptr->transaction_id, shm_ptr->author);
    ipc_log(LOG_LVL_INFO, "SHM", "    [Written to RAM] Data: \"%s\"", shm_ptr->data);

    ipc_log(LOG_LVL_INFO, "SHM", "Step 6: Reader process accessing shared memory directly (ZERO-COPY overhead)...");
    ipc_log(LOG_LVL_INFO, "SHM", "    [Read Back] TxID: %d | Status: READY | Data: \"%s\"",
            shm_ptr->transaction_id, shm_ptr->data);

    munmap(shm_ptr, sizeof(shm_payload_t));
    close(shm_fd);
    shm_unlink(SHM_DEMO_NAME);

    ipc_log(LOG_LVL_INFO, "SHM", "Step 7: Memory unmapped (munmap) and object unlinked (shm_unlink).");
    ipc_log(LOG_LVL_INFO, "SHM", "ADVANTAGE: Direct RAM access enables maximum IPC bandwidth without OS copying!");
    ipc_log(LOG_LVL_INFO, "SHM", "========================================================");

#else
    ipc_log(LOG_LVL_INFO, "SHM", "Step 2: Allocating shared memory block (%zu bytes)...", sizeof(shm_payload_t));
    HANDLE hMap = CreateFileMappingA(
        INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
        0, (DWORD)sizeof(shm_payload_t), "Local\\ipc_posix_demo_shm");

    if (!hMap) {
        ipc_log(LOG_LVL_ERROR, "SHM", "CreateFileMapping failed.");
        ipc_log_stop_capture();
        return -1;
    }

    shm_payload_t *shm_ptr = (shm_payload_t *)MapViewOfFile(
        hMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(shm_payload_t));

    if (!shm_ptr) {
        CloseHandle(hMap);
        ipc_log(LOG_LVL_ERROR, "SHM", "MapViewOfFile failed.");
        ipc_log_stop_capture();
        return -1;
    }

    ipc_log(LOG_LVL_INFO, "SHM", "Shared segment mapped at virtual address %p", (void *)shm_ptr);
    ipc_log(LOG_LVL_INFO, "SHM", "Step 3: Writer process populating shared memory structure directly...");

    shm_ptr->transaction_id = transaction;
    shm_ptr->timestamp_usec = 1718290000;
    strncpy(shm_ptr->author, "Process #1 (Core Engine)", sizeof(shm_ptr->author) - 1);
    strncpy(shm_ptr->data, payload, sizeof(shm_ptr->data) - 1);
    shm_ptr->is_ready = 1;

    ipc_log(LOG_LVL_INFO, "SHM", "    [Written to RAM] TxID: %d | Author: \"%s\"", shm_ptr->transaction_id, shm_ptr->author);
    ipc_log(LOG_LVL_INFO, "SHM", "    [Written to RAM] Data: \"%s\"", shm_ptr->data);

    ipc_log(LOG_LVL_INFO, "SHM", "Step 4: Reader process reading directly from mapped memory space...");
    ipc_log(LOG_LVL_INFO, "SHM", "    [Read Back] TxID: %d | Status: READY | Data: \"%s\"",
            shm_ptr->transaction_id, shm_ptr->data);

    UnmapViewOfFile(shm_ptr);
    CloseHandle(hMap);

    ipc_log(LOG_LVL_INFO, "SHM", "Step 5: Memory unmapped and segment released.");
    ipc_log(LOG_LVL_INFO, "SHM", "ADVANTAGE: Zero-copy inter-process memory sharing achieved!");
    ipc_log(LOG_LVL_INFO, "SHM", "========================================================");
#endif

    ipc_log_stop_capture();
    return 0;
}

void run_shared_memory(void)
{
    printf("\n========================================================\n");
    printf("          STUDENT 2: POSIX SHARED MEMORY MODULE         \n");
    printf("========================================================\n");

    char data_str[256] = {0};
    int tx_id = 9001;

    printf("Enter data string to write to Shared Memory [e.g. Telemetry State Matrix 4x4]: ");
    if (fgets(data_str, sizeof(data_str), stdin)) {
        data_str[strcspn(data_str, "\r\n")] = 0;
    }
    if (strlen(data_str) == 0) strcpy(data_str, "High-Speed Telemetry Block: Frame #1042");

    printf("Enter transaction ID integer [e.g. 9001]: ");
    if (scanf("%d", &tx_id) != 1) { tx_id = 9001; while (getchar() != '\n'); }

    printf("\n--- Executing POSIX Shared Memory IPC & Streaming Logs ---\n");
    execute_shared_memory_demo(data_str, tx_id, NULL, 0);
}
