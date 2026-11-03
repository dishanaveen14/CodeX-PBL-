/**
 * @file panel_bankers.c
 * @brief Person 2: Banker's Algorithm & Safe-State Analysis GUI Panel.
 * 
 * Deadlock Simulation System - PBL Project
 * Assigned to: Person 2 (Banker's Algorithm and Safe-State Analysis)
 */

#include "../../include/gui/panel_bankers.h"
#include "../../include/person2_bankers/bankers.h"
#include "../../include/person1_process_resource/matrix_mgmt.h"
#include "../../include/common/error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PANEL_BANKERS_CLASS "DeadlockSimPanelBankers"

/* Control IDs */
enum {
    IDC_BTN_RUN_SAFETY = 2001,
    IDC_BTN_EVAL_REQ,
    IDC_BTN_APPLY_REQ,
    IDC_COMBO_BANKER_PROC,
    IDC_EDIT_BANKER_REQ,
    IDC_STATIC_SAFETY_STATUS,
    IDC_STATIC_SAFE_SEQ,
    IDC_EDIT_TRACE_DISPLAY,
    IDC_STATIC_BANKER_FEEDBACK
};

static SystemContext *g_banker_ctx = NULL;
static HWND g_hwnd_banker_panel = NULL;
static HWND g_static_safety_status = NULL;
static HWND g_static_safe_seq = NULL;
static HWND g_combo_banker_proc = NULL;
static HWND g_edit_banker_req = NULL;
static HWND g_edit_trace = NULL;
static HWND g_static_feedback = NULL;

static BankerSafetyResult g_last_safety_res;
static BankerRequestResult g_last_request_res;
static bool g_has_evaluated_request = false;

static void set_feedback(const char *msg, bool is_error) {
    if (g_static_feedback) {
        char buf[512];
        snprintf(buf, sizeof(buf), "[%s] %s", is_error ? "ALERT" : "INFO", msg);
        SetWindowText(g_static_feedback, buf);
    }
}

static void update_process_selector(void) {
    if (!g_banker_ctx || !g_combo_banker_proc) return;

    SystemState *st = &g_banker_ctx->live_state;
    SendMessage(g_combo_banker_proc, CB_RESETCONTENT, 0, 0);

    for (int i = 0; i < st->process_count; i++) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%s (ID: %d)", st->processes[i].name, i);
        SendMessage(g_combo_banker_proc, CB_ADDSTRING, 0, (LPARAM)buf);
    }

    if (st->process_count > 0) {
        SendMessage(g_combo_banker_proc, CB_SETCURSEL, 0, 0);
    }
}

static void render_trace_text(const BankerSafetyResult *res) {
    if (!g_banker_ctx || !g_edit_trace) return;

    SystemState *st = &g_banker_ctx->live_state;
    char text[4096];
    int offset = 0;

    offset += snprintf(text + offset, sizeof(text) - offset,
        "========================================================================================\r\n"
        " BANKER'S SAFETY ALGORITHM EXECUTION TRACE & WORK VECTOR ANALYSIS\r\n"
        "========================================================================================\r\n\r\n");

    /* Initial Work Vector */
    offset += snprintf(text + offset, sizeof(text) - offset, "INITIAL WORK VECTOR (Copy of Available):\r\n  [ ");
    for (int j = 0; j < st->resource_count; j++) {
        offset += snprintf(text + offset, sizeof(text) - offset, "%s:%d ", st->resources[j].name, st->available[j]);
    }
    offset += snprintf(text + offset, sizeof(text) - offset, "]\r\n\r\n");

    /* Step-by-step trace */
    if (res && res->step_count > 0) {
        offset += snprintf(text + offset, sizeof(text) - offset, "STEP-BY-STEP PROCESS FINISH ORDER:\r\n");
        offset += snprintf(text + offset, sizeof(text) - offset,
            "----------------------------------------------------------------------------------------\r\n");

        for (int s = 0; s < res->step_count; s++) {
            const BankerWorkStep *step = &res->steps[s];
            int pid = step->process_id;
            const char *pname = (pid < st->process_count) ? st->processes[pid].name : "P?";

            /* Format Work Before */
            char wb[64] = "";
            for (int j = 0; j < st->resource_count; j++) {
                char temp[16];
                snprintf(temp, sizeof(temp), "%d ", step->work_before[j]);
                strcat(wb, temp);
            }

            /* Format Need */
            char nd[64] = "";
            for (int j = 0; j < st->resource_count; j++) {
                char temp[16];
                snprintf(temp, sizeof(temp), "%d ", st->need[pid][j]);
                strcat(nd, temp);
            }

            /* Format Reclaimed Alloc */
            char al[64] = "";
            for (int j = 0; j < st->resource_count; j++) {
                char temp[16];
                snprintf(temp, sizeof(temp), "%d ", st->allocation[pid][j]);
                strcat(al, temp);
            }

            /* Format Work After */
            char wa[64] = "";
            for (int j = 0; j < st->resource_count; j++) {
                char temp[16];
                snprintf(temp, sizeof(temp), "%d ", step->work_after[j]);
                strcat(wa, temp);
            }

            offset += snprintf(text + offset, sizeof(text) - offset,
                "  Step %d: Process %-4s | Need: [%s] <= Work: [%s]\r\n"
                "          Action: Finish[%s] = TRUE. Reclaiming Allocation [%s]\r\n"
                "          New Work Vector = [%s]\r\n\r\n",
                s + 1, pname, nd, wb, pname, al, wa);
        }
    } else {
        offset += snprintf(text + offset, sizeof(text) - offset,
            "  (No safety steps recorded. Click 'Run Safety Check' to execute algorithm.)\r\n\r\n");
    }

    /* Summary Conclusion */
    offset += snprintf(text + offset, sizeof(text) - offset,
        "========================================================================================\r\n"
        " CONCLUSION:\r\n  %s\r\n"
        "========================================================================================\r\n",
        (res && res->explanation[0]) ? res->explanation : "State not yet analyzed.");

    SetWindowText(g_edit_trace, text);
}

static void on_run_safety_check(void) {
    if (!g_banker_ctx) return;

    SystemState *st = &g_banker_ctx->live_state;
    if (st->process_count == 0) {
        set_feedback("Cannot run safety check: No processes configured. Load a preset in Setup tab.", true);
        return;
    }

    ErrorCode err = banker_run_safety_check(st, &g_last_safety_res);
    if (err == ERR_SUCCESS) {
        /* Update overall status */
        st->overall_status = g_last_safety_res.is_safe ? SYSTEM_STATUS_SAFE : SYSTEM_STATUS_UNSAFE;
        strncpy(st->status_message, g_last_safety_res.explanation, MAX_DESC_LEN - 1);

        /* Update UI Banner */
        if (g_last_safety_res.is_safe) {
            SetWindowText(g_static_safety_status, "STATUS: SAFE (Deadlock Avoidance Guaranteed)");

            char seq_buf[256];
            char formatted[128];
            banker_format_safe_sequence(st, &g_last_safety_res.safe_sequence, formatted, sizeof(formatted));
            snprintf(seq_buf, sizeof(seq_buf), "Safe Sequence: < %s >", formatted);
            SetWindowText(g_static_safe_seq, seq_buf);

            set_feedback("Banker's Algorithm completed: System is in a SAFE state.", false);
        } else {
            SetWindowText(g_static_safety_status, "STATUS: UNSAFE (Potential Deadlock Risk!)");

            char unfin_str[128] = "";
            for (int k = 0; k < g_last_safety_res.unfinished_count; k++) {
                int pid = g_last_safety_res.unfinished_processes[k];
                char temp[32];
                snprintf(temp, sizeof(temp), "%s%s", (k > 0) ? ", " : "", st->processes[pid].name);
                strcat(unfin_str, temp);
            }
            char seq_buf[256];
            snprintf(seq_buf, sizeof(seq_buf), "Safe Sequence: None (Unfinished: [%s])", unfin_str);
            SetWindowText(g_static_safe_seq, seq_buf);

            set_feedback("Banker's Algorithm completed: State is UNSAFE. No complete safe sequence exists.", true);
        }

        render_trace_text(&g_last_safety_res);
    } else {
        set_feedback(error_code_to_string(err), true);
    }
}

static void on_evaluate_request(void) {
    if (!g_banker_ctx) return;

    int pid = (int)SendMessage(g_combo_banker_proc, CB_GETCURSEL, 0, 0);
    if (pid < 0) {
        set_feedback("Select a process to evaluate request for.", true);
        return;
    }

    SystemState *st = &g_banker_ctx->live_state;
    if (st->resource_count == 0) {
        set_feedback("No resources configured in system.", true);
        return;
    }

    char req_str[128] = {0};
    GetWindowText(g_edit_banker_req, req_str, sizeof(req_str));

    int req[MAX_RESOURCES] = {0};
    char *token = strtok(req_str, ", ");
    int idx = 0;
    while (token != NULL && idx < st->resource_count) {
        req[idx++] = atoi(token);
        token = strtok(NULL, ", ");
    }

    /* Run Banker's Resource Request Algorithm */
    ErrorCode err = banker_evaluate_resource_request(st, pid, req, &g_last_request_res);
    g_has_evaluated_request = true;

    if (err == ERR_SUCCESS && g_last_request_res.is_granted) {
        set_feedback(g_last_request_res.reason, false);
        render_trace_text(&g_last_request_res.trial_result);
    } else {
        set_feedback(g_last_request_res.reason, true);
        if (g_last_request_res.trial_result.step_count > 0 || !g_last_request_res.trial_result.is_safe) {
            render_trace_text(&g_last_request_res.trial_result);
        }
    }
}

static void on_apply_request(void) {
    if (!g_banker_ctx || !g_has_evaluated_request) {
        set_feedback("Please evaluate the resource request safety first.", true);
        return;
    }

    if (!g_last_request_res.is_granted) {
        set_feedback("Cannot apply request: Banker's safety check denied this request.", true);
        return;
    }

    int pid = (int)SendMessage(g_combo_banker_proc, CB_GETCURSEL, 0, 0);
    if (pid < 0) return;

    SystemState *st = &g_banker_ctx->live_state;
    char req_str[128] = {0};
    GetWindowText(g_edit_banker_req, req_str, sizeof(req_str));

    int req[MAX_RESOURCES] = {0};
    char *token = strtok(req_str, ", ");
    int idx = 0;
    while (token != NULL && idx < st->resource_count) {
        req[idx++] = atoi(token);
        token = strtok(NULL, ", ");
    }

    ErrorCode err = sys_allocation_apply(st, pid, req);
    if (err == ERR_SUCCESS) {
        set_feedback("Request safely committed to live state! Re-running safety check on updated state...", false);
        g_has_evaluated_request = false;
        on_run_safety_check();
    } else {
        set_feedback(error_code_to_string(err), true);
    }
}

void panel_bankers_refresh(HWND panel, const SystemContext *ctx) {
    (void)panel;
    if (ctx) {
        g_banker_ctx = (SystemContext*)ctx;
    }
    update_process_selector();
    if (g_banker_ctx && g_banker_ctx->live_state.process_count > 0) {
        on_run_safety_check();
    }
}

static LRESULT CALLBACK BankersPanelProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            switch (wmId) {
                case IDC_BTN_RUN_SAFETY: on_run_safety_check(); break;
                case IDC_BTN_EVAL_REQ:   on_evaluate_request(); break;
                case IDC_BTN_APPLY_REQ:  on_apply_request(); break;
                default: break;
            }
            return 0;
        }

        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            SetBkMode(hdcStatic, OPAQUE);
            SetBkColor(hdcStatic, GetSysColor(COLOR_BTNFACE));
            SetTextColor(hdcStatic, GetSysColor(COLOR_WINDOWTEXT));
            return (LRESULT)GetSysColorBrush(COLOR_BTNFACE);
        }

        case WM_CTLCOLORBTN: {
            HDC hdcBtn = (HDC)wParam;
            SetBkMode(hdcBtn, OPAQUE);
            SetBkColor(hdcBtn, GetSysColor(COLOR_BTNFACE));
            return (LRESULT)GetSysColorBrush(COLOR_BTNFACE);
        }

        case WM_ERASEBKGND: {
            HDC hdc = (HDC)wParam;
            RECT rc;
            GetClientRect(hwnd, &rc);
            FillRect(hdc, &rc, GetSysColorBrush(COLOR_BTNFACE));
            return 1;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rect;
            GetClientRect(hwnd, &rect);
            FillRect(hdc, &rect, GetSysColorBrush(COLOR_BTNFACE));
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
        wc.lpfnWndProc = BankersPanelProc;
        wc.hInstance = hInstance;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
        wc.lpszClassName = PANEL_BANKERS_CLASS;
        RegisterClassEx(&wc);
        registered = true;
    }
}

HWND panel_bankers_create(HWND parent, SystemContext *ctx) {
    g_banker_ctx = ctx;
    HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(parent, GWLP_HINSTANCE);
    register_class_once(hInstance);

    HWND panel = CreateWindowEx(
        0,
        PANEL_BANKERS_CLASS,
        NULL,
        WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        0, 0, 100, 100,
        parent,
        NULL,
        hInstance,
        NULL
    );
    g_hwnd_banker_panel = panel;

    /* Left Control Area (Width: 340) */

    /* Group 1: Safety Check Execution */
    CreateWindow("BUTTON", "1. Deadlock Avoidance (Safety Algorithm)", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                 15, 10, 330, 140, panel, NULL, hInstance, NULL);

    CreateWindow("BUTTON", "Run Banker's Safety Check", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                 25, 32, 310, 28, panel, (HMENU)IDC_BTN_RUN_SAFETY, hInstance, NULL);

    g_static_safety_status = CreateWindow(
        "STATIC",
        "STATUS: UNCHECKED",
        WS_CHILD | WS_VISIBLE | SS_CENTER | SS_SUNKEN,
        25, 68, 310, 24,
        panel,
        (HMENU)IDC_STATIC_SAFETY_STATUS,
        hInstance,
        NULL
    );

    g_static_safe_seq = CreateWindow(
        "STATIC",
        "Safe Sequence: (Click Run to evaluate)",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        25, 98, 310, 42,
        panel,
        (HMENU)IDC_STATIC_SAFE_SEQ,
        hInstance,
        NULL
    );

    /* Group 2: Banker's Resource Request Evaluator */
    CreateWindow("BUTTON", "2. Resource-Request Safety Analysis", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                 15, 160, 330, 175, panel, NULL, hInstance, NULL);

    CreateWindow("STATIC", "Requesting Process:", WS_CHILD | WS_VISIBLE, 25, 185, 130, 18, panel, NULL, hInstance, NULL);
    g_combo_banker_proc = CreateWindow("COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
                                      160, 182, 175, 150, panel, (HMENU)IDC_COMBO_BANKER_PROC, hInstance, NULL);

    CreateWindow("STATIC", "Request Vector (e.g. 1, 0, 2):", WS_CHILD | WS_VISIBLE, 25, 215, 200, 18, panel, NULL, hInstance, NULL);
    g_edit_banker_req = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "1, 0, 2", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                                       25, 235, 310, 22, panel, (HMENU)IDC_EDIT_BANKER_REQ, hInstance, NULL);

    CreateWindow("BUTTON", "Evaluate Request (Banker's)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                 25, 265, 310, 28, panel, (HMENU)IDC_BTN_EVAL_REQ, hInstance, NULL);

    CreateWindow("BUTTON", "Apply && Commit if Safe", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                 25, 298, 310, 26, panel, (HMENU)IDC_BTN_APPLY_REQ, hInstance, NULL);

    /* Group 3: Instructions & Invariants */
    CreateWindow("BUTTON", "3. Banker's Algorithm Invariants", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                 15, 345, 330, 235, panel, NULL, hInstance, NULL);

    CreateWindow("STATIC",
        "Operating Principles:\r\n"
        " 1. Work = Available vector copy.\r\n"
        " 2. Finish[i] = FALSE for all active processes.\r\n"
        " 3. Finds unfinished P_i with Need[i] <= Work.\r\n"
        " 4. Work = Work + Allocation[i]; Finish[i] = TRUE.\r\n"
        " 5. Repeats until all finish (SAFE) or none can proceed (UNSAFE).\r\n\r\n"
        "Request Evaluation:\r\n"
        " - Check Req <= Need (or reject immediately).\r\n"
        " - Check Req <= Available (or wait).\r\n"
        " - Construct trial state & evaluate safety.\r\n"
        " - Never mutates live state on unsafe requests.",
        WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOPREFIX,
        25, 370, 310, 200, panel, NULL, hInstance, NULL);

    /* Right Column: Educational Step-by-Step Trace & Work Vector View */
    CreateWindow("STATIC", "Educational Step-by-Step Execution Trace (Work Vector Updates):",
                 WS_CHILD | WS_VISIBLE | SS_NOPREFIX, 360, 10, 660, 20, panel, NULL, hInstance, NULL);

    HFONT hFont = CreateFont(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                             ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");

    g_edit_trace = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        "EDIT",
        "",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
        360, 30, 670, 480,
        panel,
        (HMENU)IDC_EDIT_TRACE_DISPLAY,
        hInstance,
        NULL
    );
    SendMessage(g_edit_trace, WM_SETFONT, (WPARAM)hFont, TRUE);

    /* Bottom: Real-Time Feedback */
    CreateWindow("BUTTON", "Safety Feedback & Operation Messages", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                 360, 515, 670, 65, panel, NULL, hInstance, NULL);

    g_static_feedback = CreateWindow(
        "STATIC",
        "[INFO] Person 2 Banker's Algorithm ready. Click 'Run Banker's Safety Check' to evaluate state.",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        375, 538, 640, 30,
        panel,
        (HMENU)IDC_STATIC_BANKER_FEEDBACK,
        hInstance,
        NULL
    );

    panel_bankers_refresh(panel, ctx);

    return panel;
}
