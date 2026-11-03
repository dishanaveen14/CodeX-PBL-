/**
 * @file panel_dashboard.c
 * @brief Dashboard Overview Panel Implementation (Live Status, Metrics & Coordination).
 * 
 * Deadlock Simulation System - PBL Project
 * Person 4: Architecture, GUI Coordination, and Integration.
 */

#include "../../include/gui/panel_dashboard.h"
#include "../../include/common/error.h"
#include "../../include/person1_process_resource/matrix_mgmt.h"
#include "../../include/person2_bankers/bankers.h"
#include "../../include/person3_detection_recovery/detection.h"
#include "../../include/person3_detection_recovery/rag.h"
#include <stdio.h>
#include <string.h>

#define PANEL_DASHBOARD_CLASS "DeadlockSimPanelDashboard"

enum {
    IDC_BTN_DASH_RUN_ANALYSIS = 1001,
    IDC_BTN_DASH_LOAD_SAFE,
    IDC_BTN_DASH_LOAD_DEADLOCK,
    IDC_BTN_DASH_RESET,
    IDC_STATIC_DASH_STATUS,
    IDC_STATIC_DASH_METRICS,
    IDC_STATIC_DASH_SUBSYSTEMS,
    IDC_STATIC_DASH_FEEDBACK
};

static SystemContext *g_dash_ctx = NULL;
static HWND g_hwnd_dash_panel = NULL;
static HWND g_hwnd_status_static = NULL;
static HWND g_hwnd_metrics_static = NULL;
static HWND g_hwnd_subsystems_static = NULL;
static HWND g_hwnd_feedback_static = NULL;

static void set_dashboard_feedback(const char *msg) {
    if (g_hwnd_feedback_static && msg) {
        char buf[256];
        snprintf(buf, sizeof(buf), "Coordinator Status: %s", msg);
        SetWindowText(g_hwnd_feedback_static, buf);
    }
}

static LRESULT CALLBACK DashboardPanelProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
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

        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            if (!g_dash_ctx) return 0;

            switch (wmId) {
                case IDC_BTN_DASH_RUN_ANALYSIS: {
                    coordinator_run_full_analysis(g_dash_ctx);
                    panel_dashboard_refresh(hwnd, g_dash_ctx);
                    set_dashboard_feedback("Full system analysis executed. All 4 subsystems synchronized.");
                    break;
                }
                case IDC_BTN_DASH_LOAD_SAFE: {
                    sys_load_sample_configuration(&g_dash_ctx->live_state, 0);
                    coordinator_run_full_analysis(g_dash_ctx);
                    panel_dashboard_refresh(hwnd, g_dash_ctx);
                    set_dashboard_feedback("Loaded Preset 0 (Silberschatz Safe 5P/3R). Analysis updated.");
                    break;
                }
                case IDC_BTN_DASH_LOAD_DEADLOCK: {
                    sys_load_sample_configuration(&g_dash_ctx->live_state, 1);
                    coordinator_run_full_analysis(g_dash_ctx);
                    panel_dashboard_refresh(hwnd, g_dash_ctx);
                    set_dashboard_feedback("Loaded Preset 1 (Circular Wait Deadlock 3P/3R). Analysis updated.");
                    break;
                }
                case IDC_BTN_DASH_RESET: {
                    coordinator_init(g_dash_ctx);
                    panel_dashboard_refresh(hwnd, g_dash_ctx);
                    set_dashboard_feedback("System state cleared and reset to initial state.");
                    break;
                }
                default:
                    break;
            }
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
        wc.lpfnWndProc = DashboardPanelProc;
        wc.hInstance = hInstance;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
        wc.lpszClassName = PANEL_DASHBOARD_CLASS;
        RegisterClassEx(&wc);
        registered = true;
    }
}

HWND panel_dashboard_create(HWND parent, SystemContext *ctx) {
    HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(parent, GWLP_HINSTANCE);
    register_class_once(hInstance);
    g_dash_ctx = ctx;

    HWND panel = CreateWindowEx(
        0,
        PANEL_DASHBOARD_CLASS,
        NULL,
        WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        0, 0, 100, 100,
        parent,
        NULL,
        hInstance,
        NULL
    );
    g_hwnd_dash_panel = panel;

    /* Title Label */
    CreateWindow(
        "STATIC",
        "System Overview && Integration Dashboard (Person 4 Coordinator)",
        WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOPREFIX,
        20, 15, 700, 22,
        panel, NULL, hInstance, NULL
    );

    /* Description */
    CreateWindow(
        "STATIC",
        "Central coordinator orchestrating Person 1 (State && Matrices), Person 2 (Banker's Algorithm), "
        "and Person 3 (Deadlock Detection && RAG).",
        WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOPREFIX,
        20, 38, 960, 20,
        panel, NULL, hInstance, NULL
    );

    /* Action Buttons Bar */
    CreateWindow(
        "BUTTON",
        "Run Full System Analysis",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        20, 65, 200, 30,
        panel, (HMENU)(INT_PTR)IDC_BTN_DASH_RUN_ANALYSIS, hInstance, NULL
    );

    CreateWindow(
        "BUTTON",
        "Load Preset 0 (Safe 5P/3R)",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        230, 65, 200, 30,
        panel, (HMENU)(INT_PTR)IDC_BTN_DASH_LOAD_SAFE, hInstance, NULL
    );

    CreateWindow(
        "BUTTON",
        "Load Preset 1 (Deadlock 3P/3R)",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        440, 65, 220, 30,
        panel, (HMENU)(INT_PTR)IDC_BTN_DASH_LOAD_DEADLOCK, hInstance, NULL
    );

    CreateWindow(
        "BUTTON",
        "Reset System",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        670, 65, 130, 30,
        panel, (HMENU)(INT_PTR)IDC_BTN_DASH_RESET, hInstance, NULL
    );

    /* Group Box 1: Overall System Health */
    CreateWindow(
        "BUTTON",
        "Overall System State Status",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        20, 105, 480, 175,
        panel, NULL, hInstance, NULL
    );

    g_hwnd_status_static = CreateWindow(
        "STATIC",
        "Status: UNCHECKED\n\nClick 'Run Full System Analysis' or select a preset to analyze.",
        WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOPREFIX,
        35, 128, 450, 142,
        panel, (HMENU)(INT_PTR)IDC_STATIC_DASH_STATUS, hInstance, NULL
    );

    /* Group Box 2: Subsystem Integration Summary */
    CreateWindow(
        "BUTTON",
        "Subsystem Roles && Module Status",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        520, 105, 480, 175,
        panel, NULL, hInstance, NULL
    );

    g_hwnd_subsystems_static = CreateWindow(
        "STATIC",
        "Person 1: Process && Resource Mgmt, Allocation && Need Matrices [ONLINE]\n\n"
        "Person 2: Banker's Algorithm, Safety Analysis, Safe Sequences [ONLINE]\n\n"
        "Person 3: Deadlock Detection, Request Tracker, RAG Visualizer, Recovery [ONLINE]\n\n"
        "Person 4: Coordinator, Multi-module Workflow, Integrated Validation [ONLINE]",
        WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOPREFIX,
        535, 128, 450, 142,
        panel, (HMENU)(INT_PTR)IDC_STATIC_DASH_SUBSYSTEMS, hInstance, NULL
    );

    /* Group Box 3: Live Model Dimensions & Metrics */
    CreateWindow(
        "BUTTON",
        "Integrated System Metrics (Live State + Algorithm Findings)",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        20, 290, 980, 185,
        panel, NULL, hInstance, NULL
    );

    g_hwnd_metrics_static = CreateWindow(
        "STATIC",
        "Active Processes: 0\n"
        "Configured Resource Types: 0\n",
        WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOPREFIX,
        35, 315, 950, 150,
        panel, (HMENU)(INT_PTR)IDC_STATIC_DASH_METRICS, hInstance, NULL
    );

    /* Feedback bar */
    g_hwnd_feedback_static = CreateWindow(
        "STATIC",
        "Coordinator Status: Ready. All four team modules loaded and operational.",
        WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOPREFIX,
        20, 485, 980, 22,
        panel, (HMENU)(INT_PTR)IDC_STATIC_DASH_FEEDBACK, hInstance, NULL
    );

    panel_dashboard_refresh(panel, ctx);

    return panel;
}

void panel_dashboard_refresh(HWND panel, const SystemContext *ctx) {
    (void)panel;
    if (!ctx) return;

    const SystemState *st = &ctx->live_state;

    if (g_hwnd_status_static) {
        char status_buf[512];
        const char *badge = "UNCHECKED";
        if (st->overall_status == SYSTEM_STATUS_SAFE) {
            badge = "SAFE (No Deadlock Risk)";
        } else if (st->overall_status == SYSTEM_STATUS_UNSAFE) {
            badge = "UNSAFE (Deadlock Risk Possible)";
        } else if (st->overall_status == SYSTEM_STATUS_DEADLOCKED) {
            badge = "DEADLOCKED (Active Deadlock Detected)";
        }

        snprintf(status_buf, sizeof(status_buf),
            "System Classification: %s\n\n"
            "Status Detail:\n%s\n\n"
            "Safe Sequence: %s\n"
            "Deadlocked Processes: %d",
            badge,
            st->status_message[0] ? st->status_message : "No state changes evaluated yet.",
            ctx->latest_safety_result.is_safe ? "Computed (Safe Execution Path Available)" : "None Available",
            ctx->latest_deadlock_result.deadlocked_count);
        SetWindowText(g_hwnd_status_static, status_buf);
        InvalidateRect(g_hwnd_status_static, NULL, TRUE);
    }

    if (g_hwnd_metrics_static) {
        char metrics_buf[1024];
        char res_avail[256] = "";
        char res_total[256] = "";

        for (int j = 0; j < st->resource_count; j++) {
            char temp[48];
            snprintf(temp, sizeof(temp), "%s:%d ", st->resources[j].name, st->available[j]);
            strcat(res_avail, temp);
            snprintf(temp, sizeof(temp), "%s:%d ", st->resources[j].name, st->total[j]);
            strcat(res_total, temp);
        }

        /* Count outstanding requests */
        int waiting_processes = 0;
        for (int i = 0; i < st->process_count; i++) {
            bool has_req = false;
            for (int j = 0; j < st->resource_count; j++) {
                if (st->outstanding_requests[i][j] > 0) {
                    has_req = true;
                    break;
                }
            }
            if (has_req) waiting_processes++;
        }

        snprintf(metrics_buf, sizeof(metrics_buf),
            "[Person 1: State] Processes: %d / %d | Resource Types: %d / %d\n"
            "  Total Resources:     %s\n"
            "  Available Resources: %s\n\n"
            "[Person 2: Banker's Algorithm] Safety Check: %s | Evaluation Steps: %d\n\n"
            "[Person 3: Deadlock && RAG] Deadlock Detected: %s | Waiting Processes: %d | RAG Nodes: %d | RAG Edges: %d\n\n"
            "[Person 4: Integration] All modules connected via central Coordinator workflow.",
            st->process_count, MAX_PROCESSES,
            st->resource_count, MAX_RESOURCES,
            st->resource_count > 0 ? res_total : "None",
            st->resource_count > 0 ? res_avail : "None",
            ctx->latest_safety_result.is_safe ? "SAFE" : "NOT SAFE / UNCHECKED",
            ctx->latest_safety_result.step_count,
            ctx->latest_deadlock_result.deadlock_detected ? "YES (DEADLOCK)" : "NO (NO DEADLOCK)",
            waiting_processes,
            ctx->latest_rag.node_count,
            ctx->latest_rag.edge_count);

        SetWindowText(g_hwnd_metrics_static, metrics_buf);
        InvalidateRect(g_hwnd_metrics_static, NULL, TRUE);
    }
}
