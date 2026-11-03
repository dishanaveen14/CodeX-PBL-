/**
 * @file panel_dashboard.c
 * @brief Dashboard Overview Panel Implementation (GUI Shell / Placeholders).
 * 
 * Deadlock Simulation System - PBL Project
 */

#include "../../include/gui/panel_dashboard.h"
#include "../../include/common/error.h"
#include <stdio.h>

#define PANEL_DASHBOARD_CLASS "DeadlockSimPanelDashboard"

static LRESULT CALLBACK DashboardPanelProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rect;
            GetClientRect(hwnd, &rect);
            FillRect(hdc, &rect, (HBRUSH)(COLOR_WINDOW + 1));
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
    (void)ctx;
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
        "Architecture Skeleton: Shared state and integration contracts established.\n"
        "This dashboard integrates all active subsystems and reflects live simulation state.",
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

    CreateWindow(
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
        "  - Person 1: Process & Resource Management, Allocation/Need Matrices\n"
        "  - Person 2: Banker's Algorithm, Safety Check, Safe Sequence\n"
        "  - Person 3: Deadlock Detection, Recovery, Resource Allocation Graph\n"
        "  - Person 4: System Architecture, State Contracts, GUI & Integration",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        535, 125, 450, 120,
        panel, NULL, hInstance, NULL
    );

    /* Group Box 3: Live Model Dimensions Placeholder */
    CreateWindow(
        "BUTTON",
        "Live Model Metrics (Placeholder)",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        20, 280, 980, 180,
        panel, NULL, hInstance, NULL
    );

    CreateWindow(
        "STATIC",
        "Active Processes: 0 / 16\n"
        "Configured Resource Types: 0 / 16\n"
        "Outstanding Requests: None\n"
        "Deadlocked Processes: None\n\n"
        "[Ready for independent module integration from Person 1, Person 2, and Person 3]",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        35, 310, 940, 130,
        panel, NULL, hInstance, NULL
    );

    return panel;
}

void panel_dashboard_refresh(HWND panel, const SystemContext *ctx) {
    (void)panel;
    (void)ctx;
    /* Refresh UI controls when data updates */
}

