/**
 * @file gui_app.c
 * @brief GUI Application Shell, Main Window, Tab Navigation, and Event Loop.
 * 
 * Deadlock Simulation System - PBL Project
 * Person 4: Architecture, GUI Coordination, and Integration.
 */

#include "../../include/gui/gui_app.h"
#include "../../include/gui/panel_dashboard.h"
#include "../../include/gui/panel_setup.h"
#include "../../include/gui/panel_bankers.h"
#include "../../include/gui/panel_detection.h"
#include "../../include/gui/panel_rag.h"
#include "../../include/utils/logger.h"
#include <stdio.h>

#define MAIN_WINDOW_CLASS "DeadlockSimulationSystemMain"
#define TAB_BAR_HEIGHT    30

static GuiApp *g_app_instance = NULL;
static HFONT g_default_gui_font = NULL;

void gui_apply_standard_font(HWND parent) {
    if (!parent) return;

    if (!g_default_gui_font) {
        /* Crisp, modern Segoe UI (approx 9pt / 13px at 96 DPI) */
        g_default_gui_font = CreateFont(
            -13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI"
        );
        if (!g_default_gui_font) {
            g_default_gui_font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        }
    }

    HWND child = GetWindow(parent, GW_CHILD);
    while (child) {
        SendMessage(child, WM_SETFONT, (WPARAM)g_default_gui_font, TRUE);
        child = GetWindow(child, GW_HWNDNEXT);
    }
}

static void resize_panels(GuiApp *app) {
    if (!app || !app->hwnd_tab) return;

    RECT rc;
    GetClientRect(app->hwnd_main, &rc);

    /* Status bar height compensation */
    RECT rc_status;
    GetClientRect(app->hwnd_status_bar, &rc_status);
    int status_height = rc_status.bottom - rc_status.top;
    if (status_height <= 0) status_height = 22;

    int total_w = rc.right;
    int client_h = rc.bottom - status_height;

    /* Sizing tab control to the top strip (height 30) avoids sibling Z-order collision */
    MoveWindow(app->hwnd_tab, 0, 0, total_w, TAB_BAR_HEIGHT, TRUE);

    int panel_y = TAB_BAR_HEIGHT;
    int panel_h = client_h - panel_y;
    if (panel_h < 0) panel_h = 0;

    for (int i = 0; i < TAB_COUNT; i++) {
        if (app->hwnd_panels[i]) {
            MoveWindow(app->hwnd_panels[i], 
                       0, 
                       panel_y, 
                       total_w, 
                       panel_h, 
                       TRUE);
        }
    }

    SendMessage(app->hwnd_status_bar, WM_SIZE, 0, 0);
}

static LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            return 0;

        case WM_SIZE: {
            if (g_app_instance) {
                resize_panels(g_app_instance);
            }
            return 0;
        }

        case WM_GETMINMAXINFO: {
            LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
            lpMMI->ptMinTrackSize.x = GUI_WINDOW_MIN_W;
            lpMMI->ptMinTrackSize.y = GUI_WINDOW_MIN_H;
            return 0;
        }

        case WM_NOTIFY: {
            LPNMHDR nmhdr = (LPNMHDR)lParam;
            if (g_app_instance && nmhdr->hwndFrom == g_app_instance->hwnd_tab && nmhdr->code == TCN_SELCHANGE) {
                int selected = TabCtrl_GetCurSel(g_app_instance->hwnd_tab);
                gui_app_switch_tab(g_app_instance, (GuiTab)selected);
            }
            return 0;
        }

        case WM_DESTROY:
            if (g_default_gui_font) {
                DeleteObject(g_default_gui_font);
                g_default_gui_font = NULL;
            }
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

ErrorCode gui_app_init(GuiApp *app, HINSTANCE h_instance, int n_cmd_show, SystemContext *ctx) {
    if (!app || !ctx) {
        return ERR_INVALID_PARAM;
    }

    g_app_instance = app;
    app->h_instance = h_instance;
    app->system_ctx = ctx;
    app->current_tab = TAB_INDEX_DASHBOARD;

    /* Initialize Common Controls for Tabs and Status Bar */
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_TAB_CLASSES | ICC_BAR_CLASSES;
    InitCommonControlsEx(&icex);

    /* Register Main Window Class */
    WNDCLASSEX wc;
    memset(&wc, 0, sizeof(wc));
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MainWindowProc;
    wc.hInstance = h_instance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = MAIN_WINDOW_CLASS;

    if (!RegisterClassEx(&wc)) {
        LOG_ERROR("GUI", "Failed to register main window class.");
        return ERR_INVALID_PARAM;
    }

    /* Create Main Window */
    app->hwnd_main = CreateWindowEx(
        0,
        MAIN_WINDOW_CLASS,
        "Deadlock Simulation System - PBL Architecture Skeleton",
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT,
        GUI_WINDOW_WIDTH, GUI_WINDOW_HEIGHT,
        NULL, NULL, h_instance, NULL
    );

    if (!app->hwnd_main) {
        LOG_ERROR("GUI", "Failed to create main application window.");
        return ERR_INVALID_PARAM;
    }

    /* Create Status Bar */
    app->hwnd_status_bar = CreateWindowEx(
        0,
        STATUSCLASSNAME,
        NULL,
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0,
        app->hwnd_main,
        NULL,
        h_instance,
        NULL
    );

    /* Create Tab Control */
    app->hwnd_tab = CreateWindowEx(
        0,
        WC_TABCONTROL,
        "",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
        0, 0, GUI_WINDOW_WIDTH, TAB_BAR_HEIGHT,
        app->hwnd_main,
        NULL,
        h_instance,
        NULL
    );

    /* Insert Navigation Tabs (double ampersand '&&' escapes accelerator underscore) */
    const char *tab_titles[TAB_COUNT] = {
        "Dashboard",
        "Setup && Matrices (P1)",
        "Banker's Algorithm (P2)",
        "Deadlock Detection (P3)",
        "RAG Visualizer (P3/P4)"
    };

    for (int i = 0; i < TAB_COUNT; i++) {
        TCITEM tie;
        tie.mask = TCIF_TEXT;
        tie.pszText = (LPSTR)tab_titles[i];
        TabCtrl_InsertItem(app->hwnd_tab, i, &tie);
    }

    /* Instantiate Modular Panels */
    app->hwnd_panels[TAB_INDEX_DASHBOARD] = panel_dashboard_create(app->hwnd_main, ctx);
    app->hwnd_panels[TAB_INDEX_SETUP]     = panel_setup_create(app->hwnd_main, ctx);
    app->hwnd_panels[TAB_INDEX_BANKERS]   = panel_bankers_create(app->hwnd_main, ctx);
    app->hwnd_panels[TAB_INDEX_DETECTION] = panel_detection_create(app->hwnd_main, ctx);
    app->hwnd_panels[TAB_INDEX_RAG]       = panel_rag_create(app->hwnd_main, ctx);

    /* Apply clean font to all panels and tab header */
    for (int i = 0; i < TAB_COUNT; i++) {
        if (app->hwnd_panels[i]) {
            gui_apply_standard_font(app->hwnd_panels[i]);
        }
    }
    if (g_default_gui_font) {
        SendMessage(app->hwnd_tab, WM_SETFONT, (WPARAM)g_default_gui_font, TRUE);
    }

    /* Initial panel display */
    resize_panels(app);
    gui_app_switch_tab(app, TAB_INDEX_DASHBOARD);
    gui_app_set_status_bar(app, "Dashboard: System Overview & Shared State Status");

    ShowWindow(app->hwnd_main, n_cmd_show);
    UpdateWindow(app->hwnd_main);

    /* Trigger layout and display refresh after window show */
    gui_app_switch_tab(app, TAB_INDEX_DASHBOARD);

    LOG_INFO("GUI", "Deadlock Simulation System GUI window created successfully.");
    return ERR_SUCCESS;
}

void gui_app_switch_tab(GuiApp *app, GuiTab tab) {
    if (!app || tab >= TAB_COUNT) return;

    app->current_tab = tab;
    TabCtrl_SetCurSel(app->hwnd_tab, tab);

    /* 1. Hide all non-selected panels first to prevent bleed-through */
    for (int i = 0; i < TAB_COUNT; i++) {
        if (app->hwnd_panels[i] && i != (int)tab) {
            ShowWindow(app->hwnd_panels[i], SW_HIDE);
        }
    }

    /* 2. Show, bring to front, and repaint active panel */
    if (app->hwnd_panels[tab]) {
        ShowWindow(app->hwnd_panels[tab], SW_SHOW);
        BringWindowToTop(app->hwnd_panels[tab]);
        SetWindowPos(app->hwnd_panels[tab], HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
        InvalidateRect(app->hwnd_panels[tab], NULL, TRUE);
        UpdateWindow(app->hwnd_panels[tab]);
    }

    switch (tab) {
        case TAB_INDEX_DASHBOARD:
            panel_dashboard_refresh(app->hwnd_panels[TAB_INDEX_DASHBOARD], app->system_ctx);
            gui_app_set_status_bar(app, "Dashboard: System Overview & Shared State Status");
            break;
        case TAB_INDEX_SETUP:
            panel_setup_refresh(app->hwnd_panels[TAB_INDEX_SETUP], app->system_ctx);
            gui_app_set_status_bar(app, "Person 1: Process and Resource Management (Active Module)");
            break;
        case TAB_INDEX_BANKERS:
            panel_bankers_refresh(app->hwnd_panels[TAB_INDEX_BANKERS], app->system_ctx);
            gui_app_set_status_bar(app, "Person 2: Banker's Algorithm and Safe State Analysis (Active Module)");
            break;
        case TAB_INDEX_DETECTION:
            panel_detection_refresh(app->hwnd_panels[TAB_INDEX_DETECTION], app->system_ctx);
            gui_app_set_status_bar(app, "Person 3: Deadlock Detection, Requests & Recovery (Active Module)");
            break;
        case TAB_INDEX_RAG:
            panel_rag_refresh(app->hwnd_panels[TAB_INDEX_RAG], app->system_ctx);
            gui_app_set_status_bar(app, "Person 3 / 4: Resource Allocation Graph (RAG) Visualizer (Active Module)");
            break;
        default:
            break;
    }
}

void gui_app_set_status_bar(GuiApp *app, const char *status_text) {
    if (app && app->hwnd_status_bar && status_text) {
        SendMessage(app->hwnd_status_bar, SB_SETTEXT, 0, (LPARAM)status_text);
    }
}

int gui_app_run(GuiApp *app) {
    if (!app) return -1;

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}
