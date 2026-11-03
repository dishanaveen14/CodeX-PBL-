/**
 * @file panel_detection.c
 * @brief Person 3: Deadlock Detection & Recovery GUI Panel Placeholder.
 * 
 * Deadlock Simulation System - PBL Project
 */

#include "../../include/gui/panel_detection.h"
#include <stdio.h>

#define PANEL_DETECTION_CLASS "DeadlockSimPanelDetection"

static LRESULT CALLBACK DetectionPanelProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
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
    (void)ctx;
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

    /* Header */
    CreateWindow(
        "STATIC",
        "Deadlock Detection & Recovery (Person 3 Responsibility)",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 20, 650, 24,
        panel, NULL, hInstance, NULL
    );

    CreateWindow(
        "STATIC",
        "Placeholder screen prepared for Person 3 to implement detection, waiting requests, and recovery.\n"
        "Interfaces defined in 'include/person3_detection_recovery/'.",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 50, 750, 40,
        panel, NULL, hInstance, NULL
    );

    /* Group Box 1: Deadlock Detection Controls */
    CreateWindow(
        "BUTTON",
        "1. Deadlock Detection (UI Placeholder)",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        20, 100, 480, 200,
        panel, NULL, hInstance, NULL
    );

    CreateWindow(
        "STATIC",
        "Planned UI Controls for Person 3:\n\n"
        "  [Button] Run Deadlock Detection\n"
        "  [*] Detection Algorithm Status: DETECTED / NO DEADLOCK\n"
        "  [*] Deadlocked Processes List (e.g. { P1, P2 })\n"
        "  [*] Contested Resource Types Display\n"
        "  [*] (Uses Outstanding Requests Matrix, NOT Need Matrix)",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        35, 130, 450, 150,
        panel, NULL, hInstance, NULL
    );

    /* Group Box 2: Outstanding Requests Table */
    CreateWindow(
        "BUTTON",
        "2. Outstanding Requests Tracker (UI Placeholder)",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        520, 100, 480, 200,
        panel, NULL, hInstance, NULL
    );

    CreateWindow(
        "STATIC",
        "Planned UI Controls for Person 3:\n\n"
        "  [*] Outstanding Request Matrix View\n"
        "  [*] Waiting Process Indicator\n"
        "  [Button] Clear / Resolve Waiting Request",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        535, 130, 450, 150,
        panel, NULL, hInstance, NULL
    );

    /* Group Box 3: Deadlock Recovery Workflow */
    CreateWindow(
        "BUTTON",
        "3. Deadlock Recovery Demonstration (UI Placeholder)",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        20, 320, 980, 220,
        panel, NULL, hInstance, NULL
    );

    CreateWindow(
        "STATIC",
        "Planned UI Controls for Person 3:\n\n"
        "  [*] Victim Selection Dropdown (populated from deadlocked processes list)\n"
        "  [Button] Suggest Victim Heuristic\n"
        "  [Button] Terminate Victim Process & Reclaim Resources (with confirmation)\n"
        "  [*] Before-and-After Resource Reclamation Diff Display\n"
        "  [*] Automatic Post-Recovery Detection Re-evaluation\n\n"
        "[Ready for Person 3 implementation]",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        35, 350, 940, 170,
        panel, NULL, hInstance, NULL
    );

    return panel;
}

void panel_detection_refresh(HWND panel, const SystemContext *ctx) {
    (void)panel;
    (void)ctx;
}

