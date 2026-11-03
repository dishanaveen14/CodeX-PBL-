/**
 * @file panel_detection.c
 * @brief Person 3: Deadlock Detection & Recovery GUI Panel Implementation.
 * 
 * Deadlock Simulation System - PBL Project
 * Assigned to: Person 3 (Deadlock Detection, Recovery, and RAG)
 */

#include "../../include/gui/panel_detection.h"
#include "../../include/person3_detection_recovery/detection.h"
#include "../../include/person3_detection_recovery/recovery.h"
#include "../../include/person1_process_resource/matrix_mgmt.h"
#include "../../include/common/error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PANEL_DETECTION_CLASS "DeadlockSimPanelDetection"

/* Control IDs */
enum {
    IDC_BTN_RUN_DETECT = 3001,
    IDC_BTN_RECORD_REQ,
    IDC_BTN_CLEAR_REQ,
    IDC_BTN_SUGGEST_VICTIM,
    IDC_BTN_TERMINATE_VICTIM,
    IDC_COMBO_REQ_PROC,
    IDC_COMBO_VICTIM_PROC,
    IDC_EDIT_WAIT_REQ,
    IDC_STATIC_DETECT_STATUS,
    IDC_STATIC_DEADLOCK_PROCS,
    IDC_EDIT_DETECT_DISPLAY,
    IDC_STATIC_DETECT_FEEDBACK
};

static SystemContext *g_detection_ctx = NULL;
static HWND g_hwnd_detect_panel = NULL;
static HWND g_static_detect_status = NULL;
static HWND g_static_deadlock_procs = NULL;
static HWND g_combo_req_proc = NULL;
static HWND g_combo_victim_proc = NULL;
static HWND g_edit_wait_req = NULL;
static HWND g_edit_detect_display = NULL;
static HWND g_static_feedback = NULL;

static DeadlockResult g_last_detection_res;

static void set_feedback(const char *msg, bool is_error) {
    if (g_static_feedback) {
        char buf[512];
        snprintf(buf, sizeof(buf), "[%s] %s", is_error ? "ALERT" : "INFO", msg);
        SetWindowText(g_static_feedback, buf);
    }
}

static void update_selectors(void) {
    if (!g_detection_ctx) return;
    SystemState *st = &g_detection_ctx->live_state;

    /* Update Waiting Request Process Selector */
    if (g_combo_req_proc) {
        SendMessage(g_combo_req_proc, CB_RESETCONTENT, 0, 0);
        for (int i = 0; i < st->process_count; i++) {
            char buf[64];
            snprintf(buf, sizeof(buf), "%s (Status: %s)", st->processes[i].name, process_status_to_string(st->processes[i].status));
            SendMessage(g_combo_req_proc, CB_ADDSTRING, 0, (LPARAM)buf);
        }
        if (st->process_count > 0) {
            SendMessage(g_combo_req_proc, CB_SETCURSEL, 0, 0);
        }
    }

    /* Update Victim Process Selector */
    if (g_combo_victim_proc) {
        SendMessage(g_combo_victim_proc, CB_RESETCONTENT, 0, 0);
        if (g_last_detection_res.deadlock_detected && g_last_detection_res.deadlocked_count > 0) {
            for (int k = 0; k < g_last_detection_res.deadlocked_count; k++) {
                int pid = g_last_detection_res.deadlocked_processes[k];
                if (pid >= 0 && pid < st->process_count) {
                    char buf[64];
                    int held = 0;
                    for (int j = 0; j < st->resource_count; j++) held += st->allocation[pid][j];
                    snprintf(buf, sizeof(buf), "%s (Held: %d instances)", st->processes[pid].name, held);
                    SendMessage(g_combo_victim_proc, CB_ADDSTRING, 0, (LPARAM)buf);
                }
            }
            SendMessage(g_combo_victim_proc, CB_SETCURSEL, 0, 0);
        }
    }
}

static void render_detection_display(void) {
    if (!g_detection_ctx || !g_edit_detect_display) return;
    SystemState *st = &g_detection_ctx->live_state;

    char text[4096];
    int offset = 0;

    offset += snprintf(text + offset, sizeof(text) - offset,
        "========================================================================================\r\n"
        " DEADLOCK DETECTION & OUTSTANDING REQUEST MATRIX REPORT\r\n"
        "========================================================================================\r\n\r\n");

    /* 1. Outstanding Requests Table */
    offset += snprintf(text + offset, sizeof(text) - offset, "CURRENT PENDING / WAITING REQUEST MATRIX:\r\n");
    offset += snprintf(text + offset, sizeof(text) - offset,
        "%-6s | %-12s | %-24s | %-20s\r\n", "PID", "STATUS", "OUTSTANDING REQUEST", "CURRENT ALLOCATION");
    offset += snprintf(text + offset, sizeof(text) - offset,
        "----------------------------------------------------------------------------------------\r\n");

    for (int i = 0; i < st->process_count; i++) {
        char req_buf[64] = "";
        char alloc_buf[64] = "";

        for (int j = 0; j < st->resource_count; j++) {
            char temp[16];
            snprintf(temp, sizeof(temp), "%d ", st->outstanding_requests[i][j]);
            strcat(req_buf, temp);

            snprintf(temp, sizeof(temp), "%d ", st->allocation[i][j]);
            strcat(alloc_buf, temp);
        }

        offset += snprintf(text + offset, sizeof(text) - offset,
            "%-6s | %-12s | %-24s | %-20s\r\n",
            st->processes[i].name,
            process_status_to_string(st->processes[i].status),
            req_buf, alloc_buf);
    }

    if (st->process_count == 0) {
        offset += snprintf(text + offset, sizeof(text) - offset, "  (No processes configured. Load a preset in Setup tab.)\r\n");
    }

    offset += snprintf(text + offset, sizeof(text) - offset, "\r\nAVAILABLE RESOURCE VECTOR:\r\n  [ ");
    for (int j = 0; j < st->resource_count; j++) {
        offset += snprintf(text + offset, sizeof(text) - offset, "%s:%d ", st->resources[j].name, st->available[j]);
    }
    offset += snprintf(text + offset, sizeof(text) - offset, "]\r\n\r\n");

    /* 2. Deadlock Detection Analysis */
    offset += snprintf(text + offset, sizeof(text) - offset,
        "========================================================================================\r\n"
        " DETECTION ANALYSIS OUTCOME:\r\n"
        "  %s\r\n"
        "========================================================================================\r\n",
        (g_last_detection_res.explanation[0]) ? g_last_detection_res.explanation : "Detection not yet executed.");

    SetWindowText(g_edit_detect_display, text);
}

static void on_run_detection(void) {
    if (!g_detection_ctx) return;
    SystemState *st = &g_detection_ctx->live_state;

    if (st->process_count == 0) {
        set_feedback("Cannot run detection: No processes configured.", true);
        return;
    }

    ErrorCode err = detection_run(st, &g_last_detection_res);
    if (err == ERR_SUCCESS) {
        if (g_last_detection_res.deadlock_detected) {
            st->overall_status = SYSTEM_STATUS_DEADLOCKED;
            SetWindowText(g_static_detect_status, "STATUS: DEADLOCK DETECTED!");

            /* Mark deadlocked process statuses */
            for (int k = 0; k < g_last_detection_res.deadlocked_count; k++) {
                int pid = g_last_detection_res.deadlocked_processes[k];
                if (pid >= 0 && pid < st->process_count) {
                    st->processes[pid].status = PROCESS_STATUS_DEADLOCKED;
                }
            }

            char procs_str[256] = "Deadlocked Processes: [ ";
            for (int k = 0; k < g_last_detection_res.deadlocked_count; k++) {
                int pid = g_last_detection_res.deadlocked_processes[k];
                char temp[32];
                snprintf(temp, sizeof(temp), "%s%s", (k > 0) ? ", " : "", st->processes[pid].name);
                strcat(procs_str, temp);
            }
            strcat(procs_str, " ]");
            SetWindowText(g_static_deadlock_procs, procs_str);

            set_feedback("Deadlock Detection completed: System has deadlocked processes in circular wait!", true);
        } else {
            if (st->overall_status == SYSTEM_STATUS_DEADLOCKED) {
                st->overall_status = SYSTEM_STATUS_UNCHECKED;
            }
            SetWindowText(g_static_detect_status, "STATUS: NO DEADLOCK DETECTED");
            SetWindowText(g_static_deadlock_procs, "Deadlocked Processes: None (All requests resolvable)");
            set_feedback("Deadlock Detection completed: No deadlocks found.", false);
        }

        update_selectors();
        render_detection_display();
    } else {
        set_feedback(error_code_to_string(err), true);
    }
}

static void on_record_waiting_request(void) {
    if (!g_detection_ctx) return;
    int pid = (int)SendMessage(g_combo_req_proc, CB_GETCURSEL, 0, 0);
    if (pid < 0) {
        set_feedback("Select a process to record outstanding request for.", true);
        return;
    }

    SystemState *st = &g_detection_ctx->live_state;
    char req_str[128] = {0};
    GetWindowText(g_edit_wait_req, req_str, sizeof(req_str));

    int req[MAX_RESOURCES] = {0};
    char *token = strtok(req_str, ", ");
    int idx = 0;
    while (token != NULL && idx < st->resource_count) {
        req[idx++] = atoi(token);
        token = strtok(NULL, ", ");
    }

    ErrorCode err = detection_record_outstanding_request(st, pid, req);
    if (err == ERR_SUCCESS) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Recorded outstanding request for %s. Process status set to WAITING.", st->processes[pid].name);
        set_feedback(msg, false);
        on_run_detection();
    } else {
        set_feedback(error_code_to_string(err), true);
    }
}

static void on_clear_waiting_request(void) {
    if (!g_detection_ctx) return;
    int pid = (int)SendMessage(g_combo_req_proc, CB_GETCURSEL, 0, 0);
    if (pid < 0) {
        set_feedback("Select a process to clear request for.", true);
        return;
    }

    SystemState *st = &g_detection_ctx->live_state;
    ErrorCode err = detection_clear_outstanding_request(st, pid);
    if (err == ERR_SUCCESS) {
        set_feedback("Cleared outstanding requests for selected process.", false);
        on_run_detection();
    } else {
        set_feedback(error_code_to_string(err), true);
    }
}

static void on_suggest_victim(void) {
    if (!g_detection_ctx) return;
    SystemState *st = &g_detection_ctx->live_state;

    if (!g_last_detection_res.deadlock_detected) {
        set_feedback("No deadlock detected. Cannot select victim.", true);
        return;
    }

    int victim_id = -1;
    ErrorCode err = recovery_select_victim(st, &g_last_detection_res, &victim_id);
    if (err == ERR_SUCCESS && victim_id >= 0) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Heuristic selected victim: Process %s (holds most allocated resources).", st->processes[victim_id].name);
        set_feedback(msg, false);

        /* Select in combo */
        for (int k = 0; k < g_last_detection_res.deadlocked_count; k++) {
            if (g_last_detection_res.deadlocked_processes[k] == victim_id) {
                SendMessage(g_combo_victim_proc, CB_SETCURSEL, k, 0);
                break;
            }
        }
    } else {
        set_feedback("Failed to select recovery victim.", true);
    }
}

static void on_terminate_victim(void) {
    if (!g_detection_ctx) return;
    SystemState *st = &g_detection_ctx->live_state;

    int sel = (int)SendMessage(g_combo_victim_proc, CB_GETCURSEL, 0, 0);
    if (sel < 0 || sel >= g_last_detection_res.deadlocked_count) {
        set_feedback("Select a deadlocked victim process to terminate.", true);
        return;
    }

    int victim_pid = g_last_detection_res.deadlocked_processes[sel];

    /* Execute victim termination & resource reclamation */
    RecoveryResult rec_res;
    ErrorCode err = recovery_execute_victim_termination(st, victim_pid, &rec_res);
    if (err == ERR_SUCCESS) {
        g_last_detection_res = rec_res.post_recovery_detection;
        set_feedback(rec_res.description, false);
        on_run_detection();
    } else {
        set_feedback(error_code_to_string(err), true);
    }
}

void panel_detection_refresh(HWND panel, const SystemContext *ctx) {
    (void)panel;
    if (ctx) {
        g_detection_ctx = (SystemContext*)ctx;
    }
    update_selectors();
    if (g_detection_ctx && g_detection_ctx->live_state.process_count > 0) {
        on_run_detection();
    }
}

static LRESULT CALLBACK DetectionPanelProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            switch (wmId) {
                case IDC_BTN_RUN_DETECT:        on_run_detection(); break;
                case IDC_BTN_RECORD_REQ:        on_record_waiting_request(); break;
                case IDC_BTN_CLEAR_REQ:         on_clear_waiting_request(); break;
                case IDC_BTN_SUGGEST_VICTIM:    on_suggest_victim(); break;
                case IDC_BTN_TERMINATE_VICTIM:  on_terminate_victim(); break;
                default: break;
            }
            return 0;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rect;
            GetClientRect(hwnd, &rect);
            FillRect(hdc, &rect, (HBRUSH)(COLOR_BTNFACE + 1));
            EndPaint(hwnd, &ps);
            return 0;
        }

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

static void register_class_once(HINSTANCE hInstance) {
    static bool registered = false;
    if (!registered) {
        WNDCLASSEX wc;
        memset(&wc, 0, sizeof(wc));
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = DetectionPanelProc;
        wc.hInstance = hInstance;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
        wc.lpszClassName = PANEL_DETECTION_CLASS;
        RegisterClassEx(&wc);
        registered = true;
    }
}

HWND panel_detection_create(HWND parent, SystemContext *ctx) {
    g_detection_ctx = ctx;
    HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(parent, GWLP_HINSTANCE);
    register_class_once(hInstance);

    HWND panel = CreateWindowEx(
        0,
        PANEL_DETECTION_CLASS,
        NULL,
        WS_CHILD | WS_CLIPSIBLINGS,
        0, 0, 100, 100,
        parent,
        NULL,
        hInstance,
        NULL
    );
    g_hwnd_detect_panel = panel;

    /* Left Control Area (Width: 340) */

    /* Group 1: Deadlock Detection Controls */
    CreateWindow("BUTTON", "1. Deadlock Detection Action", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                 15, 10, 330, 140, panel, NULL, hInstance, NULL);

    CreateWindow("BUTTON", "Run Deadlock Detection Algorithm", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                 25, 32, 310, 28, panel, (HMENU)IDC_BTN_RUN_DETECT, hInstance, NULL);

    g_static_detect_status = CreateWindow(
        "STATIC",
        "STATUS: UNCHECKED",
        WS_CHILD | WS_VISIBLE | SS_CENTER | SS_SUNKEN,
        25, 68, 310, 24,
        panel,
        (HMENU)IDC_STATIC_DETECT_STATUS,
        hInstance,
        NULL
    );

    g_static_deadlock_procs = CreateWindow(
        "STATIC",
        "Deadlocked Processes: None",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        25, 98, 310, 42,
        panel,
        (HMENU)IDC_STATIC_DEADLOCK_PROCS,
        hInstance,
        NULL
    );

    /* Group 2: Outstanding Request Tracking */
    CreateWindow("BUTTON", "2. Outstanding (Waiting) Requests", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                 15, 160, 330, 175, panel, NULL, hInstance, NULL);

    CreateWindow("STATIC", "Process:", WS_CHILD | WS_VISIBLE, 25, 185, 70, 18, panel, NULL, hInstance, NULL);
    g_combo_req_proc = CreateWindow("COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
                                    95, 182, 240, 150, panel, (HMENU)IDC_COMBO_REQ_PROC, hInstance, NULL);

    CreateWindow("STATIC", "Waiting Request Vector (e.g. 1, 0, 1):", WS_CHILD | WS_VISIBLE, 25, 215, 290, 18, panel, NULL, hInstance, NULL);
    g_edit_wait_req = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "1, 0, 0", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                                     25, 235, 310, 22, panel, (HMENU)IDC_EDIT_WAIT_REQ, hInstance, NULL);

    CreateWindow("BUTTON", "Record Waiting Request", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                 25, 265, 150, 26, panel, (HMENU)IDC_BTN_RECORD_REQ, hInstance, NULL);
    CreateWindow("BUTTON", "Clear Request", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                 185, 265, 150, 26, panel, (HMENU)IDC_BTN_CLEAR_REQ, hInstance, NULL);

    CreateWindow("STATIC", "(Sets process to WAITING and models blocked state)", WS_CHILD | WS_VISIBLE,
                 25, 300, 300, 20, panel, NULL, hInstance, NULL);

    /* Group 3: Deadlock Recovery Demonstration */
    CreateWindow("BUTTON", "3. Deadlock Recovery (Victim Termination)", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                 15, 345, 330, 235, panel, NULL, hInstance, NULL);

    CreateWindow("STATIC", "Select Victim Process:", WS_CHILD | WS_VISIBLE, 25, 370, 200, 18, panel, NULL, hInstance, NULL);
    g_combo_victim_proc = CreateWindow("COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
                                       25, 390, 310, 150, panel, (HMENU)IDC_COMBO_VICTIM_PROC, hInstance, NULL);

    CreateWindow("BUTTON", "Auto-Select Victim (Heuristic)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                 25, 425, 310, 26, panel, (HMENU)IDC_BTN_SUGGEST_VICTIM, hInstance, NULL);

    CreateWindow("BUTTON", "Terminate Victim & Recover", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                 25, 460, 310, 30, panel, (HMENU)IDC_BTN_TERMINATE_VICTIM, hInstance, NULL);

    CreateWindow("STATIC",
        "Recovery Action:\r\n"
        " - Terminates selected deadlocked process.\r\n"
        " - Releases all held allocations to Available.\r\n"
        " - Clears outstanding request.\r\n"
        " - Re-runs detection to confirm deadlock broken.",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        25, 500, 310, 70, panel, NULL, hInstance, NULL);

    /* Right Column: Detailed Diagnostics & Matrix View */
    CreateWindow("STATIC", "Outstanding Request Matrix & Deadlock Diagnostics:",
                 WS_CHILD | WS_VISIBLE, 360, 10, 660, 20, panel, NULL, hInstance, NULL);

    HFONT hFont = CreateFont(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                             ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");

    g_edit_detect_display = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        "EDIT",
        "",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
        360, 30, 670, 480,
        panel,
        (HMENU)IDC_EDIT_DETECT_DISPLAY,
        hInstance,
        NULL
    );
    SendMessage(g_edit_detect_display, WM_SETFONT, (WPARAM)hFont, TRUE);

    /* Bottom: Real-Time Feedback */
    CreateWindow("BUTTON", "Operation Status & Feedback", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                 360, 515, 670, 65, panel, NULL, hInstance, NULL);

    g_static_feedback = CreateWindow(
        "STATIC",
        "[INFO] Person 3 Deadlock Detection & Recovery ready. Click 'Run Deadlock Detection Algorithm'.",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        375, 538, 640, 30,
        panel,
        (HMENU)IDC_STATIC_DETECT_FEEDBACK,
        hInstance,
        NULL
    );

    panel_detection_refresh(panel, ctx);

    return panel;
}
