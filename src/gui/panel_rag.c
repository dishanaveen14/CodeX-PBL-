/**
 * @file panel_rag.c
 * @brief Resource Allocation Graph (RAG) Visualizer GUI Panel Implementation.
 * 
 * Deadlock Simulation System - PBL Project
 * Assigned to: Person 3 (Deadlock Detection, Recovery, and RAG)
 */

#include "../../include/gui/panel_rag.h"
#include "../../include/person3_detection_recovery/rag.h"
#include "../../include/person3_detection_recovery/detection.h"
#include <stdio.h>
#include <math.h>

#define PANEL_RAG_CLASS "DeadlockSimPanelRAG"
#define IDC_BTN_REFRESH_RAG 4001

static SystemContext *g_rag_ctx = NULL;
static HWND g_hwnd_rag_panel = NULL;
static RAGGraph g_rag_graph;

static void draw_arrow_head(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color) {
    float dx = (float)(x2 - x1);
    float dy = (float)(y2 - y1);
    float length = sqrtf(dx * dx + dy * dy);
    if (length < 1.0f) return;

    float ux = dx / length;
    float uy = dy / length;

    /* Arrow tip offset slightly before destination node center */
    float tip_x = (float)x2 - ux * 22.0f;
    float tip_y = (float)y2 - uy * 22.0f;

    float arrow_len = 10.0f;
    float arrow_width = 5.0f;

    POINT pts[3];
    pts[0].x = (long)tip_x;
    pts[0].y = (long)tip_y;
    pts[1].x = (long)(tip_x - ux * arrow_len + uy * arrow_width);
    pts[1].y = (long)(tip_y - uy * arrow_len - ux * arrow_width);
    pts[2].x = (long)(tip_x - ux * arrow_len - uy * arrow_width);
    pts[2].y = (long)(tip_y - uy * arrow_len + ux * arrow_width);

    HBRUSH brush = CreateSolidBrush(color);
    HPEN pen = CreatePen(PS_SOLID, 1, color);
    HBRUSH old_brush = (HBRUSH)SelectObject(hdc, brush);
    HPEN old_pen = (HPEN)SelectObject(hdc, pen);

    Polygon(hdc, pts, 3);

    SelectObject(hdc, old_brush);
    SelectObject(hdc, old_pen);
    DeleteObject(brush);
    DeleteObject(pen);
}

static void draw_rag(HDC hdc, RECT *rect) {
    /* 1. Background Fill */
    HBRUSH bg_brush = CreateSolidBrush(RGB(250, 252, 255));
    FillRect(hdc, rect, bg_brush);
    DeleteObject(bg_brush);

    /* Canvas Boundary */
    RECT canvas = { 20, 50, rect->right - 20, rect->bottom - 90 };
    HPEN border_pen = CreatePen(PS_SOLID, 1, RGB(203, 213, 225));
    HPEN old_pen = (HPEN)SelectObject(hdc, border_pen);
    Rectangle(hdc, canvas.left, canvas.top, canvas.right, canvas.bottom);
    SelectObject(hdc, old_pen);
    DeleteObject(border_pen);

    SetBkMode(hdc, TRANSPARENT);

    /* 2. Title Banner */
    HFONT title_font = CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                  ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                  DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    HFONT old_font = (HFONT)SelectObject(hdc, title_font);
    SetTextColor(hdc, RGB(30, 41, 59));
    TextOut(hdc, 25, 20, "Resource Allocation Graph (RAG) — Directed Bipartite System View", 63);

    HFONT label_font = CreateFont(14, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                                  ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                  DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    SelectObject(hdc, label_font);

    /* Check if graph has nodes */
    if (g_rag_graph.node_count == 0) {
        RECT msg_rect = canvas;
        SetTextColor(hdc, RGB(148, 163, 184));
        DrawText(hdc, "No processes or resources loaded.\nLoad a configuration preset in 'Setup & Matrices (P1)' tab.",
                 -1, &msg_rect, DT_CENTER | DT_VCENTER | DT_WORDBREAK);
        SelectObject(hdc, old_font);
        DeleteObject(title_font);
        DeleteObject(label_font);
        return;
    }

    /* 3. Draw Directed Edges */
    for (int e = 0; e < g_rag_graph.edge_count; e++) {
        const RAGEdge *edge = &g_rag_graph.edges[e];

        /* Find source and destination node positions */
        int x1 = 0, y1 = 0, x2 = 0, y2 = 0;
        for (int n = 0; n < g_rag_graph.node_count; n++) {
            if (g_rag_graph.nodes[n].type == edge->from_type && g_rag_graph.nodes[n].id == edge->from_id) {
                x1 = g_rag_graph.nodes[n].pos_x;
                y1 = g_rag_graph.nodes[n].pos_y;
            }
            if (g_rag_graph.nodes[n].type == edge->to_type && g_rag_graph.nodes[n].id == edge->to_id) {
                x2 = g_rag_graph.nodes[n].pos_x;
                y2 = g_rag_graph.nodes[n].pos_y;
            }
        }

        COLORREF edge_color;
        int pen_style;
        int pen_width;

        if (edge->is_deadlock_cycle) {
            edge_color = RGB(220, 38, 38); /* Red for deadlock cycle */
            pen_style = PS_SOLID;
            pen_width = 3;
        } else if (edge->edge_type == RAG_EDGE_ALLOCATION) {
            edge_color = RGB(2, 132, 199); /* Blue for allocation */
            pen_style = PS_SOLID;
            pen_width = 2;
        } else {
            edge_color = RGB(217, 119, 6); /* Amber for request */
            pen_style = PS_DASH;
            pen_width = 1;
        }

        HPEN edge_pen = CreatePen(pen_style, pen_width, edge_color);
        old_pen = (HPEN)SelectObject(hdc, edge_pen);

        MoveToEx(hdc, x1, y1, NULL);
        LineTo(hdc, x2, y2);

        SelectObject(hdc, old_pen);
        DeleteObject(edge_pen);

        /* Draw Arrow Head */
        draw_arrow_head(hdc, x1, y1, x2, y2, edge_color);

        /* Draw instance count badge midway */
        int mid_x = (x1 + x2) / 2;
        int mid_y = (y1 + y2) / 2;
        char cnt_str[16];
        snprintf(cnt_str, sizeof(cnt_str), "%d", edge->instance_count);

        SetTextColor(hdc, edge_color);
        TextOut(hdc, mid_x - 4, mid_y - 8, cnt_str, (int)strlen(cnt_str));
    }

    /* 4. Draw Graph Nodes */
    for (int n = 0; n < g_rag_graph.node_count; n++) {
        const RAGNode *node = &g_rag_graph.nodes[n];

        if (node->type == RAG_NODE_PROCESS) {
            /* Process Node: Circle */
            int r = 22;
            COLORREF fill_color = node->is_in_deadlock ? RGB(254, 226, 226) :
                (node->total_or_status == PROCESS_STATUS_WAITING ? RGB(254, 243, 199) :
                (node->total_or_status == PROCESS_STATUS_TERMINATED ? RGB(241, 245, 249) : RGB(224, 242, 254)));

            COLORREF stroke_color = node->is_in_deadlock ? RGB(220, 38, 38) :
                (node->total_or_status == PROCESS_STATUS_WAITING ? RGB(217, 119, 6) :
                (node->total_or_status == PROCESS_STATUS_TERMINATED ? RGB(148, 163, 184) : RGB(2, 132, 199)));

            HBRUSH brush = CreateSolidBrush(fill_color);
            HPEN pen = CreatePen(PS_SOLID, node->is_in_deadlock ? 3 : 2, stroke_color);

            HBRUSH ob = (HBRUSH)SelectObject(hdc, brush);
            HPEN op = (HPEN)SelectObject(hdc, pen);

            Ellipse(hdc, node->pos_x - r, node->pos_y - r, node->pos_x + r, node->pos_y + r);

            SelectObject(hdc, ob);
            SelectObject(hdc, op);
            DeleteObject(brush);
            DeleteObject(pen);

            /* Label */
            RECT text_rect = { node->pos_x - r, node->pos_y - 8, node->pos_x + r, node->pos_y + 12 };
            SetTextColor(hdc, node->is_in_deadlock ? RGB(185, 28, 28) : RGB(15, 23, 42));
            DrawText(hdc, node->label, -1, &text_rect, DT_CENTER | DT_SINGLELINE);

        } else {
            /* Resource Node: Rounded Rectangle */
            int w = 70;
            int h = 42;
            int left = node->pos_x - w / 2;
            int top = node->pos_y - h / 2;
            int right = node->pos_x + w / 2;
            int bottom = node->pos_y + h / 2;

            COLORREF fill_color = node->is_in_deadlock ? RGB(254, 226, 226) : RGB(236, 253, 245);
            COLORREF stroke_color = node->is_in_deadlock ? RGB(220, 38, 38) : RGB(16, 185, 129);

            HBRUSH brush = CreateSolidBrush(fill_color);
            HPEN pen = CreatePen(PS_SOLID, node->is_in_deadlock ? 3 : 2, stroke_color);

            HBRUSH ob = (HBRUSH)SelectObject(hdc, brush);
            HPEN op = (HPEN)SelectObject(hdc, pen);

            RoundRect(hdc, left, top, right, bottom, 10, 10);

            SelectObject(hdc, ob);
            SelectObject(hdc, op);
            DeleteObject(brush);
            DeleteObject(pen);

            /* Label */
            RECT text_rect = { left, top + 10, right, bottom };
            SetTextColor(hdc, node->is_in_deadlock ? RGB(185, 28, 28) : RGB(6, 95, 70));
            DrawText(hdc, node->label, -1, &text_rect, DT_CENTER | DT_SINGLELINE);
        }
    }

    /* 5. Draw Legend at Bottom */
    int leg_y = rect->bottom - 80;
    RECT leg_box = { 20, leg_y, rect->right - 20, rect->bottom - 15 };
    HBRUSH leg_bg = CreateSolidBrush(RGB(241, 245, 249));
    FillRect(hdc, &leg_box, leg_bg);
    DeleteObject(leg_bg);

    HPEN leg_pen = CreatePen(PS_SOLID, 1, RGB(203, 213, 225));
    old_pen = (HPEN)SelectObject(hdc, leg_pen);
    Rectangle(hdc, leg_box.left, leg_box.top, leg_box.right, leg_box.bottom);
    SelectObject(hdc, old_pen);
    DeleteObject(leg_pen);

    SetTextColor(hdc, RGB(51, 65, 85));
    TextOut(hdc, 35, leg_y + 10, "LEGEND:", 7);

    /* Process Circle */
    HBRUSH pb = CreateSolidBrush(RGB(224, 242, 254));
    HPEN pp = CreatePen(PS_SOLID, 2, RGB(2, 132, 199));
    SelectObject(hdc, pb); SelectObject(hdc, pp);
    Ellipse(hdc, 100, leg_y + 10, 120, leg_y + 30);
    TextOut(hdc, 125, leg_y + 10, "Process Node", 12);
    DeleteObject(pb); DeleteObject(pp);

    /* Resource Rect */
    HBRUSH rb = CreateSolidBrush(RGB(236, 253, 245));
    HPEN rp = CreatePen(PS_SOLID, 2, RGB(16, 185, 129));
    SelectObject(hdc, rb); SelectObject(hdc, rp);
    RoundRect(hdc, 230, leg_y + 8, 255, leg_y + 32, 6, 6);
    TextOut(hdc, 262, leg_y + 10, "Resource Node (Total)", 21);
    DeleteObject(rb); DeleteObject(rp);

    /* Solid Arrow: Allocation */
    HPEN ap = CreatePen(PS_SOLID, 2, RGB(2, 132, 199));
    SelectObject(hdc, ap);
    MoveToEx(hdc, 430, leg_y + 20, NULL); LineTo(hdc, 470, leg_y + 20);
    draw_arrow_head(hdc, 430, leg_y + 20, 470, leg_y + 20, RGB(2, 132, 199));
    TextOut(hdc, 480, leg_y + 10, "Allocation (Resource -> Process)", 32);
    DeleteObject(ap);

    /* Dashed Arrow: Request */
    HPEN qp = CreatePen(PS_DASH, 1, RGB(217, 119, 6));
    SelectObject(hdc, qp);
    MoveToEx(hdc, 720, leg_y + 20, NULL); LineTo(hdc, 760, leg_y + 20);
    draw_arrow_head(hdc, 720, leg_y + 20, 760, leg_y + 20, RGB(217, 119, 6));
    TextOut(hdc, 770, leg_y + 10, "Request (Process -> Resource)", 29);
    DeleteObject(qp);

    /* Deadlock Highlight */
    SetTextColor(hdc, RGB(220, 38, 38));
    TextOut(hdc, 35, leg_y + 36, "[Red Outline / Thick Red Arrows = Participating in Active Deadlock Cycle]", 73);

    SelectObject(hdc, old_font);
    DeleteObject(title_font);
    DeleteObject(label_font);
}

void panel_rag_refresh(HWND panel, const SystemContext *ctx) {
    if (ctx) {
        g_rag_ctx = (SystemContext*)ctx;
        rag_build_from_state(&ctx->live_state, &g_rag_graph);
    }
    if (panel) {
        InvalidateRect(panel, NULL, TRUE);
    }
}

static LRESULT CALLBACK RAGPanelProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_COMMAND: {
            if (LOWORD(wParam) == IDC_BTN_REFRESH_RAG) {
                if (g_rag_ctx) {
                    panel_rag_refresh(hwnd, g_rag_ctx);
                }
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
            return 1;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rect;
            GetClientRect(hwnd, &rect);

            /* Double-buffering to prevent flicker */
            HDC mem_dc = CreateCompatibleDC(hdc);
            HBITMAP mem_bm = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
            HBITMAP old_bm = (HBITMAP)SelectObject(mem_dc, mem_bm);

            draw_rag(mem_dc, &rect);

            BitBlt(hdc, 0, 0, rect.right, rect.bottom, mem_dc, 0, 0, SRCCOPY);

            SelectObject(mem_dc, old_bm);
            DeleteObject(mem_bm);
            DeleteDC(mem_dc);

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
    g_rag_ctx = ctx;
    HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(parent, GWLP_HINSTANCE);
    register_class_once(hInstance);

    HWND panel = CreateWindowEx(
        0,
        PANEL_RAG_CLASS,
        NULL,
        WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        0, 0, 100, 100,
        parent,
        NULL,
        hInstance,
        NULL
    );
    g_hwnd_rag_panel = panel;

    /* Refresh Button */
    CreateWindow("BUTTON", "Re-generate && Refresh Graph", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                 750, 12, 230, 28, panel, (HMENU)IDC_BTN_REFRESH_RAG, hInstance, NULL);

    if (ctx) {
        panel_rag_refresh(panel, ctx);
    }

    return panel;
}
