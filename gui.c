/* gui.c - Native Graphical User Interface (Win32) for CPU Scheduling Simulator.
 * Features:
 *   - Visual Process Queue management (Add, Sample, Random, Clear)
 *   - One-click algorithm execution (FCFS, SJF NP/Preemptive, Priority NP/Preemptive, Round Robin)
 *   - Graphical Gantt Chart with colored blocks, process labels, and timeline markers
 *   - Detailed Process Execution Table (CT, TAT, WT, RT)
 *   - Side-by-side comparison of all 6 algorithms
 *   - Zero external dependencies: pure Win32 C code.
 */

#include "gui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>

/* Control IDs */
enum {
    ID_BTN_ADD = 1001,
    ID_BTN_SAMPLE,
    ID_BTN_RANDOM,
    ID_BTN_CLEAR,
    ID_BTN_FCFS,
    ID_BTN_SJF_NP,
    ID_BTN_SJF_P,
    ID_BTN_PRIO_NP,
    ID_BTN_PRIO_P,
    ID_BTN_RR,
    ID_BTN_COMPARE_ALL,
    ID_EDIT_PID,
    ID_EDIT_AT,
    ID_EDIT_BT,
    ID_EDIT_PR,
    ID_EDIT_TQ,
    ID_EDIT_OUTPUT,
    ID_GANTT_CANVAS
};

/* State */
static Process s_procs[MAX_PROC];
static int     s_proc_count = 0;
static Process s_result_procs[MAX_PROC];
static int     s_result_count = 0;
static Gantt   s_gantt;
static Stats   s_stats;
static char    s_current_algo[64] = "None (Select algorithm above)";
static int     s_has_run = 0;

/* Win32 Handles */
static HWND s_hWndMain     = NULL;
static HWND s_hEditPID     = NULL;
static HWND s_hEditAT      = NULL;
static HWND s_hEditBT      = NULL;
static HWND s_hEditPR      = NULL;
static HWND s_hEditTQ      = NULL;
static HWND s_hEditOutput  = NULL;
static HWND s_hGanttCanvas = NULL;
static HWND s_hStatusLabel = NULL;
static HFONT s_hFontUI     = NULL;
static HFONT s_hFontMono   = NULL;
static HFONT s_hFontBold   = NULL;

/* Color palette for process blocks in Gantt chart */
static const COLORREF PROCESS_COLORS[] = {
    RGB(66, 133, 244),   /* Dodger Blue */
    RGB(52, 168, 83),    /* Emerald Green */
    RGB(251, 188, 5),    /* Amber Yellow */
    RGB(234, 67, 53),    /* Coral Red */
    RGB(155, 89, 182),   /* Amethyst Purple */
    RGB(26, 188, 156),   /* Turquoise */
    RGB(230, 126, 34),   /* Carrot Orange */
    RGB(52, 73, 94)      /* Wet Asphalt Slate */
};
#define NUM_COLORS (sizeof(PROCESS_COLORS) / sizeof(PROCESS_COLORS[0]))

/* Helper to set text in output edit box */
static void update_output_text(const char *text)
{
    if (s_hEditOutput) {
        SetWindowTextA(s_hEditOutput, text);
    }
}

static void update_status_count(void)
{
    char buf[128];
    snprintf(buf, sizeof(buf), "Processes Loaded: %d", s_proc_count);
    if (s_hStatusLabel) {
        SetWindowTextA(s_hStatusLabel, buf);
    }
}

/* Format results into monospaced text buffer */
static void refresh_display(void)
{
    char buf[8192];
    int offset = 0;

    offset += snprintf(buf + offset, sizeof(buf) - offset,
                       "========================================================================================\r\n"
                       "  ALGORITHM: %s\r\n"
                       "========================================================================================\r\n\r\n",
                       s_current_algo);

    if (s_has_run && s_result_count > 0) {
        offset += snprintf(buf + offset, sizeof(buf) - offset,
                           "%-6s %-10s %-8s %-10s %-8s %-8s %-8s %-8s\r\n",
                           "PID", "Arrival", "Burst", "Priority", "CT", "WT", "TAT", "RT");
        offset += snprintf(buf + offset, sizeof(buf) - offset,
                           "------ ---------- -------- ---------- -------- -------- -------- --------\r\n");

        for (int i = 0; i < s_result_count; i++) {
            offset += snprintf(buf + offset, sizeof(buf) - offset,
                               "P%-5d %-10d %-8d %-10d %-8d %-8d %-8d %-8d\r\n",
                               s_result_procs[i].pid,
                               s_result_procs[i].at,
                               s_result_procs[i].bt,
                               s_result_procs[i].pr,
                               s_result_procs[i].ct,
                               s_result_procs[i].wt,
                               s_result_procs[i].tat,
                               s_result_procs[i].rt);
        }

        offset += snprintf(buf + offset, sizeof(buf) - offset,
                           "\r\n----------------------------------------------------------------------------------------\r\n"
                           "PERFORMANCE SUMMARY METRICS:\r\n"
                           "  Average Waiting Time (WT)    : %.2f ms\r\n"
                           "  Average Turnaround Time (TAT): %.2f ms\r\n"
                           "  Average Response Time (RT)   : %.2f ms\r\n"
                           "  CPU Utilization              : %.2f %%\r\n"
                           "  Total Completion Time        : %d ms\r\n"
                           "----------------------------------------------------------------------------------------\r\n",
                           s_stats.avg_wt, s_stats.avg_tat, s_stats.avg_rt, s_stats.cpu_util,
                           (s_gantt.count > 0 ? s_gantt.e[s_gantt.count - 1].end : 0));
    } else {
        if (s_proc_count == 0) {
            offset += snprintf(buf + offset, sizeof(buf) - offset,
                               "No processes loaded.\r\n"
                               "Use '+ Add Process', 'Load Sample', or 'Random (4)' to add processes,\r\n"
                               "then select a scheduling algorithm to view results and graphical Gantt chart.\r\n");
        } else {
            offset += snprintf(buf + offset, sizeof(buf) - offset,
                               "Current Processes in Queue (%d):\r\n\r\n", s_proc_count);
            offset += snprintf(buf + offset, sizeof(buf) - offset,
                               "%-6s %-10s %-8s %-10s\r\n", "PID", "Arrival", "Burst", "Priority");
            offset += snprintf(buf + offset, sizeof(buf) - offset,
                               "------ ---------- -------- ----------\r\n");
            for (int i = 0; i < s_proc_count; i++) {
                offset += snprintf(buf + offset, sizeof(buf) - offset,
                                   "P%-5d %-10d %-8d %-10d\r\n",
                                   s_procs[i].pid, s_procs[i].at, s_procs[i].bt, s_procs[i].pr);
            }
            offset += snprintf(buf + offset, sizeof(buf) - offset,
                               "\r\nSelect an algorithm above to run scheduling simulation.\r\n");
        }
    }

    update_output_text(buf);
    if (s_hGanttCanvas) {
        InvalidateRect(s_hGanttCanvas, NULL, TRUE);
        UpdateWindow(s_hGanttCanvas);
    }
}

/* Run chosen algorithm */
static void execute_algorithm(int algo_id)
{
    if (s_proc_count <= 0) {
        MessageBoxA(s_hWndMain, "Please add at least one process before running an algorithm.",
                    "No Processes", MB_OK | MB_ICONWARNING);
        return;
    }

    /* Copy input processes */
    s_result_count = s_proc_count;
    memcpy(s_result_procs, s_procs, (size_t)s_proc_count * sizeof(Process));
    for (int i = 0; i < s_result_count; i++) {
        s_result_procs[i].ct = s_result_procs[i].first_start = 0;
        s_result_procs[i].wt = s_result_procs[i].tat = s_result_procs[i].rt = 0;
    }
    memset(&s_gantt, 0, sizeof(s_gantt));

    int tq = 2;
    if (s_hEditTQ) {
        char tq_buf[32];
        GetWindowTextA(s_hEditTQ, tq_buf, sizeof(tq_buf));
        int val = atoi(tq_buf);
        if (val >= 1 && val <= 100) tq = val;
    }

    switch (algo_id) {
    case 0:
        snprintf(s_current_algo, sizeof(s_current_algo), "FCFS (First-Come First-Served)");
        fcfs(s_result_procs, s_result_count, &s_gantt);
        break;
    case 1:
        snprintf(s_current_algo, sizeof(s_current_algo), "SJF (Shortest Job First - Non-preemptive)");
        sjf_np(s_result_procs, s_result_count, &s_gantt);
        break;
    case 2:
        snprintf(s_current_algo, sizeof(s_current_algo), "SJF (Preemptive / SRTF)");
        sjf_p(s_result_procs, s_result_count, &s_gantt);
        break;
    case 3:
        snprintf(s_current_algo, sizeof(s_current_algo), "Priority (Non-preemptive)");
        priority_np(s_result_procs, s_result_count, &s_gantt);
        break;
    case 4:
        snprintf(s_current_algo, sizeof(s_current_algo), "Priority (Preemptive)");
        priority_p(s_result_procs, s_result_count, &s_gantt);
        break;
    case 5:
        snprintf(s_current_algo, sizeof(s_current_algo), "Round Robin (Time Quantum = %d)", tq);
        round_robin(s_result_procs, s_result_count, tq, &s_gantt);
        break;
    default:
        return;
    }

    compute_metrics(s_result_procs, s_result_count);
    s_stats = compute_stats(s_result_procs, s_result_count, &s_gantt);
    s_has_run = 1;

    refresh_display();
}

/* Compare all 6 algorithms */
static void execute_compare_all(void)
{
    if (s_proc_count <= 0) {
        MessageBoxA(s_hWndMain, "Please add processes before comparing algorithms.",
                    "No Processes", MB_OK | MB_ICONWARNING);
        return;
    }

    int tq = 2;
    if (s_hEditTQ) {
        char tq_buf[32];
        GetWindowTextA(s_hEditTQ, tq_buf, sizeof(tq_buf));
        int val = atoi(tq_buf);
        if (val >= 1 && val <= 100) tq = val;
    }

    const char *names[6] = {
        "FCFS",
        "SJF (Non-preemptive)",
        "SJF (Preemptive / SRTF)",
        "Priority (Non-preemptive)",
        "Priority (Preemptive)",
        "Round Robin"
    };

    Stats stats[6];
    Process work[MAX_PROC];
    Gantt g;

    for (int a = 0; a < 6; a++) {
        memcpy(work, s_procs, (size_t)s_proc_count * sizeof(Process));
        for (int i = 0; i < s_proc_count; i++) {
            work[i].ct = work[i].first_start = work[i].wt = work[i].tat = work[i].rt = 0;
        }
        memset(&g, 0, sizeof(g));

        switch (a) {
        case 0: fcfs(work, s_proc_count, &g); break;
        case 1: sjf_np(work, s_proc_count, &g); break;
        case 2: sjf_p(work, s_proc_count, &g); break;
        case 3: priority_np(work, s_proc_count, &g); break;
        case 4: priority_p(work, s_proc_count, &g); break;
        case 5: round_robin(work, s_proc_count, tq, &g); break;
        }

        compute_metrics(work, s_proc_count);
        stats[a] = compute_stats(work, s_proc_count, &g);
    }

    /* Find best algorithm for Avg WT */
    int best_wt_idx = 0;
    for (int a = 1; a < 6; a++) {
        if (stats[a].avg_wt < stats[best_wt_idx].avg_wt) {
            best_wt_idx = a;
        }
    }

    char buf[4096];
    int offset = 0;
    offset += snprintf(buf + offset, sizeof(buf) - offset,
                       "========================================================================================\r\n"
                       "  ALGORITHM COMPARISON BENCHMARK (RR Quantum = %d)\r\n"
                       "========================================================================================\r\n\r\n",
                       tq);

    offset += snprintf(buf + offset, sizeof(buf) - offset,
                       "%-28s %-12s %-12s %-12s %-14s\r\n",
                       "Algorithm", "Avg WT (ms)", "Avg TAT (ms)", "Avg RT (ms)", "CPU Util (%)");
    offset += snprintf(buf + offset, sizeof(buf) - offset,
                       "---------------------------- ------------ ------------ ------------ --------------\r\n");

    for (int i = 0; i < 6; i++) {
        offset += snprintf(buf + offset, sizeof(buf) - offset,
                           "%-28s %-12.2f %-12.2f %-12.2f %-13.2f%%\r\n",
                           names[i], stats[i].avg_wt, stats[i].avg_tat, stats[i].avg_rt, stats[i].cpu_util);
    }

    offset += snprintf(buf + offset, sizeof(buf) - offset,
                       "---------------------------- ------------ ------------ ------------ --------------\r\n\r\n"
                       "SUMMARY & INSIGHTS:\r\n"
                       "  * Best Algorithm for Minimum Waiting Time: %s (Avg WT = %.2f ms)\r\n"
                       "  * CPU Utilization across all non-idle ranges maintained optimally.\r\n"
                       "  * Preemptive algorithms reduce response times for burst-intensive workloads.\r\n",
                       names[best_wt_idx], stats[best_wt_idx].avg_wt);

    snprintf(s_current_algo, sizeof(s_current_algo), "All Algorithms Compared (Best: %s)", names[best_wt_idx]);
    update_output_text(buf);

    /* Also run the best algorithm for visual Gantt display */
    execute_algorithm(best_wt_idx);
    /* Re-overwrite output text with the comparison table */
    update_output_text(buf);
}

/* Load 4 sample standard processes */
static void load_sample_processes(void)
{
    s_proc_count = 4;
    s_procs[0].pid = 1; s_procs[0].at = 0; s_procs[0].bt = 5; s_procs[0].pr = 2;
    s_procs[1].pid = 2; s_procs[1].at = 1; s_procs[1].bt = 3; s_procs[1].pr = 1;
    s_procs[2].pid = 3; s_procs[2].at = 2; s_procs[2].bt = 8; s_procs[2].pr = 4;
    s_procs[3].pid = 4; s_procs[3].at = 3; s_procs[3].bt = 6; s_procs[3].pr = 3;

    for (int i = 0; i < s_proc_count; i++) {
        s_procs[i].ct = s_procs[i].first_start = s_procs[i].wt = s_procs[i].tat = s_procs[i].rt = 0;
    }

    s_has_run = 0;
    snprintf(s_current_algo, sizeof(s_current_algo), "Sample Processes Loaded");
    update_status_count();

    /* Set next PID in edit box */
    char next_pid[16];
    snprintf(next_pid, sizeof(next_pid), "%d", s_proc_count + 1);
    SetWindowTextA(s_hEditPID, next_pid);

    refresh_display();
}

/* Generate random processes */
static void generate_random_processes(void)
{
    s_proc_count = 4;
    for (int i = 0; i < s_proc_count; i++) {
        s_procs[i].pid = i + 1;
        s_procs[i].at  = rand() % 8;
        s_procs[i].bt  = 1 + (rand() % 10);
        s_procs[i].pr  = 1 + (rand() % 6);
        s_procs[i].ct  = s_procs[i].first_start = s_procs[i].wt = s_procs[i].tat = s_procs[i].rt = 0;
    }

    s_has_run = 0;
    snprintf(s_current_algo, sizeof(s_current_algo), "Generated 4 Random Processes");
    update_status_count();

    char next_pid[16];
    snprintf(next_pid, sizeof(next_pid), "%d", s_proc_count + 1);
    SetWindowTextA(s_hEditPID, next_pid);

    refresh_display();
}

/* Clear all processes */
static void clear_all_processes(void)
{
    s_proc_count = 0;
    s_result_count = 0;
    s_has_run = 0;
    memset(&s_gantt, 0, sizeof(s_gantt));
    snprintf(s_current_algo, sizeof(s_current_algo), "All Processes Cleared");
    update_status_count();
    SetWindowTextA(s_hEditPID, "1");
    refresh_display();
}

/* Add process from input fields */
static void add_process_from_inputs(void)
{
    if (s_proc_count >= MAX_PROC) {
        MessageBoxA(s_hWndMain, "Maximum process limit reached (100).", "Limit", MB_OK | MB_ICONWARNING);
        return;
    }

    char buf[32];
    int pid = 1, at = 0, bt = 1, pr = 1;

    GetWindowTextA(s_hEditPID, buf, sizeof(buf)); pid = atoi(buf);
    GetWindowTextA(s_hEditAT,  buf, sizeof(buf)); at  = atoi(buf);
    GetWindowTextA(s_hEditBT,  buf, sizeof(buf)); bt  = atoi(buf);
    GetWindowTextA(s_hEditPR,  buf, sizeof(buf)); pr  = atoi(buf);

    if (pid <= 0) pid = s_proc_count + 1;
    if (at < 0)   at = 0;
    if (bt <= 0)  bt = 1;
    if (pr < 0)   pr = 1;

    /* Check duplicate PID */
    for (int i = 0; i < s_proc_count; i++) {
        if (s_procs[i].pid == pid) {
            char warn[128];
            snprintf(warn, sizeof(warn), "Process ID %d already exists. Please choose a different PID.", pid);
            MessageBoxA(s_hWndMain, warn, "Duplicate PID", MB_OK | MB_ICONWARNING);
            return;
        }
    }

    s_procs[s_proc_count].pid = pid;
    s_procs[s_proc_count].at  = at;
    s_procs[s_proc_count].bt  = bt;
    s_procs[s_proc_count].pr  = pr;
    s_procs[s_proc_count].ct = s_procs[s_proc_count].first_start = 0;
    s_procs[s_proc_count].wt = s_procs[s_proc_count].tat = s_procs[s_proc_count].rt = 0;
    s_proc_count++;

    s_has_run = 0;
    snprintf(s_current_algo, sizeof(s_current_algo), "Added Process P%d", pid);
    update_status_count();

    /* Next suggested PID */
    snprintf(buf, sizeof(buf), "%d", pid + 1);
    SetWindowTextA(s_hEditPID, buf);

    refresh_display();
}

/* Paint Gantt Chart canvas */
static void paint_gantt_chart(HWND hwnd, HDC hdc)
{
    RECT rect;
    GetClientRect(hwnd, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    /* Double-buffering */
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBM = CreateCompatibleBitmap(hdc, width, height);
    HBITMAP oldBM = (HBITMAP)SelectObject(memDC, memBM);

    /* Fill background */
    HBRUSH bgBrush = CreateSolidBrush(RGB(248, 250, 252));
    FillRect(memDC, &rect, bgBrush);
    DeleteObject(bgBrush);

    /* Draw border */
    HPEN borderPen = CreatePen(PS_SOLID, 1, RGB(226, 232, 240));
    HPEN oldPen = (HPEN)SelectObject(memDC, borderPen);
    Rectangle(memDC, 0, 0, width, height);

    SelectObject(memDC, s_hFontUI);
    SetBkMode(memDC, TRANSPARENT);

    if (!s_has_run || s_gantt.count == 0) {
        SetTextColor(memDC, RGB(100, 116, 139));
        RECT textRect = rect;
        DrawTextA(memDC, "Gantt Chart Visualization will render here after running an algorithm.",
                  -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    } else {
        int total_span = s_gantt.e[s_gantt.count - 1].end;
        if (total_span <= 0) total_span = 1;

        int margin_left = 30;
        int margin_right = 30;
        int chart_w = width - margin_left - margin_right;
        int block_y = 20;
        int block_h = 42;

        /* Title / header in canvas */
        SetTextColor(memDC, RGB(30, 41, 59));
        SelectObject(memDC, s_hFontBold);
        char title_buf[128];
        snprintf(title_buf, sizeof(title_buf), "Timeline: 0 -> %d ms (Total Blocks: %d)", total_span, s_gantt.count);
        TextOutA(memDC, margin_left, 4, title_buf, (int)strlen(title_buf));

        SelectObject(memDC, s_hFontUI);

        /* Draw Gantt blocks */
        for (int i = 0; i < s_gantt.count; i++) {
            int start_t = s_gantt.e[i].start;
            int end_t   = s_gantt.e[i].end;
            int pid     = s_gantt.e[i].pid;

            int x1 = margin_left + (int)(((double)start_t / (double)total_span) * (double)chart_w);
            int x2 = margin_left + (int)(((double)end_t   / (double)total_span) * (double)chart_w);
            if (x2 <= x1) x2 = x1 + 1;

            RECT blockRect;
            blockRect.left   = x1;
            blockRect.top    = block_y;
            blockRect.right  = x2;
            blockRect.bottom = block_y + block_h;

            COLORREF blockColor;
            COLORREF textColor;
            char label[32];

            if (pid == -1) {
                blockColor = RGB(203, 213, 225); /* Slate gray for IDLE */
                textColor  = RGB(71, 85, 105);
                snprintf(label, sizeof(label), "IDLE");
            } else {
                blockColor = PROCESS_COLORS[(pid - 1) % NUM_COLORS];
                textColor  = RGB(255, 255, 255);
                snprintf(label, sizeof(label), "P%d", pid);
            }

            /* Draw block */
            HBRUSH brush = CreateSolidBrush(blockColor);
            FillRect(memDC, &blockRect, brush);
            DeleteObject(brush);

            /* Block outline */
            HPEN bPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
            HPEN pOld = (HPEN)SelectObject(memDC, bPen);
            MoveToEx(memDC, blockRect.left, blockRect.top, NULL);
            LineTo(memDC, blockRect.right, blockRect.top);
            LineTo(memDC, blockRect.right, blockRect.bottom);
            LineTo(memDC, blockRect.left, blockRect.bottom);
            LineTo(memDC, blockRect.left, blockRect.top);
            SelectObject(memDC, pOld);
            DeleteObject(bPen);

            /* Label text */
            if (x2 - x1 >= 18) {
                SetTextColor(memDC, textColor);
                SelectObject(memDC, s_hFontBold);
                DrawTextA(memDC, label, -1, &blockRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                SelectObject(memDC, s_hFontUI);
            }

            /* Time ticks underneath */
            SetTextColor(memDC, RGB(51, 65, 85));
            char time_str[16];

            /* Start tick for first block */
            if (i == 0) {
                snprintf(time_str, sizeof(time_str), "%d", start_t);
                TextOutA(memDC, x1 - 4, block_y + block_h + 4, time_str, (int)strlen(time_str));
            }

            /* End tick */
            snprintf(time_str, sizeof(time_str), "%d", end_t);
            int txt_len = (int)strlen(time_str);
            TextOutA(memDC, x2 - (txt_len * 4), block_y + block_h + 4, time_str, txt_len);
        }
    }

    SelectObject(memDC, oldPen);
    DeleteObject(borderPen);

    /* Blit to screen */
    BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

    SelectObject(memDC, oldBM);
    DeleteObject(memBM);
    DeleteDC(memDC);
}

/* Canvas Window Procedure */
static LRESULT CALLBACK GanttCanvasWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        paint_gantt_chart(hwnd, hdc);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_ERASEBKGND:
        return 1; /* Handled in WM_PAINT */
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

/* Main Window Procedure */
static LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_CREATE: {
        /* Create UI elements */
        int x = 20, y = 15;

        /* Group 1: Process Input */
        HWND hGrpInput = CreateWindowA("BUTTON", " Process Input & Configuration ",
                                      WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                                      x, y, 920, 85, hwnd, NULL, NULL, NULL);
        SendMessageA(hGrpInput, WM_SETFONT, (WPARAM)s_hFontBold, TRUE);

        /* Input Labels & Edit boxes */
        int ix = x + 15, iy = y + 25;
        HWND hLblPID = CreateWindowA("STATIC", "PID:", WS_CHILD | WS_VISIBLE, ix, iy + 4, 32, 20, hwnd, NULL, NULL, NULL);
        s_hEditPID   = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "1", WS_CHILD | WS_VISIBLE | ES_NUMBER, ix + 35, iy, 45, 24, hwnd, (HMENU)ID_EDIT_PID, NULL, NULL);

        ix += 95;
        HWND hLblAT = CreateWindowA("STATIC", "Arrival:", WS_CHILD | WS_VISIBLE, ix, iy + 4, 48, 20, hwnd, NULL, NULL, NULL);
        s_hEditAT   = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "0", WS_CHILD | WS_VISIBLE | ES_NUMBER, ix + 52, iy, 45, 24, hwnd, (HMENU)ID_EDIT_AT, NULL, NULL);

        ix += 110;
        HWND hLblBT = CreateWindowA("STATIC", "Burst:", WS_CHILD | WS_VISIBLE, ix, iy + 4, 42, 20, hwnd, NULL, NULL, NULL);
        s_hEditBT   = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "4", WS_CHILD | WS_VISIBLE | ES_NUMBER, ix + 45, iy, 45, 24, hwnd, (HMENU)ID_EDIT_BT, NULL, NULL);

        ix += 105;
        HWND hLblPR = CreateWindowA("STATIC", "Priority:", WS_CHILD | WS_VISIBLE, ix, iy + 4, 52, 20, hwnd, NULL, NULL, NULL);
        s_hEditPR   = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "1", WS_CHILD | WS_VISIBLE | ES_NUMBER, ix + 55, iy, 45, 24, hwnd, (HMENU)ID_EDIT_PR, NULL, NULL);

        /* Action Buttons */
        ix += 120;
        HWND hBtnAdd = CreateWindowA("BUTTON", "+ Add Process", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                    ix, iy - 2, 110, 28, hwnd, (HMENU)ID_BTN_ADD, NULL, NULL);

        ix += 120;
        HWND hBtnSample = CreateWindowA("BUTTON", "Load Sample (4)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                       ix, iy - 2, 120, 28, hwnd, (HMENU)ID_BTN_SAMPLE, NULL, NULL);

        ix += 130;
        HWND hBtnRandom = CreateWindowA("BUTTON", "Random (4)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                       ix, iy - 2, 95, 28, hwnd, (HMENU)ID_BTN_RANDOM, NULL, NULL);

        ix += 105;
        HWND hBtnClear = CreateWindowA("BUTTON", "Clear All", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                      ix, iy - 2, 85, 28, hwnd, (HMENU)ID_BTN_CLEAR, NULL, NULL);

        /* Status Count Label */
        s_hStatusLabel = CreateWindowA("STATIC", "Processes Loaded: 0", WS_CHILD | WS_VISIBLE,
                                      x + 15, y + 58, 200, 20, hwnd, NULL, NULL, NULL);

        /* Group 2: Scheduling Algorithms */
        y += 95;
        HWND hGrpAlgo = CreateWindowA("BUTTON", " Scheduling Algorithms & Controls ",
                                     WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                                     x, y, 920, 75, hwnd, NULL, NULL, NULL);
        SendMessageA(hGrpAlgo, WM_SETFONT, (WPARAM)s_hFontBold, TRUE);

        int bx = x + 12, by = y + 25;
        HWND hBtnFCFS = CreateWindowA("BUTTON", "FCFS", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                     bx, by, 75, 32, hwnd, (HMENU)ID_BTN_FCFS, NULL, NULL);
        bx += 82;
        HWND hBtnSJFNP = CreateWindowA("BUTTON", "SJF (NP)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                      bx, by, 85, 32, hwnd, (HMENU)ID_BTN_SJF_NP, NULL, NULL);
        bx += 92;
        HWND hBtnSJFP = CreateWindowA("BUTTON", "SRTF / SJF (P)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                     bx, by, 110, 32, hwnd, (HMENU)ID_BTN_SJF_P, NULL, NULL);
        bx += 117;
        HWND hBtnPrioNP = CreateWindowA("BUTTON", "Priority (NP)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                       bx, by, 100, 32, hwnd, (HMENU)ID_BTN_PRIO_NP, NULL, NULL);
        bx += 107;
        HWND hBtnPrioP = CreateWindowA("BUTTON", "Priority (P)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                      bx, by, 95, 32, hwnd, (HMENU)ID_BTN_PRIO_P, NULL, NULL);
        bx += 102;
        HWND hBtnRR = CreateWindowA("BUTTON", "Round Robin", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                   bx, by, 100, 32, hwnd, (HMENU)ID_BTN_RR, NULL, NULL);
        bx += 105;
        HWND hLblTQ = CreateWindowA("STATIC", "TQ:", WS_CHILD | WS_VISIBLE, bx, by + 6, 26, 20, hwnd, NULL, NULL, NULL);
        s_hEditTQ   = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "2", WS_CHILD | WS_VISIBLE | ES_NUMBER, bx + 28, by + 3, 35, 24, hwnd, (HMENU)ID_EDIT_TQ, NULL, NULL);

        bx += 72;
        HWND hBtnCompare = CreateWindowA("BUTTON", "* COMPARE ALL *", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                        bx, by, 135, 32, hwnd, (HMENU)ID_BTN_COMPARE_ALL, NULL, NULL);

        /* Group 3: Graphical Gantt Chart */
        y += 85;
        HWND hGrpGantt = CreateWindowA("BUTTON", " Visual Gantt Chart Timeline ",
                                      WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                                      x, y, 920, 125, hwnd, NULL, NULL, NULL);
        SendMessageA(hGrpGantt, WM_SETFONT, (WPARAM)s_hFontBold, TRUE);

        s_hGanttCanvas = CreateWindowExA(0, "GanttChartCanvas", "",
                                         WS_CHILD | WS_VISIBLE | WS_BORDER,
                                         x + 12, y + 24, 896, 90, hwnd, (HMENU)ID_GANTT_CANVAS, NULL, NULL);

        /* Group 4: Results & Metrics Log */
        y += 135;
        HWND hGrpResults = CreateWindowA("BUTTON", " Process Execution Table & Performance Metrics ",
                                        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                                        x, y, 920, 310, hwnd, NULL, NULL, NULL);
        SendMessageA(hGrpResults, WM_SETFONT, (WPARAM)s_hFontBold, TRUE);

        s_hEditOutput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
                                        WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL |
                                        ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_READONLY,
                                        x + 12, y + 24, 896, 274, hwnd, (HMENU)ID_EDIT_OUTPUT, NULL, NULL);

        /* Set Fonts */
        SendMessageA(s_hEditPID, WM_SETFONT, (WPARAM)s_hFontUI, TRUE);
        SendMessageA(s_hEditAT,  WM_SETFONT, (WPARAM)s_hFontUI, TRUE);
        SendMessageA(s_hEditBT,  WM_SETFONT, (WPARAM)s_hFontUI, TRUE);
        SendMessageA(s_hEditPR,  WM_SETFONT, (WPARAM)s_hFontUI, TRUE);
        SendMessageA(s_hEditTQ,  WM_SETFONT, (WPARAM)s_hFontUI, TRUE);
        SendMessageA(hLblPID,    WM_SETFONT, (WPARAM)s_hFontUI, TRUE);
        SendMessageA(hLblAT,     WM_SETFONT, (WPARAM)s_hFontUI, TRUE);
        SendMessageA(hLblBT,     WM_SETFONT, (WPARAM)s_hFontUI, TRUE);
        SendMessageA(hLblPR,     WM_SETFONT, (WPARAM)s_hFontUI, TRUE);
        SendMessageA(hLblTQ,     WM_SETFONT, (WPARAM)s_hFontUI, TRUE);
        SendMessageA(s_hStatusLabel, WM_SETFONT, (WPARAM)s_hFontUI, TRUE);

        SendMessageA(hBtnAdd,     WM_SETFONT, (WPARAM)s_hFontBold, TRUE);
        SendMessageA(hBtnSample,  WM_SETFONT, (WPARAM)s_hFontUI, TRUE);
        SendMessageA(hBtnRandom,  WM_SETFONT, (WPARAM)s_hFontUI, TRUE);
        SendMessageA(hBtnClear,   WM_SETFONT, (WPARAM)s_hFontUI, TRUE);
        SendMessageA(hBtnFCFS,    WM_SETFONT, (WPARAM)s_hFontUI, TRUE);
        SendMessageA(hBtnSJFNP,   WM_SETFONT, (WPARAM)s_hFontUI, TRUE);
        SendMessageA(hBtnSJFP,    WM_SETFONT, (WPARAM)s_hFontUI, TRUE);
        SendMessageA(hBtnPrioNP,  WM_SETFONT, (WPARAM)s_hFontUI, TRUE);
        SendMessageA(hBtnPrioP,   WM_SETFONT, (WPARAM)s_hFontUI, TRUE);
        SendMessageA(hBtnRR,      WM_SETFONT, (WPARAM)s_hFontUI, TRUE);
        SendMessageA(hBtnCompare, WM_SETFONT, (WPARAM)s_hFontBold, TRUE);

        SendMessageA(s_hEditOutput, WM_SETFONT, (WPARAM)s_hFontMono, TRUE);

        update_status_count();
        refresh_display();
        return 0;
    }

    case WM_COMMAND: {
        int id = LOWORD(wParam);
        switch (id) {
        case ID_BTN_ADD:         add_process_from_inputs(); break;
        case ID_BTN_SAMPLE:      load_sample_processes();   break;
        case ID_BTN_RANDOM:      generate_random_processes(); break;
        case ID_BTN_CLEAR:       clear_all_processes();     break;
        case ID_BTN_FCFS:        execute_algorithm(0);      break;
        case ID_BTN_SJF_NP:      execute_algorithm(1);      break;
        case ID_BTN_SJF_P:       execute_algorithm(2);      break;
        case ID_BTN_PRIO_NP:     execute_algorithm(3);      break;
        case ID_BTN_PRIO_P:      execute_algorithm(4);      break;
        case ID_BTN_RR:          execute_algorithm(5);      break;
        case ID_BTN_COMPARE_ALL: execute_compare_all();     break;
        }
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void launch_gui(Process initial_procs[], int initial_count)
{
    /* Copy initial processes if passed */
    if (initial_count > 0 && initial_procs != NULL) {
        s_proc_count = initial_count;
        memcpy(s_procs, initial_procs, (size_t)initial_count * sizeof(Process));
    } else {
        /* Default to 4 sample processes for instant, friendly demonstration */
        load_sample_processes();
    }

    HINSTANCE hInstance = GetModuleHandle(NULL);

    /* Initialize fonts */
    s_hFontUI   = CreateFontA(-13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                              DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                              CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    s_hFontBold = CreateFontA(-13, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                              DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                              CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    s_hFontMono = CreateFontA(-13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                              DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                              CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");

    /* Register Canvas Class */
    WNDCLASSEXA wcxCanvas = {0};
    wcxCanvas.cbSize        = sizeof(wcxCanvas);
    wcxCanvas.style         = CS_HREDRAW | CS_VREDRAW;
    wcxCanvas.lpfnWndProc   = GanttCanvasWndProc;
    wcxCanvas.hInstance     = hInstance;
    wcxCanvas.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wcxCanvas.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcxCanvas.lpszClassName = "GanttChartCanvas";
    RegisterClassExA(&wcxCanvas);

    /* Register Main Window Class */
    WNDCLASSEXA wcx = {0};
    wcx.cbSize        = sizeof(wcx);
    wcx.style         = CS_HREDRAW | CS_VREDRAW;
    wcx.lpfnWndProc   = MainWndProc;
    wcx.hInstance     = hInstance;
    wcx.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wcx.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wcx.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wcx.lpszClassName = "CPUSchedulerMainWnd";
    RegisterClassExA(&wcx);

    /* Window dimensions: 980x740 */
    int win_w = 980;
    int win_h = 740;
    int screen_w = GetSystemMetrics(SM_CXSCREEN);
    int screen_h = GetSystemMetrics(SM_CYSCREEN);
    int pos_x = (screen_w - win_w) / 2;
    int pos_y = (screen_h - win_h) / 2;
    if (pos_x < 0) pos_x = 20;
    if (pos_y < 0) pos_y = 20;

    s_hWndMain = CreateWindowExA(
        WS_EX_APPWINDOW,
        "CPUSchedulerMainWnd",
        "CPU Process Scheduling Simulator - CodeX PBL Week 3",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        pos_x, pos_y, win_w, win_h,
        NULL, NULL, hInstance, NULL
    );

    if (!s_hWndMain) {
        MessageBoxA(NULL, "Failed to create main GUI window.", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    ShowWindow(s_hWndMain, SW_SHOWNORMAL);
    UpdateWindow(s_hWndMain);

    /* If we have sample processes, trigger FCFS automatically so the user sees results immediately */
    if (s_proc_count > 0) {
        execute_algorithm(0);
    }

    /* Message Loop */
    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    /* Cleanup fonts */
    if (s_hFontUI)   DeleteObject(s_hFontUI);
    if (s_hFontBold) DeleteObject(s_hFontBold);
    if (s_hFontMono) DeleteObject(s_hFontMono);
}

#else

/* Fallback for non-Windows systems */
void launch_gui(Process initial_procs[], int initial_count)
{
    (void)initial_procs;
    (void)initial_count;
    printf("\n[Notice] Native Win32 GUI is supported on Windows.\n");
    printf("Please run standard CLI interactive options (0-10) in this terminal.\n\n");
}

#endif /* _WIN32 */

