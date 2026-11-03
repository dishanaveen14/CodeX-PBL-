/**
 * @file panel_dashboard.c
 * @brief Dashboard Overview Panel Implementation (Live Status & Metrics).
 * 
 * Deadlock Simulation System - PBL Project
 */

#include "../../include/gui/panel_dashboard.h"
#include "../../include/common/error.h"
#include <stdio.h>

#define PANEL_DASHBOARD_CLASS "DeadlockSimPanelDashboard"

static HWND g_hwnd_metrics_static = NULL;
static HWND g_hwnd_status_static = NULL;

static LRESULT CALLBACK DashboardPanelProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
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

    HWND panel = CreateWindowEx(
        0,
        PANEL_DASHBOARD_CLASS,
        NULL,
        WS_CHILD | WS_CLIPSIBLINGS,
        0, 0, 100, 100,
        parent,
        NULL,
        hInstance,
        NULL
    );

    /* Title Label */
    CreateWindow(
        "STATIC",
        "System Overview & Integration Dashboard (Person 4 Coordinator)",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 20, 700, 24,
        panel, NULL, hInstance, NULL
    );

    /* Placeholder explanation */
    CreateWindow(
        "STATIC",
        "Central Dashboard integrating Person 1 (State & Matrices), Person 2 (Banker's Algorithm),\n"
        "and Person 3 (Deadlock Detection & RAG).",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 50, 750, 40,
        panel, NULL, hInstance, NULL
    );

    /* Group Box 1: Overall System Health */
    CreateWindow(
        "BUTTON",
        "System State Status",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        20, 100, 480, 160,
        panel, NULL, hInstance, NULL
    );

    g_hwnd_status_static = CreateWindow(
        "STATIC",
        "Current System Status: UNCHECKED\n\n"
        "Classification States:\n"
        "  - SAFE: Verified safe sequence exists (Banker's Algorithm)\n"
        "  - UNSAFE: No guaranteed safe sequence, deadlock risk\n"
        "  - DEADLOCKED: Active circular wait / ungrantable outstanding requests",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        35, 125, 450, 120,
        panel, NULL, hInstance, NULL
    );

    /* Group Box 2: Subsystem Integration Summary */
    CreateWindow(
        "BUTTON",
        "Subsystem Roles & Assignments",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        520, 100, 480, 160,
        panel, NULL, hInstance, NULL
    );

    CreateWindow(
        "STATIC",
        "Member Assignments:\n"
        "  - Person 1: Process & Resource Management, Allocation/Need Matrices [ACTIVE]\n"
        "  - Person 2: Banker's Algorithm, Safety Check, Safe Sequence [ACTIVE]\n"
        "  - Person 3: Deadlock Detection, Recovery, Resource Allocation Graph [ACTIVE]\n"
        "  - Person 4: System Architecture, State Contracts, GUI & Integration",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        535, 125, 450, 120,
        panel, NULL, hInstance, NULL
    );

    /* Group Box 3: Live Model Dimensions */
    CreateWindow(
        "BUTTON",
        "Live System State Metrics (Person 1)",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        20, 280, 980, 180,
        panel, NULL, hInstance, NULL
    );

    g_hwnd_metrics_static = CreateWindow(
        "STATIC",
        "Active Processes: 0 / 16\n"
        "Configured Resource Types: 0 / 16\n"
        "Outstanding Requests: None\n"
        "Deadlocked Processes: None\n\n"
        "[Person 1 implementation active. Load presets or create processes in 'Setup & Matrices' tab.]",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        35, 310, 940, 130,
        panel, NULL, hInstance, NULL
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
        snprintf(status_buf, sizeof(status_buf),
            "Current System Status: %s\n"
            "Status Detail: %s\n\n"
            "Classification States:\n"
            "  - SAFE: Verified safe sequence exists (Banker's Algorithm)\n"
            "  - UNSAFE: No guaranteed safe sequence, deadlock risk\n"
            "  - DEADLOCKED: Active circular wait / ungrantable outstanding requests",
            safety_status_to_string(st->overall_status),
            st->status_message[0] ? st->status_message : "No state changes evaluated yet.");
        SetWindowText(g_hwnd_status_static, status_buf);
    }

    if (g_hwnd_metrics_static) {
        char metrics_buf[1024];
        char res_list[512] = "";
        for (int j = 0; j < st->resource_count; j++) {
            char temp[64];
            snprintf(temp, sizeof(temp), "%s:%d ", st->resources[j].name, st->total[j]);
            strcat(res_list, temp);
        }

        snprintf(metrics_buf, sizeof(metrics_buf),
            "Active Processes: %d / %d\n"
            "Configured Resource Types: %d / %d  [%s]\n"
            "Available Vector: %s\n"
            "Total In-Flight Allocations: Active\n\n"
            "[Person 1 implementation active. Switch to 'Setup & Matrices (P1)' tab to edit state or load presets.]",
            st->process_count, MAX_PROCESSES,
            st->resource_count, MAX_RESOURCES,
            (st->resource_count > 0) ? res_list : "None",
            (st->resource_count > 0) ? "Updated" : "None");
        SetWindowText(g_hwnd_metrics_static, metrics_buf);
    }
}
