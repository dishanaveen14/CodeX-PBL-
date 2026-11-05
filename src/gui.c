/**
 * @file gui.c
 * @brief Graphical User Interface in C with User Inputs, Process Descriptions, and Clean Vertical Log.
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
#define ID_EDIT_DESC        203
#define ID_EDIT_OUTPUT      204
#define ID_STATUS_LABEL     205

static HWND g_hwndInput1 = NULL;
static HWND g_hwndInput2 = NULL;
static HWND g_hwndOutput = NULL;
static HWND g_hwndStatus = NULL;
static HWND g_hwndDesc   = NULL;

/* Concise Process Descriptions */
static const char *DESC_DEFAULT =
    "• Architecture : Isolated processes communicating across virtual address space boundaries.\r\n"
    "• Instructions : Enter parameters above (or keep defaults) and click any mechanism button.\r\n"
    "• Telemetry    : Step-by-step execution logs stream cleanly into the vertical log window below.";

static const char *DESC_PIPES =
    "• Process A (Parent) : Creates pipe descriptors (fd[0], fd[1]), waits and reads sum from child.\r\n"
    "• Process B (Child)  : Writes payload into pipe write-end fd[1], exits cleanly.\r\n"
    "• IPC Channel        : Unidirectional Linux kernel FIFO byte-stream pipe ring-buffer.";

static const char *DESC_MQ =
    "• Process A (Sender)   : Opens queue /posix_ipc_prio_queue, packs message + priority, calls mq_send().\r\n"
    "• Process B (Receiver) : Calls mq_receive(), retrieves highest priority message first.\r\n"
    "• IPC Channel          : POSIX kernel message queue with strict priority ordering.";

static const char *DESC_SOCKETS =
    "• Process A (TCP Server) : Binds 127.0.0.1:9099, listens, accepts connection, replies with ACK.\r\n"
    "• Process B (TCP Client) : Connects to server, transmits query stream over TCP, receives reply.\r\n"
    "• IPC Channel            : Full-duplex loopback stream socket (AF_INET / SOCK_STREAM).";

static const char *DESC_SHM =
    "• Process A (Writer) : Creates shm object with shm_open(), maps via mmap(), writes directly to RAM.\r\n"
    "• Process B (Reader) : Opens shm object with shm_open(), maps via mmap(), reads memory block.\r\n"
    "• IPC Channel        : Shared physical RAM pages mapped into virtual address spaces (zero-copy).";

static const char *DESC_SEM =
    "• Worker Threads (1..4) : Concurrent worker threads competing to increment a shared counter.\r\n"
    "• Synchronization       : Atomic sem_wait() (lock) before critical section; sem_post() (unlock) on exit.\r\n"
    "• Result & Safety       : Race condition detected without lock vs 100% accuracy with semaphore.";

static const char *DESC_MUTEX =
    "• Bank Threads (1..4) : Concurrent teller threads performing simultaneous deposits and withdrawals.\r\n"
    "• Synchronization     : pthread_mutex_lock() ensures strict mutual exclusion on balance updates.\r\n"
    "• Result & Safety     : Eliminates lost updates; account balance preserved exactly without corruption.";

static const char *DESC_RUN_ALL =
    "• Test Suite  : Sequentially executes all 6 POSIX IPC mechanisms.\r\n"
    "• Comparison  : Verifies throughput, latency, memory mapping, and synchronization locks.\r\n"
    "• Audit Log   : All telemetry is captured and persisted to sim.log.";

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

/* Strips timestamp prefixes ('2026-xx-xx xx:xx:xx.xxx [LEVEL] [TAG] ') to produce a clean, simple log */
static void append_clean_log(const char *raw)
{
    if (!raw || !*raw) return;

    char clean_buffer[16384];
    size_t out_len = 0;
    clean_buffer[0] = '\0';

    const char *p = raw;
    while (*p && out_len < sizeof(clean_buffer) - 64) {
        const char *eol = strchr(p, '\n');
        size_t line_len = eol ? (size_t)(eol - p) : strlen(p);

        char line[1024];
        if (line_len >= sizeof(line)) line_len = sizeof(line) - 1;
        strncpy(line, p, line_len);
        line[line_len] = '\0';
        if (line_len > 0 && line[line_len - 1] == '\r') line[line_len - 1] = '\0';

        /* Strip timestamp prefix */
        char *content = line;
        if (strlen(line) >= 30 && line[4] == '-' && line[7] == '-' && line[10] == ' ') {
            char *b1 = strchr(line, ']');
            if (b1) {
                char *b2 = strchr(b1 + 1, ']');
                if (b2 && *(b2 + 1) == ' ') {
                    content = b2 + 2;
                }
            }
        }

        /* Standardize divider bars */
        if (strncmp(content, "========================", 24) == 0) {
            content = "------------------------------------------------------------";
        }

        if (strlen(content) > 0) {
            int written = snprintf(clean_buffer + out_len, sizeof(clean_buffer) - out_len, "%s\r\n", content);
            if (written > 0) out_len += (size_t)written;
        }

        if (!eol) break;
        p = eol + 1;
    }

    append_gui_output(clean_buffer);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static char raw_buffer[16384];
    char in1[256] = {0};
    char in2[256] = {0};

    switch (msg) {
    case WM_CREATE:
    {
        HFONT hFontTitle = CreateFontA(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                       ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                       CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");

        HFONT hFontUi = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                    ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                    CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");

        HFONT hFontBold = CreateFontA(14, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                                      ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                      CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");

        HFONT hFontMono = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                      ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                      CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Consolas");

        /* Header Title */
        HWND hTitle = CreateWindowA("STATIC", "POSIX Inter-Process Communication Simulator (Team CodeX)",
                      WS_CHILD | WS_VISIBLE | SS_LEFT,
                      20, 10, 800, 22, hwnd, NULL, NULL, NULL);
        SendMessageA(hTitle, WM_SETFONT, (WPARAM)hFontTitle, TRUE);

        /* User Input Controls */
        HWND lbl1 = CreateWindowA("STATIC", "Parameter 1 (Num1 / Msg / Query / Payload):",
                                  WS_CHILD | WS_VISIBLE, 20, 36, 380, 18, hwnd, NULL, NULL, NULL);
        SendMessageA(lbl1, WM_SETFONT, (WPARAM)hFontUi, TRUE);

        g_hwndInput1 = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "15",
                                       WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                                       20, 56, 380, 24, hwnd, (HMENU)ID_EDIT_INPUT1, NULL, NULL);
        SendMessageA(g_hwndInput1, WM_SETFONT, (WPARAM)hFontUi, TRUE);

        HWND lbl2 = CreateWindowA("STATIC", "Parameter 2 (Num2 / Priority / Workers / Threads):",
                                  WS_CHILD | WS_VISIBLE, 430, 36, 380, 18, hwnd, NULL, NULL, NULL);
        SendMessageA(lbl2, WM_SETFONT, (WPARAM)hFontUi, TRUE);

        g_hwndInput2 = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "25",
                                       WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                                       430, 56, 380, 24, hwnd, (HMENU)ID_EDIT_INPUT2, NULL, NULL);
        SendMessageA(g_hwndInput2, WM_SETFONT, (WPARAM)hFontUi, TRUE);

        /* Mechanism Buttons */
        int bx = 20, by = 90;
        HWND btn;

        btn = CreateWindowA("BUTTON", "1. Run Pipes", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                            bx, by, 120, 30, hwnd, (HMENU)ID_BTN_PIPES, NULL, NULL);
        SendMessageA(btn, WM_SETFONT, (WPARAM)hFontBold, TRUE);
        bx += 130;

        btn = CreateWindowA("BUTTON", "2. Message Queue", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                            bx, by, 140, 30, hwnd, (HMENU)ID_BTN_MQ, NULL, NULL);
        SendMessageA(btn, WM_SETFONT, (WPARAM)hFontBold, TRUE);
        bx += 150;

        btn = CreateWindowA("BUTTON", "3. Sockets", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                            bx, by, 120, 30, hwnd, (HMENU)ID_BTN_SOCKETS, NULL, NULL);
        SendMessageA(btn, WM_SETFONT, (WPARAM)hFontBold, TRUE);
        bx += 130;

        btn = CreateWindowA("BUTTON", "4. Shared Memory", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                            bx, by, 135, 30, hwnd, (HMENU)ID_BTN_SHM, NULL, NULL);
        SendMessageA(btn, WM_SETFONT, (WPARAM)hFontBold, TRUE);
        bx += 145;

        btn = CreateWindowA("BUTTON", "5. Semaphores", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                            bx, by, 125, 30, hwnd, (HMENU)ID_BTN_SEM, NULL, NULL);
        SendMessageA(btn, WM_SETFONT, (WPARAM)hFontBold, TRUE);
        bx += 135;

        btn = CreateWindowA("BUTTON", "6. Mutex", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                            bx, by, 110, 30, hwnd, (HMENU)ID_BTN_MUTEX, NULL, NULL);
        SendMessageA(btn, WM_SETFONT, (WPARAM)hFontBold, TRUE);

        /* Action Buttons Row 2 */
        bx = 20; by = 126;
        btn = CreateWindowA("BUTTON", "⚡ Run All 6 Tests", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                            bx, by, 140, 28, hwnd, (HMENU)ID_BTN_RUN_ALL, NULL, NULL);
        SendMessageA(btn, WM_SETFONT, (WPARAM)hFontBold, TRUE);
        bx += 150;

        btn = CreateWindowA("BUTTON", "Clear Log", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                            bx, by, 95, 28, hwnd, (HMENU)ID_BTN_CLEAR, NULL, NULL);
        SendMessageA(btn, WM_SETFONT, (WPARAM)hFontUi, TRUE);
        bx += 110;

        /* Status Label */
        g_hwndStatus = CreateWindowA("STATIC", "Status: Ready. Enter inputs above and click any mechanism button.",
                                     WS_CHILD | WS_VISIBLE | SS_LEFT,
                                     bx, by + 4, 500, 20, hwnd, (HMENU)ID_STATUS_LABEL, NULL, NULL);
        SendMessageA(g_hwndStatus, WM_SETFONT, (WPARAM)hFontUi, TRUE);

        /* Process Architecture & Roles Box */
        HWND grpDesc = CreateWindowA("BUTTON", " Process Architecture & Communication Roles ",
                                     WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                                     20, 160, 830, 85, hwnd, NULL, NULL, NULL);
        SendMessageA(grpDesc, WM_SETFONT, (WPARAM)hFontBold, TRUE);

        g_hwndDesc = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", DESC_DEFAULT,
                                     WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY,
                                     30, 180, 810, 56, hwnd, (HMENU)ID_EDIT_DESC, NULL, NULL);
        SendMessageA(g_hwndDesc, WM_SETFONT, (WPARAM)hFontUi, TRUE);

        /* Execution Log Box - Strictly VERTICAL scroll (NO horizontal scroll) */
        HWND grpLog = CreateWindowA("BUTTON", " Simple Execution Log (Vertical Scroll) ",
                                    WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                                    20, 252, 830, 410, hwnd, NULL, NULL, NULL);
        SendMessageA(grpLog, WM_SETFONT, (WPARAM)hFontBold, TRUE);

        g_hwndOutput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
                                       WS_CHILD | WS_VISIBLE | WS_VSCROLL |
                                       ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
                                       30, 274, 810, 378, hwnd, (HMENU)ID_EDIT_OUTPUT, NULL, NULL);
        SendMessageA(g_hwndOutput, WM_SETFONT, (WPARAM)hFontMono, TRUE);

        append_gui_output("============================================================\r\n");
        append_gui_output("          POSIX IPC MECHANISMS - SIMPLE C LOG VIEWER        \r\n");
        append_gui_output("============================================================\r\n");
        append_gui_output("• Pure C GUI running natively on Windows.\r\n");
        append_gui_output("• Automatic word-wrapping enabled (no horizontal scrolling).\r\n");
        append_gui_output("• Ready. Enter inputs above and click any mechanism button.\r\n\r\n");
        break;
    }

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        raw_buffer[0] = '\0';
        GetWindowTextA(g_hwndInput1, in1, sizeof(in1) - 1);
        GetWindowTextA(g_hwndInput2, in2, sizeof(in2) - 1);

        switch (wmId) {
        case ID_BTN_PIPES:
        {
            long long n1 = atoll(strlen(in1) ? in1 : "15");
            long long n2 = atoll(strlen(in2) ? in2 : "25");
            set_process_description(DESC_PIPES);
            update_gui_status("Status: Executing POSIX Pipes Demo...");
            append_gui_output(">>> [1. POSIX ANONYMOUS PIPES] <<<\r\n");
            execute_pipes_demo(n1, n2, "PIPE_OP", raw_buffer, sizeof(raw_buffer));
            append_clean_log(raw_buffer);
            append_gui_output("\r\n");
            update_gui_status("Status: POSIX Pipes Demo completed successfully.");
            break;
        }

        case ID_BTN_MQ:
        {
            const char *msg = strlen(in1) ? in1 : "CRITICAL: Reactor Temperature Alert";
            int prio = atoi(strlen(in2) ? in2 : "10");
            set_process_description(DESC_MQ);
            update_gui_status("Status: Executing POSIX Message Queue Demo...");
            append_gui_output(">>> [2. POSIX MESSAGE QUEUES] <<<\r\n");
            execute_message_queue_demo(msg, prio, raw_buffer, sizeof(raw_buffer));
            append_clean_log(raw_buffer);
            append_gui_output("\r\n");
            update_gui_status("Status: POSIX Message Queue Demo completed successfully.");
            break;
        }

        case ID_BTN_SOCKETS:
        {
            const char *query = strlen(in1) ? in1 : "QUERY_SYSTEM_LOAD_METRICS";
            set_process_description(DESC_SOCKETS);
            update_gui_status("Status: Executing Stream Sockets Demo...");
            append_gui_output(">>> [3. POSIX STREAM SOCKETS] <<<\r\n");
            execute_sockets_demo(query, raw_buffer, sizeof(raw_buffer));
            append_clean_log(raw_buffer);
            append_gui_output("\r\n");
            update_gui_status("Status: Stream Sockets Demo completed successfully.");
            break;
        }

        case ID_BTN_SHM:
        {
            const char *data = strlen(in1) ? in1 : "Shared Matrix State: 1024x1024 Buffer";
            int tx = atoi(strlen(in2) ? in2 : "9001");
            set_process_description(DESC_SHM);
            update_gui_status("Status: Executing POSIX Shared Memory Demo...");
            append_gui_output(">>> [4. POSIX SHARED MEMORY] <<<\r\n");
            execute_shared_memory_demo(data, tx, raw_buffer, sizeof(raw_buffer));
            append_clean_log(raw_buffer);
            append_gui_output("\r\n");
            update_gui_status("Status: POSIX Shared Memory Demo completed successfully.");
            break;
        }

        case ID_BTN_SEM:
        {
            int w = atoi(strlen(in1) ? in1 : "4");
            int it = atoi(strlen(in2) ? in2 : "25000");
            set_process_description(DESC_SEM);
            update_gui_status("Status: Executing POSIX Semaphores Demo...");
            append_gui_output(">>> [5. POSIX SEMAPHORES] <<<\r\n");
            execute_semaphores_demo(w, it, raw_buffer, sizeof(raw_buffer));
            append_clean_log(raw_buffer);
            append_gui_output("\r\n");
            update_gui_status("Status: POSIX Semaphores Demo completed successfully.");
            break;
        }

        case ID_BTN_MUTEX:
        {
            double b = atof(strlen(in1) ? in1 : "10000.0");
            int ops = atoi(strlen(in2) ? in2 : "10000");
            set_process_description(DESC_MUTEX);
            update_gui_status("Status: Executing POSIX Mutex Locks Demo...");
            append_gui_output(">>> [6. POSIX MUTEX LOCKS] <<<\r\n");
            execute_mutex_demo(b, 4, ops, raw_buffer, sizeof(raw_buffer));
            append_clean_log(raw_buffer);
            append_gui_output("\r\n");
            update_gui_status("Status: POSIX Mutex Locks Demo completed successfully.");
            break;
        }

        case ID_BTN_RUN_ALL:
            set_process_description(DESC_RUN_ALL);
            update_gui_status("Status: Running all 6 POSIX IPC mechanisms sequentially...");
            append_gui_output("============================================================\r\n");
            append_gui_output("             RUNNING ALL 6 POSIX IPC MECHANISMS             \r\n");
            append_gui_output("============================================================\r\n");

            execute_pipes_demo(15, 25, "SUITE", raw_buffer, sizeof(raw_buffer));
            append_clean_log(raw_buffer);

            execute_message_queue_demo("SUITE: Critical Alert", 10, raw_buffer, sizeof(raw_buffer));
            append_clean_log(raw_buffer);

            execute_sockets_demo("SUITE_QUERY", raw_buffer, sizeof(raw_buffer));
            append_clean_log(raw_buffer);

            execute_shared_memory_demo("SUITE_RAM_DATA", 9001, raw_buffer, sizeof(raw_buffer));
            append_clean_log(raw_buffer);

            execute_semaphores_demo(4, 25000, raw_buffer, sizeof(raw_buffer));
            append_clean_log(raw_buffer);

            execute_mutex_demo(10000.0, 4, 10000, raw_buffer, sizeof(raw_buffer));
            append_clean_log(raw_buffer);

            append_gui_output("\r\n✔ ALL 6 IPC MECHANISMS EXECUTED SUCCESSFULLY!\r\n\r\n");
            update_gui_status("Status: All 6 IPC mechanisms completed successfully!");
            break;

        case ID_BTN_CLEAR:
            SetWindowTextA(g_hwndOutput, "");
            set_process_description(DESC_DEFAULT);
            update_gui_status("Status: Log view cleared. Ready.");
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
    const char CLASS_NAME[] = "SimplePosixIpcGuiClass";

    WNDCLASSA wc = {0};
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);

    RegisterClassA(&wc);

    int win_w = 885;
    int win_h = 715;
    int screen_w = GetSystemMetrics(SM_CXSCREEN);
    int screen_h = GetSystemMetrics(SM_CYSCREEN);
    int pos_x = (screen_w - win_w) / 2;
    int pos_y = (screen_h - win_h) / 2;
    if (pos_x < 0) pos_x = 20;
    if (pos_y < 0) pos_y = 20;

    HWND hwnd = CreateWindowExA(
        WS_EX_APPWINDOW, CLASS_NAME,
        "POSIX IPC Simulator (Simple C GUI) - Team CodeX",
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
    printf("\n[Simple C GUI: Native Win32 window is available on Windows]\n\n");
    return 0;
}
#endif
