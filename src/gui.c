/**
 * @file gui.c
 * @brief Simple Graphical User Interface in C with Inputs and Live Telemetry.
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

static HWND g_hwndInput1 = NULL;
static HWND g_hwndInput2 = NULL;
static HWND g_hwndOutput = NULL;
static HWND g_hwndStatus = NULL;

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

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static char output_buffer[16384];
    char in1[256] = {0};
    char in2[256] = {0};

    switch (msg) {
    case WM_CREATE:
    {
        HFONT hFontMono = CreateFontA(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                      ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                      DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Consolas");

        HFONT hFontUi = CreateFontA(14, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                                    ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                    DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");

        /* Title */
        HWND hTitle = CreateWindowA("STATIC", "POSIX Inter-Process Communication Simulator (Team CodeX)",
                      WS_CHILD | WS_VISIBLE | SS_LEFT,
                      20, 12, 650, 22, hwnd, NULL, NULL, NULL);
        SendMessageA(hTitle, WM_SETFONT, (WPARAM)hFontUi, TRUE);

        /* Input Controls */
        HWND lbl1 = CreateWindowA("STATIC", "Parameter 1 (Num1 / Msg / Query / Payload):",
                                  WS_CHILD | WS_VISIBLE, 20, 38, 360, 18, hwnd, NULL, NULL, NULL);
        SendMessageA(lbl1, WM_SETFONT, (WPARAM)hFontUi, TRUE);

        g_hwndInput1 = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "45",
                                       WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                                       20, 58, 360, 24, hwnd, (HMENU)ID_EDIT_INPUT1, NULL, NULL);
        SendMessageA(g_hwndInput1, WM_SETFONT, (WPARAM)hFontUi, TRUE);

        HWND lbl2 = CreateWindowA("STATIC", "Parameter 2 (Num2 / Priority / Workers / Threads):",
                                  WS_CHILD | WS_VISIBLE, 400, 38, 360, 18, hwnd, NULL, NULL, NULL);
        SendMessageA(lbl2, WM_SETFONT, (WPARAM)hFontUi, TRUE);

        g_hwndInput2 = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "55",
                                       WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                                       400, 58, 360, 24, hwnd, (HMENU)ID_EDIT_INPUT2, NULL, NULL);
        SendMessageA(g_hwndInput2, WM_SETFONT, (WPARAM)hFontUi, TRUE);

        /* Action Buttons */
        HWND btn;
        btn = CreateWindowA("BUTTON", "1. Run Pipes", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                            20, 92, 115, 30, hwnd, (HMENU)ID_BTN_PIPES, NULL, NULL);
        SendMessageA(btn, WM_SETFONT, (WPARAM)hFontUi, TRUE);

        btn = CreateWindowA("BUTTON", "2. Message Queue", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                            145, 92, 130, 30, hwnd, (HMENU)ID_BTN_MQ, NULL, NULL);
        SendMessageA(btn, WM_SETFONT, (WPARAM)hFontUi, TRUE);

        btn = CreateWindowA("BUTTON", "3. Sockets", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                            285, 92, 115, 30, hwnd, (HMENU)ID_BTN_SOCKETS, NULL, NULL);
        SendMessageA(btn, WM_SETFONT, (WPARAM)hFontUi, TRUE);

        btn = CreateWindowA("BUTTON", "4. Shared Memory", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                            410, 92, 130, 30, hwnd, (HMENU)ID_BTN_SHM, NULL, NULL);
        SendMessageA(btn, WM_SETFONT, (WPARAM)hFontUi, TRUE);

        btn = CreateWindowA("BUTTON", "5. Semaphores", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                            550, 92, 110, 30, hwnd, (HMENU)ID_BTN_SEM, NULL, NULL);
        SendMessageA(btn, WM_SETFONT, (WPARAM)hFontUi, TRUE);

        btn = CreateWindowA("BUTTON", "6. Mutex", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                            670, 92, 90, 30, hwnd, (HMENU)ID_BTN_MUTEX, NULL, NULL);
        SendMessageA(btn, WM_SETFONT, (WPARAM)hFontUi, TRUE);

        btn = CreateWindowA("BUTTON", "Run All 6 Tests", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                            20, 128, 140, 28, hwnd, (HMENU)ID_BTN_RUN_ALL, NULL, NULL);
        SendMessageA(btn, WM_SETFONT, (WPARAM)hFontUi, TRUE);

        btn = CreateWindowA("BUTTON", "Clear Log", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                            170, 128, 100, 28, hwnd, (HMENU)ID_BTN_CLEAR, NULL, NULL);
        SendMessageA(btn, WM_SETFONT, (WPARAM)hFontUi, TRUE);

        /* Status Label */
        g_hwndStatus = CreateWindowA("STATIC", "Status: Ready. Enter inputs above and click any mechanism button.",
                                     WS_CHILD | WS_VISIBLE | SS_LEFT,
                                     280, 134, 480, 20, hwnd, (HMENU)ID_STATUS_LABEL, NULL, NULL);
        SendMessageA(g_hwndStatus, WM_SETFONT, (WPARAM)hFontUi, TRUE);

        /* Output View */
        g_hwndOutput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
                                       WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL |
                                       ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_READONLY,
                                       20, 165, 740, 385, hwnd, (HMENU)ID_EDIT_OUTPUT, NULL, NULL);
        SendMessageA(g_hwndOutput, WM_SETFONT, (WPARAM)hFontMono, TRUE);

        append_gui_output("=======================================================================\r\n");
        append_gui_output("       POSIX IPC MECHANISMS GUI - C SYSTEM EXECUTION LOGGER           \r\n");
        append_gui_output("=======================================================================\r\n");
        append_gui_output("All operations stream step-by-step telemetry logs into this window and sim.log\r\n\r\n");
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
            update_gui_status("Status: Executing POSIX Pipes Demo...");
            execute_pipes_demo(n1, n2, "GUI_PIPE_OP", output_buffer, sizeof(output_buffer));
            append_gui_output(output_buffer);
            update_gui_status("Status: POSIX Pipes Demo completed successfully.");
            break;
        }

        case ID_BTN_MQ:
        {
            const char *msg = strlen(in1) ? in1 : "CRITICAL: Reactor Temperature High";
            int prio = atoi(strlen(in2) ? in2 : "10");
            update_gui_status("Status: Executing POSIX Message Queue Demo...");
            execute_message_queue_demo(msg, prio, output_buffer, sizeof(output_buffer));
            append_gui_output(output_buffer);
            update_gui_status("Status: POSIX Message Queue Demo completed successfully.");
            break;
        }

        case ID_BTN_SOCKETS:
        {
            const char *query = strlen(in1) ? in1 : "QUERY_SYSTEM_LOAD_METRICS";
            update_gui_status("Status: Executing POSIX Stream Sockets Demo...");
            execute_sockets_demo(query, output_buffer, sizeof(output_buffer));
            append_gui_output(output_buffer);
            update_gui_status("Status: POSIX Sockets Demo completed successfully.");
            break;
        }

        case ID_BTN_SHM:
        {
            const char *data = strlen(in1) ? in1 : "Shared Matrix State: 1024x1024 Buffer";
            int tx = atoi(strlen(in2) ? in2 : "9001");
            update_gui_status("Status: Executing POSIX Shared Memory Demo...");
            execute_shared_memory_demo(data, tx, output_buffer, sizeof(output_buffer));
            append_gui_output(output_buffer);
            update_gui_status("Status: POSIX Shared Memory Demo completed successfully.");
            break;
        }

        case ID_BTN_SEM:
        {
            int w = atoi(strlen(in1) ? in1 : "4");
            int it = atoi(strlen(in2) ? in2 : "25000");
            update_gui_status("Status: Executing POSIX Semaphores Demo...");
            execute_semaphores_demo(w, it, output_buffer, sizeof(output_buffer));
            append_gui_output(output_buffer);
            update_gui_status("Status: POSIX Semaphores Demo completed successfully.");
            break;
        }

        case ID_BTN_MUTEX:
        {
            double b = atof(strlen(in1) ? in1 : "10000.0");
            int ops = atoi(strlen(in2) ? in2 : "10000");
            update_gui_status("Status: Executing POSIX Mutex Locks Demo...");
            execute_mutex_demo(b, 4, ops, output_buffer, sizeof(output_buffer));
            append_gui_output(output_buffer);
            update_gui_status("Status: POSIX Mutex Locks Demo completed successfully.");
            break;
        }

        case ID_BTN_RUN_ALL:
            update_gui_status("Status: Running all 6 POSIX IPC mechanisms sequentially...");
            append_gui_output("\r\n>>> RUNNING FULL POSIX IPC SUITE <<<\r\n");
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

    HWND hwnd = CreateWindowExA(
        0, CLASS_NAME,
        "POSIX IPC Simulator (Simple GUI in C) - Team CodeX",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
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
