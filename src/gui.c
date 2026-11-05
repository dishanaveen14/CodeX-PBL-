/**
 * @file gui.c
 * @brief Graphical User Interface in C with Inputs, Process Descriptions, and Vertical Telemetry Log.
 *
 * Owned by: Student 4 (Integration and Main Application)
 * Course: Operating Systems - POSIX IPC Project
 */

#include "gui.h"
#include "pipes.h"
#include "message_queue.h"
#include "sockets.h"
#include "shared_memory.h"
#include "semaphores.h"
#include "mutex.h"
#include "ipc_logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>

#define ID_BTN_PIPES        101
#define ID_BTN_MQ           102
#define ID_BTN_SOCKETS      103
#define ID_BTN_SHM          104
#define ID_BTN_SEM          105
#define ID_BTN_MUTEX        106
#define ID_BTN_RUN_ALL      107
#define ID_BTN_CLEAR        108

#define ID_EDIT_INPUT1      201
#define ID_EDIT_INPUT2      202
#define ID_EDIT_OUTPUT      203
#define ID_STATUS_LABEL     204
#define ID_EDIT_DESC        205

static HWND g_hwndInput1 = NULL;
static HWND g_hwndInput2 = NULL;
static HWND g_hwndOutput = NULL;
static HWND g_hwndStatus = NULL;
static HWND g_hwndDesc   = NULL;

/* Process Descriptions for each IPC mechanism */
static const char *DESC_DEFAULT =
    "[IPC ARCHITECTURE OVERVIEW]\r\n"
    "• IPC enables isolated processes with separate address spaces to exchange data and synchronize.\r\n"
    "• Select any mechanism button above to view its process roles, IPC channel, and telemetry stream.";

static const char *DESC_PIPES =
    "[1. POSIX ANONYMOUS PIPES - PROCESS ROLES & DATA FLOW]\r\n"
    "• Process A (Parent Process): Spawns child with fork(); closes write-end fd[1]; reads result from fd[0]; computes sum.\r\n"
    "• Process B (Child Process): Closes read-end fd[0]; writes integer payload into fd[1]; exits cleanly.\r\n"
    "• IPC Channel: Unidirectional kernel ring-buffer (pipe) managed directly by the OS kernel.\r\n"
    "• Synchronization: Automatic OS kernel blocking (read blocks when empty; write blocks when full).";

static const char *DESC_MQ =
    "[2. POSIX MESSAGE QUEUES - PROCESS ROLES & DATA FLOW]\r\n"
    "• Process A (Sender Process): Opens /posix_ipc_prio_queue; packs message & priority level (1-10); calls mq_send().\r\n"
    "• Process B (Receiver Process): Listens on queue descriptor; calls mq_receive(); retrieves highest priority first.\r\n"
    "• IPC Channel: POSIX kernel message queue maintaining prioritized FIFO ordering in kernel space.\r\n"
    "• Synchronization: Non-blocking or blocking message retrieval with strict priority guarantees.";

static const char *DESC_SOCKETS =
    "[3. POSIX STREAM SOCKETS - PROCESS ROLES & DATA FLOW]\r\n"
    "• Process A (TCP Server): Creates AF_INET socket; binds to 127.0.0.1:9099; listens & accepts connection; returns ACK.\r\n"
    "• Process B (TCP Client): Connects to server socket; sends query string over stream; receives and validates response.\r\n"
    "• IPC Channel: Loopback stream socket (SOCK_STREAM) providing full-duplex, reliable byte-stream protocol.\r\n"
    "• Synchronization: OS network stack handles 3-way handshake, packet buffering, flow control, and teardown.";

static const char *DESC_SHM =
    "[4. POSIX SHARED MEMORY - PROCESS ROLES & DATA FLOW]\r\n"
    "• Process A (Writer Process): Creates shm object with shm_open(); resizes with ftruncate(); maps memory via mmap(); writes data.\r\n"
    "• Process B (Reader Process): Opens existing shm object; maps via mmap(); reads directly from shared physical RAM.\r\n"
    "• IPC Channel: Shared physical RAM pages mapped into the virtual address spaces of both processes.\r\n"
    "• Synchronization: Fastest IPC (zero-copy); requires coordination primitives to ensure atomicity.";

static const char *DESC_SEM =
    "[5. POSIX SEMAPHORES - PROCESS ROLES & DATA FLOW]\r\n"
    "• Concurrent Threads / Processes: 4 worker threads performing 25,000 parallel increments on a shared counter.\r\n"
    "• Synchronization Mechanism: Atomic sem_wait() (P operation) decrements lock; sem_post() (V operation) increments lock.\r\n"
    "• Result & Safety: Demonstrates critical race conditions in unprotected code vs guaranteed accuracy with semaphores.\r\n"
    "• IPC Channel: POSIX counting semaphore (sem_t) maintaining kernel-level atomic counters.";

static const char *DESC_MUTEX =
    "[6. POSIX MUTEX LOCKS - PROCESS ROLES & DATA FLOW]\r\n"
    "• Concurrent Threads / Processes: 4 bank transaction threads performing parallel deposits and withdrawals.\r\n"
    "• Synchronization Mechanism: pthread_mutex_lock() grants exclusive critical section access; pthread_mutex_unlock() releases.\r\n"
    "• Result & Safety: Demonstrates balance corruption without locks vs perfect balance preservation with mutual exclusion.\r\n"
    "• IPC Channel: POSIX mutual exclusion lock (pthread_mutex_t) enforced by CPU atomic instructions.";

static const char *DESC_RUN_ALL =
    "[FULL IPC TEST SUITE - PROCESS ROLES & DATA FLOW]\r\n"
    "• Executes all 6 IPC mechanisms sequentially to compare throughput, latency, and synchronization guarantees.\r\n"
    "• Telemetry log streams live into the vertical log window below and is persisted to sim.log for auditing.";

static void append_gui_output(const char *text)
{
    if (!g_hwndOutput) return;
    int len = GetWindowTextLengthA(g_hwndOutput);
    SendMessageA(g_hwndOutput, EM_SETSEL, (WPARAM)len, (LPARAM)len);
    SendMessageA(g_hwndOutput, EM_REPLACESEL, 0, (LPARAM)text);
    SendMessageA(g_hwndOutput, EM_SCROLLCARET, 0, 0);
}

static void update_gui_status(const char *status)
{
    if (g_hwndStatus) {
        SetWindowTextA(g_hwndStatus, status);
    }
}

static void set_process_description(const char *desc)
{
    if (g_hwndDesc) {
        SetWindowTextA(g_hwndDesc, desc);
    }
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static char output_buffer[16384];
    char in1[256] = {0};
    char in2[256] = {0};

    switch (msg) {
    case WM_CREATE:
    {
        HFONT hFontMono = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                      ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                      DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Consolas");

        HFONT hFontUi = CreateFontA(14, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                                    ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                    DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");

        HFONT hFontDesc = CreateFontA(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                      ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                      DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");

        /* Title */
        HWND hTitle = CreateWindowA("STATIC", "POSIX Inter-Process Communication Simulator (Team CodeX)",
                      WS_CHILD | WS_VISIBLE | SS_LEFT,
                      20, 10, 800, 22, hwnd, NULL, NULL, NULL);
        SendMessageA(hTitle, WM_SETFONT, (WPARAM)hFontUi, TRUE);

        /* Input Controls */
        HWND lbl1 = CreateWindowA("STATIC", "Parameter 1 (Num1 / Msg / Query / Payload):",
                                  WS_CHILD | WS_VISIBLE, 20, 36, 380, 18, hwnd, NULL, NULL, NULL);
        SendMessageA(lbl1, WM_SETFONT, (WPARAM)hFontUi, TRUE);

        g_hwndInput1 = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "45",
                                       WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                                       20, 56, 380, 24, hwnd, (HMENU)ID_EDIT_INPUT1, NULL, NULL);
        SendMessageA(g_hwndInput1, WM_SETFONT, (WPARAM)hFontUi, TRUE);

        HWND lbl2 = CreateWindowA("STATIC", "Parameter 2 (Num2 / Priority / Workers / Threads):",
                                  WS_CHILD | WS_VISIBLE, 430, 36, 380, 18, hwnd, NULL, NULL, NULL);
        SendMessageA(lbl2, WM_SETFONT, (WPARAM)hFontUi, TRUE);

        g_hwndInput2 = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "55",
                                       WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                                       430, 56, 380, 24, hwnd, (HMENU)ID_EDIT_INPUT2, NULL, NULL);
        SendMessageA(g_hwndInput2, WM_SETFONT, (WPARAM)hFontUi, TRUE);

        /* Action Buttons - Row 1 */
        HWND btn;
        btn = CreateWindowA("BUTTON", "1. Run Pipes", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                            20, 88, 120, 30, hwnd, (HMENU)ID_BTN_PIPES, NULL, NULL);
        SendMessageA(btn, WM_SETFONT, (WPARAM)hFontUi, TRUE);

        btn = CreateWindowA("BUTTON", "2. Message Queue", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                            150, 88, 135, 30, hwnd, (HMENU)ID_BTN_MQ, NULL, NULL);
        SendMessageA(btn, WM_SETFONT, (WPARAM)hFontUi, TRUE);

        btn = CreateWindowA("BUTTON", "3. Sockets", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                            295, 88, 120, 30, hwnd, (HMENU)ID_BTN_SOCKETS, NULL, NULL);
        SendMessageA(btn, WM_SETFONT, (WPARAM)hFontUi, TRUE);

        btn = CreateWindowA("BUTTON", "4. Shared Memory", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                            425, 88, 135, 30, hwnd, (HMENU)ID_BTN_SHM, NULL, NULL);
        SendMessageA(btn, WM_SETFONT, (WPARAM)hFontUi, TRUE);

        btn = CreateWindowA("BUTTON", "5. Semaphores", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                            570, 88, 125, 30, hwnd, (HMENU)ID_BTN_SEM, NULL, NULL);
        SendMessageA(btn, WM_SETFONT, (WPARAM)hFontUi, TRUE);

        btn = CreateWindowA("BUTTON", "6. Mutex", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                            705, 88, 115, 30, hwnd, (HMENU)ID_BTN_MUTEX, NULL, NULL);
        SendMessageA(btn, WM_SETFONT, (WPARAM)hFontUi, TRUE);

        /* Action Buttons - Row 2 */
        btn = CreateWindowA("BUTTON", "⚡ Run All 6 Tests", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                            20, 124, 135, 28, hwnd, (HMENU)ID_BTN_RUN_ALL, NULL, NULL);
        SendMessageA(btn, WM_SETFONT, (WPARAM)hFontUi, TRUE);

        btn = CreateWindowA("BUTTON", "Clear Log", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                            165, 124, 95, 28, hwnd, (HMENU)ID_BTN_CLEAR, NULL, NULL);
        SendMessageA(btn, WM_SETFONT, (WPARAM)hFontUi, TRUE);

        /* Status Label */
        g_hwndStatus = CreateWindowA("STATIC", "Status: Ready. Enter inputs above and click any mechanism button.",
                                     WS_CHILD | WS_VISIBLE | SS_LEFT,
                                     275, 129, 540, 20, hwnd, (HMENU)ID_STATUS_LABEL, NULL, NULL);
        SendMessageA(g_hwndStatus, WM_SETFONT, (WPARAM)hFontUi, TRUE);

        /* Process Description Group Box */
        HWND grpDesc = CreateWindowA("BUTTON", " Process Architecture & Roles Description ",
                                     WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                                     20, 158, 825, 106, hwnd, NULL, NULL, NULL);
        SendMessageA(grpDesc, WM_SETFONT, (WPARAM)hFontUi, TRUE);

        /* Process Description Display (Word-wrapped text box) */
        g_hwndDesc = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", DESC_DEFAULT,
                                     WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY,
                                     30, 178, 805, 76, hwnd, (HMENU)ID_EDIT_DESC, NULL, NULL);
        SendMessageA(g_hwndDesc, WM_SETFONT, (WPARAM)hFontDesc, TRUE);

        /* Execution Log Group Box */
        HWND grpLog = CreateWindowA("BUTTON", " System Execution Log & Telemetry (Vertical Scroll) ",
                                    WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                                    20, 270, 825, 385, hwnd, NULL, NULL, NULL);
        SendMessageA(grpLog, WM_SETFONT, (WPARAM)hFontUi, TRUE);

        /* Output View - Strictly VERTICAL scroll (NO WS_HSCROLL, NO ES_AUTOHSCROLL for word wrapping) */
        g_hwndOutput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
                                       WS_CHILD | WS_VISIBLE | WS_VSCROLL |
                                       ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
                                       30, 290, 805, 355, hwnd, (HMENU)ID_EDIT_OUTPUT, NULL, NULL);
        SendMessageA(g_hwndOutput, WM_SETFONT, (WPARAM)hFontMono, TRUE);

        append_gui_output("===============================================================================\r\n");
        append_gui_output("         POSIX IPC MECHANISMS GUI - C SYSTEM EXECUTION LOGGER                  \r\n");
        append_gui_output("===============================================================================\r\n");
        append_gui_output("Log streams vertically with automatic word-wrapping and persists to sim.log.\r\n\r\n");
        break;
    }

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        output_buffer[0] = '\0';
        GetWindowTextA(g_hwndInput1, in1, sizeof(in1) - 1);
        GetWindowTextA(g_hwndInput2, in2, sizeof(in2) - 1);

        switch (wmId) {
        case ID_BTN_PIPES:
        {
            long long n1 = atoll(strlen(in1) ? in1 : "25");
            long long n2 = atoll(strlen(in2) ? in2 : "35");
            set_process_description(DESC_PIPES);
            update_gui_status("Status: Executing POSIX Pipes Demo...");
            append_gui_output("\r\n--- [1. POSIX ANONYMOUS PIPES EXECUTION] ---\r\n");
            append_gui_output("PROCESS ROLES: Parent Process (Reader) <--- [Kernel Pipe] <--- Child Process (Writer)\r\n");
            execute_pipes_demo(n1, n2, "GUI_PIPE_OP", output_buffer, sizeof(output_buffer));
            append_gui_output(output_buffer);
            update_gui_status("Status: POSIX Pipes Demo completed successfully.");
            break;
        }

        case ID_BTN_MQ:
        {
            const char *msg = strlen(in1) ? in1 : "CRITICAL: Reactor Temperature High";
            int prio = atoi(strlen(in2) ? in2 : "10");
            set_process_description(DESC_MQ);
            update_gui_status("Status: Executing POSIX Message Queue Demo...");
            append_gui_output("\r\n--- [2. POSIX MESSAGE QUEUES EXECUTION] ---\r\n");
            append_gui_output("PROCESS ROLES: Sender Process ---> [Kernel Message Queue] ---> Receiver Process\r\n");
            execute_message_queue_demo(msg, prio, output_buffer, sizeof(output_buffer));
            append_gui_output(output_buffer);
            update_gui_status("Status: POSIX Message Queue Demo completed successfully.");
            break;
        }

        case ID_BTN_SOCKETS:
        {
            const char *query = strlen(in1) ? in1 : "QUERY_SYSTEM_LOAD_METRICS";
            set_process_description(DESC_SOCKETS);
            update_gui_status("Status: Executing POSIX Stream Sockets Demo...");
            append_gui_output("\r\n--- [3. POSIX STREAM SOCKETS EXECUTION] ---\r\n");
            append_gui_output("PROCESS ROLES: TCP Client <===> [Loopback Stream Socket 127.0.0.1:9099] <===> TCP Server\r\n");
            execute_sockets_demo(query, output_buffer, sizeof(output_buffer));
            append_gui_output(output_buffer);
            update_gui_status("Status: POSIX Sockets Demo completed successfully.");
            break;
        }

        case ID_BTN_SHM:
        {
            const char *data = strlen(in1) ? in1 : "Shared Matrix State: 1024x1024 Buffer";
            int tx = atoi(strlen(in2) ? in2 : "9001");
            set_process_description(DESC_SHM);
            update_gui_status("Status: Executing POSIX Shared Memory Demo...");
            append_gui_output("\r\n--- [4. POSIX SHARED MEMORY EXECUTION] ---\r\n");
            append_gui_output("PROCESS ROLES: Writer Process ===> [Direct Physical RAM Pages] <=== Reader Process\r\n");
            execute_shared_memory_demo(data, tx, output_buffer, sizeof(output_buffer));
            append_gui_output(output_buffer);
            update_gui_status("Status: POSIX Shared Memory Demo completed successfully.");
            break;
        }

        case ID_BTN_SEM:
        {
            int w = atoi(strlen(in1) ? in1 : "4");
            int it = atoi(strlen(in2) ? in2 : "25000");
            set_process_description(DESC_SEM);
            update_gui_status("Status: Executing POSIX Semaphores Demo...");
            append_gui_output("\r\n--- [5. POSIX SEMAPHORES EXECUTION] ---\r\n");
            append_gui_output("PROCESS ROLES: Worker Threads (1..4) ===> [sem_wait / sem_post] ===> Shared Counter\r\n");
            execute_semaphores_demo(w, it, output_buffer, sizeof(output_buffer));
            append_gui_output(output_buffer);
            update_gui_status("Status: POSIX Semaphores Demo completed successfully.");
            break;
        }

        case ID_BTN_MUTEX:
        {
            double b = atof(strlen(in1) ? in1 : "10000.0");
            int ops = atoi(strlen(in2) ? in2 : "10000");
            set_process_description(DESC_MUTEX);
            update_gui_status("Status: Executing POSIX Mutex Locks Demo...");
            append_gui_output("\r\n--- [6. POSIX MUTEX LOCKS EXECUTION] ---\r\n");
            append_gui_output("PROCESS ROLES: Transaction Threads (1..4) ===> [pthread_mutex_lock] ===> Bank Balance\r\n");
            execute_mutex_demo(b, 4, ops, output_buffer, sizeof(output_buffer));
            append_gui_output(output_buffer);
            update_gui_status("Status: POSIX Mutex Locks Demo completed successfully.");
            break;
        }

        case ID_BTN_RUN_ALL:
            set_process_description(DESC_RUN_ALL);
            update_gui_status("Status: Running all 6 POSIX IPC mechanisms sequentially...");
            append_gui_output("\r\n===============================================================================\r\n");
            append_gui_output("                      RUNNING FULL POSIX IPC TEST SUITE                        \r\n");
            append_gui_output("===============================================================================\r\n");
            execute_pipes_demo(20, 30, "SUITE", output_buffer, sizeof(output_buffer));
            append_gui_output(output_buffer);
            execute_message_queue_demo("SUITE_ALERT", 10, output_buffer, sizeof(output_buffer));
            append_gui_output(output_buffer);
            execute_sockets_demo("SUITE_SOCKET_QUERY", output_buffer, sizeof(output_buffer));
            append_gui_output(output_buffer);
            execute_shared_memory_demo("SUITE_RAM_DATA", 9001, output_buffer, sizeof(output_buffer));
            append_gui_output(output_buffer);
            execute_semaphores_demo(4, 25000, output_buffer, sizeof(output_buffer));
            append_gui_output(output_buffer);
            execute_mutex_demo(10000.0, 4, 10000, output_buffer, sizeof(output_buffer));
            append_gui_output(output_buffer);
            update_gui_status("Status: All 6 IPC mechanisms executed successfully!");
            break;

        case ID_BTN_CLEAR:
            SetWindowTextA(g_hwndOutput, "");
            set_process_description(DESC_DEFAULT);
            update_gui_status("Status: Log view cleared.");
            break;
        }
        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int launch_gui(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    HINSTANCE hInstance = GetModuleHandle(NULL);
    const char CLASS_NAME[] = "PosixIpcGuiClass";

    WNDCLASSA wc = {0};
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);

    RegisterClassA(&wc);

    int win_w = 880;
    int win_h = 710;
    int screen_w = GetSystemMetrics(SM_CXSCREEN);
    int screen_h = GetSystemMetrics(SM_CYSCREEN);
    int pos_x = (screen_w - win_w) / 2;
    int pos_y = (screen_h - win_h) / 2;
    if (pos_x < 0) pos_x = 20;
    if (pos_y < 0) pos_y = 20;

    HWND hwnd = CreateWindowExA(
        WS_EX_APPWINDOW, CLASS_NAME,
        "POSIX IPC Simulator (Simple GUI in C) - Team CodeX",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        pos_x, pos_y, win_w, win_h,
        NULL, NULL, hInstance, NULL
    );

    if (!hwnd) return 1;

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg = {0};
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return (int)msg.wParam;
}

#else
/* Linux / POSIX ANSI TUI Mode */
int launch_gui(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    int choice = 0;

    while (1) {
        printf("\033[2J\033[H");
        printf("\033[1;36m+-----------------------------------------------------------+\033[0m\n");
        printf("\033[1;36m|          POSIX IPC MECHANISMS SIMULATOR - C GUI           |\033[0m\n");
        printf("\033[1;36m+-----------------------------------------------------------+\033[0m\n");
        printf("\033[1;33m  [1]\033[0m Pipes Demonstration          \033[1;32m(Student 1)\033[0m\n");
        printf("\033[1;33m  [2]\033[0m Message Queues Demonstration \033[1;32m(Student 1)\033[0m\n");
        printf("\033[1;33m  [3]\033[0m Sockets Demonstration        \033[1;32m(Student 2)\033[0m\n");
        printf("\033[1;33m  [4]\033[0m Shared Memory Demonstration  \033[1;32m(Student 2)\033[0m\n");
        printf("\033[1;33m  [5]\033[0m Semaphores Demonstration     \033[1;32m(Student 3)\033[0m\n");
        printf("\033[1;33m  [6]\033[0m Mutex Locks Demonstration    \033[1;32m(Student 3)\033[0m\n");
        printf("\033[1;33m  [7]\033[0m Run All 6 Demonstrations     \033[1;35m(Full Suite)\033[0m\n");
        printf("\033[1;31m  [8]\033[0m Exit GUI\n");
        printf("\033[1;36m+-----------------------------------------------------------+\033[0m\n");
        printf("Select option (1-8): ");

        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            continue;
        }

        if (choice == 8) break;

        switch (choice) {
            case 1: run_pipes(); break;
            case 2: run_message_queue(); break;
            case 3: run_sockets(); break;
            case 4: run_shared_memory(); break;
            case 5: run_semaphores(); break;
            case 6: run_mutex(); break;
            case 7:
                execute_pipes_demo(20, 30, "ALL", NULL, 0);
                execute_message_queue_demo("ALERT", 10, NULL, 0);
                execute_sockets_demo("QUERY", NULL, 0);
                execute_shared_memory_demo("RAM_DATA", 9001, NULL, 0);
                execute_semaphores_demo(4, 25000, NULL, 0);
                execute_mutex_demo(10000.0, 4, 10000, NULL, 0);
                break;
            default: printf("Invalid choice.\n"); break;
        }

        printf("\nPress ENTER to return to menu...");
        getchar();
        getchar();
    }
    return 0;
}
#endif
