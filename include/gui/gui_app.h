/**
 * @file gui_app.h
 * @brief GUI Application Shell, Main Window, Navigation, and Event Loop.
 * 
 * Deadlock Simulation System - PBL Project
 * Person 4: GUI Architecture and Coordination.
 */

#ifndef GUI_APP_H
#define GUI_APP_H

#include <windows.h>
#include <commctrl.h>
#include "../common/types.h"
#include "../person4_integration/coordinator.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Window Dimensions */
#define GUI_WINDOW_WIDTH    1080
#define GUI_WINDOW_HEIGHT   720
#define GUI_WINDOW_MIN_W    800
#define GUI_WINDOW_MIN_H    600

/* Tab Navigation Identifiers */
typedef enum {
    TAB_INDEX_DASHBOARD = 0,
    TAB_INDEX_SETUP,         /* Person 1 */
    TAB_INDEX_BANKERS,       /* Person 2 */
    TAB_INDEX_DETECTION,     /* Person 3 */
    TAB_INDEX_RAG,           /* Person 3 / RAG Visualizer */
    TAB_COUNT
} GuiTab;

/**
 * @brief GUI Application state and handle container.
 */
typedef struct {
    HINSTANCE h_instance;
    HWND hwnd_main;
    HWND hwnd_tab;
    HWND hwnd_status_bar;
    HWND hwnd_panels[TAB_COUNT];
    GuiTab current_tab;
    SystemContext *system_ctx;
} GuiApp;

/**
 * @brief Initializes and creates the main application window and shell.
 * @param app Pointer to GuiApp structure.
 * @param h_instance Windows application instance handle.
 * @param n_cmd_show Show command flag.
 * @param ctx Pointer to initialized SystemContext.
 * @return ERR_SUCCESS on success, ERR_INVALID_PARAM or error code on failure.
 */
ErrorCode gui_app_init(GuiApp *app, HINSTANCE h_instance, int n_cmd_show, SystemContext *ctx);

/**
 * @brief Runs the main Win32 GUI message loop.
 * @param app Pointer to GuiApp structure.
 * @return Exit code from message loop.
 */
int gui_app_run(GuiApp *app);

/**
 * @brief Switches the active view panel in the GUI shell.
 * @param app Pointer to GuiApp structure.
 * @param tab The tab index to activate.
 */
void gui_app_switch_tab(GuiApp *app, GuiTab tab);

/**
 * @brief Updates the text displayed on the main status bar.
 * @param app Pointer to GuiApp structure.
 * @param status_text Status message to display.
 */
void gui_app_set_status_bar(GuiApp *app, const char *status_text);

#ifdef __cplusplus
}
#endif

#endif /* GUI_APP_H */

