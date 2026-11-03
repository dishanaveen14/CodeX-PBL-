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

static GuiApp *g_app_instance = NULL;

static void resize_panels(GuiApp *app) {
    if (!app || !app->hwnd_tab) return;

    RECT rc;
    GetClientRect(app->hwnd_main, &rc);

    /* Status bar height compensation */
    RECT rc_status;
    GetClientRect(app->hwnd_status_bar, &rc_status);
    int status_height = rc_status.bottom - rc_status.top;
    if (status_height <= 0) status_height = 22;

    int tab_w = rc.right;
    int tab_h = rc.bottom - status_height;

    MoveWindow(app->hwnd_tab, 0, 0, tab_w, tab_h, TRUE);

    /* Calculate display area inside tab control */
    RECT rc_display = { 0, 0, tab_w, tab_h };
    TabCtrl_AdjustRect(app->hwnd_tab, FALSE, &rc_display);

    for (int i = 0; i < TAB_COUNT; i++) {
        if (app->hwnd_panels[i]) {
            MoveWindow(app->hwnd_panels[i], 
                       rc_display.left, 
                       rc_display.top, 
                       rc_display.right - rc_display.left, 
                       rc_display.bottom - rc_display.top, 
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
        0, 0, GUI_WINDOW_WIDTH, GUI_WINDOW_HEIGHT - 30,
        app->hwnd_main,
        NULL,
        h_instance,
        NULL
    );

    /* Insert Navigation Tabs */
    const char *tab_titles[TAB_COUNT] = {
        "Dashboard",
        "Setup & Matrices (P1)",
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

    /* Initial panel display */
    resize_panels(app);
    gui_app_switch_tab(app, TAB_INDEX_DASHBOARD);
    gui_app_set_status_bar(app, "Project Skeleton Ready | Architecture Established | Modules Unloaded (TODO)");

    ShowWindow(app->hwnd_main, n_cmd_show);
    UpdateWindow(app->hwnd_main);

    LOG_INFO("GUI", "Deadlock Simulation System GUI window created successfully.");
    return ERR_SUCCESS;
}

void gui_app_switch_tab(GuiApp *app, GuiTab tab) {
    if (!app || tab >= TAB_COUNT) return;

    app->current_tab = tab;
    TabCtrl_SetCurSel(app->hwnd_tab, tab);

    for (int i = 0; i < TAB_COUNT; i++) {
        if (app->hwnd_panels[i]) {
            ShowWindow(app->hwnd_panels[i], (i == (int)tab) ? SW_SHOW : SW_HIDE);
        }
    }

    switch (tab) {
        case TAB_INDEX_DASHBOARD:
            gui_app_set_status_bar(app, "Dashboard: System Overview & Shared State Status");
            break;
        case TAB_INDEX_SETUP:
            gui_app_set_status_bar(app, "Person 1: Process and Resource Management [Skeleton Placeholder]");
            break;
        case TAB_INDEX_BANKERS:
            gui_app_set_status_bar(app, "Person 2: Banker's Algorithm and Safe State Analysis [Skeleton Placeholder]");
            break;
        case TAB_INDEX_DETECTION:
            gui_app_set_status_bar(app, "Person 3: Deadlock Detection, Requests & Recovery [Skeleton Placeholder]");
            break;
        case TAB_INDEX_RAG:
            gui_app_set_status_bar(app, "Resource Allocation Graph (RAG) Visualizer [Skeleton Placeholder]");
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

