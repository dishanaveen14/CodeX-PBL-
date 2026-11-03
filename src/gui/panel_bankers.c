/**
 * @file panel_bankers.c
 * @brief Person 2: Banker's Algorithm GUI Panel Placeholder.
 * 
 * Deadlock Simulation System - PBL Project
 */

#include "../../include/gui/panel_bankers.h"
#include <stdio.h>

#define PANEL_BANKERS_CLASS "DeadlockSimPanelBankers"

static LRESULT CALLBACK BankersPanelProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
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
    (void)ctx;
    HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(parent, GWLP_HINSTANCE);
    register_class_once(hInstance);

    HWND panel = CreateWindowEx(
        0,
        PANEL_BANKERS_CLASS,
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
        "Banker's Algorithm & Safe-State Analysis (Person 2 Responsibility)",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 20, 650, 24,
        panel, NULL, hInstance, NULL
    );

    CreateWindow(
        "STATIC",
        "Placeholder screen prepared for Person 2 to implement safety checks and sequence analysis.\n"
        "Interfaces defined in 'include/person2_bankers/bankers.h'.",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 50, 750, 40,
        panel, NULL, hInstance, NULL
    );

    /* Group Box 1: Run Safety Check */
    CreateWindow(
        "BUTTON",
        "1. Safety Algorithm Execution (UI Placeholder)",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        20, 100, 480, 200,
        panel, NULL, hInstance, NULL
    );

    CreateWindow(
        "STATIC",
        "Planned UI Controls for Person 2:\n\n"
        "  [Button] Run Safety Check (Banker's Algorithm)\n"
        "  [*] Status Display: SAFE / UNSAFE\n"
        "  [*] Work Vector & Finish Vector Initialization\n"
        "  [*] Rollback assurance (ensures check does NOT alter live state)",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        35, 130, 450, 150,
        panel, NULL, hInstance, NULL
    );

    /* Group Box 2: Safe Sequence Display */
    CreateWindow(
        "BUTTON",
        "2. Safe Sequence Analysis (UI Placeholder)",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        520, 100, 480, 200,
        panel, NULL, hInstance, NULL
    );

    CreateWindow(
        "STATIC",
        "Planned UI Controls for Person 2:\n\n"
        "  [*] Safe Sequence Display (e.g. < P1 -> P3 -> P0 -> P2 >)\n"
        "  [*] Unfinished Processes Display (if UNSAFE)\n"
        "  [*] Denial Reason Explanation (e.g. Insufficient available / Exceeds Need)",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        535, 130, 450, 150,
        panel, NULL, hInstance, NULL
    );

    /* Group Box 3: Step-by-Step Educational Explanation */
    CreateWindow(
        "BUTTON",
        "3. Step-by-Step Educational Trace (UI Placeholder)",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        20, 320, 980, 220,
        panel, NULL, hInstance, NULL
    );

    CreateWindow(
        "STATIC",
        "Planned Educational Trace View for Person 2:\n\n"
        "  [Step List] Displays iterations of Work vector updates:\n"
        "    - Step i: Process Px selected because Need[x] <= Work\n"
        "    - Step i: Reclaiming Allocation[x] into Work\n"
        "    - Step i: Process Px marked Finish[x] = TRUE\n\n"
        "[Ready for Person 2 implementation]",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        35, 350, 940, 170,
        panel, NULL, hInstance, NULL
    );

    return panel;
}

void panel_bankers_refresh(HWND panel, const SystemContext *ctx) {
    (void)panel;
    (void)ctx;
}

