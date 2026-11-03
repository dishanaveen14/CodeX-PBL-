/**
 * @file panel_rag.c
 * @brief Resource Allocation Graph (RAG) Visualizer GUI Panel Placeholder.
 * 
 * Deadlock Simulation System - PBL Project
 */

#include "../../include/gui/panel_rag.h"
#include <stdio.h>

#define PANEL_RAG_CLASS "DeadlockSimPanelRAG"

static LRESULT CALLBACK RAGPanelProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rect;
            GetClientRect(hwnd, &rect);
            FillRect(hdc, &rect, (HBRUSH)(COLOR_BTNFACE + 1));

            /* Draw a canvas border placeholder for RAG Graph */
            RECT canvas = { 20, 100, rect.right - 20, rect.bottom - 40 };
            FrameRect(hdc, &canvas, (HBRUSH)GetStockObject(BLACK_BRUSH));

            /* Center placeholder text inside canvas */
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(100, 100, 100));
            RECT textRect = canvas;
            DrawText(hdc, 
                     "[ Resource Allocation Graph (RAG) Interactive Canvas ]\n\n"
                     "Nodes: Circles = Processes (P0, P1..), Rectangles = Resource Types (R0, R1..)\n"
                     "Edges: Solid Arrow = Allocation (Resource -> Process)\n"
                     "       Dashed Arrow = Request (Process -> Resource)\n"
                     "Highlight: Red Cycles = Deadlock Component\n\n"
                     "[Ready for Person 3 graph builder and Person 4 rendering integration]", 
                     -1, &textRect, DT_CENTER | DT_VCENTER | DT_WORDBREAK);

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
        wc.lpfnWndProc = RAGPanelProc;
        wc.hInstance = hInstance;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
        wc.lpszClassName = PANEL_RAG_CLASS;
        RegisterClassEx(&wc);
        registered = true;
    }
}

HWND panel_rag_create(HWND parent, SystemContext *ctx) {
    (void)ctx;
    HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(parent, GWLP_HINSTANCE);
    register_class_once(hInstance);

    HWND panel = CreateWindowEx(
        0,
        PANEL_RAG_CLASS,
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
        "Resource Allocation Graph (RAG) Visualizer (Person 3 Data / Person 4 Render)",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 20, 750, 24,
        panel, NULL, hInstance, NULL
    );

    CreateWindow(
        "STATIC",
        "Visual model representing active allocations and pending requests as directed edges.\n"
        "Interfaces defined in 'include/person3_detection_recovery/rag.h'.",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 50, 750, 35,
        panel, NULL, hInstance, NULL
    );

    return panel;
}

void panel_rag_refresh(HWND panel, const SystemContext *ctx) {
    (void)panel;
    (void)ctx;
    InvalidateRect(panel, NULL, TRUE);
}

