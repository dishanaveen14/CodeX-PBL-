/**
 * @file panel_setup.c
 * @brief Person 1: Process and Resource Setup GUI Panel Placeholder.
 * 
 * Deadlock Simulation System - PBL Project
 */

#include "../../include/gui/panel_setup.h"
#include <stdio.h>

#define PANEL_SETUP_CLASS "DeadlockSimPanelSetup"

static LRESULT CALLBACK SetupPanelProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
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
        wc.lpfnWndProc = SetupPanelProc;
        wc.hInstance = hInstance;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
        wc.lpszClassName = PANEL_SETUP_CLASS;
        RegisterClassEx(&wc);
        registered = true;
    }
}

HWND panel_setup_create(HWND parent, SystemContext *ctx) {
    (void)ctx;
    HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(parent, GWLP_HINSTANCE);
    register_class_once(hInstance);

    HWND panel = CreateWindowEx(
        0,
        PANEL_SETUP_CLASS,
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
        "Process & Resource Setup (Person 1 Responsibility)",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 20, 600, 24,
        panel, NULL, hInstance, NULL
    );

    CreateWindow(
        "STATIC",
        "Placeholder screen prepared for Person 1 to implement creation forms and matrix tables.\n"
        "Interfaces defined in 'include/person1_process_resource/'.",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 50, 750, 40,
        panel, NULL, hInstance, NULL
    );

    /* Section A: Process & Resource Creation Placeholders */
    CreateWindow(
        "BUTTON",
        "1. Process & Resource Creation (UI Placeholder)",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        20, 100, 480, 180,
        panel, NULL, hInstance, NULL
    );

    CreateWindow(
        "STATIC",
        "Planned UI Controls for Person 1:\n\n"
        "  [+] Create Process Form (Input Name, Assign ID P0..Pn)\n"
        "  [-] Remove Process Button\n"
        "  [+] Create Resource Type Form (Input Name, Total Instances)\n"
        "  [*] Process & Resource Lists Table View",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        35, 130, 450, 130,
        panel, NULL, hInstance, NULL
    );

    /* Section B: Resource Request Form Placeholder */
    CreateWindow(
        "BUTTON",
        "2. Resource Request Interface (UI Placeholder)",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        520, 100, 480, 180,
        panel, NULL, hInstance, NULL
    );

    CreateWindow(
        "STATIC",
        "Planned UI Controls for Person 1:\n\n"
        "  [*] Select Process Dropdown (P0, P1, ...)\n"
        "  [*] Request Vector Inputs (instances for each resource)\n"
        "  [Button] Submit Request (passes to Person 2 Safety Check)\n"
        "  [*] Preliminary Validation Status (Req <= Need, Req <= Available)",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        535, 130, 450, 130,
        panel, NULL, hInstance, NULL
    );

    /* Section C: Matrix Viewers & Editors Placeholders */
    CreateWindow(
        "BUTTON",
        "3. System Matrices & Vectors (UI Placeholder)",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        20, 300, 980, 240,
        panel, NULL, hInstance, NULL
    );

    CreateWindow(
        "STATIC",
        "Planned Matrix Grids for Person 1:\n\n"
        "  [Table 1] Allocation Matrix (Editable cells for allocated instances)\n"
        "  [Table 2] Maximum Requirement Matrix (Editable cells for maximum declared demand)\n"
        "  [Table 3] Need Matrix (Auto-calculated: Need[i][j] = Maximum[i][j] - Allocation[i][j])\n"
        "  [Vector 1] Total Resource Instances Vector\n"
        "  [Vector 2] Available Resource Vector (Auto-calculated: Available = Total - Sum(Alloc))\n\n"
        "[Ready for Person 1 implementation]",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        35, 330, 940, 190,
        panel, NULL, hInstance, NULL
    );

    return panel;
}

void panel_setup_refresh(HWND panel, const SystemContext *ctx) {
    (void)panel;
    (void)ctx;
}

