/**
 * @file panel_dashboard.h
 * @brief Dashboard Overview Panel (System Status, Quick Summary).
 * 
 * Deadlock Simulation System - PBL Project
 */

#ifndef PANEL_DASHBOARD_H
#define PANEL_DASHBOARD_H

#include <windows.h>
#include "../person4_integration/coordinator.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Creates the Dashboard overview panel placeholder.
 * @param parent Parent window handle (Tab control or container).
 * @param ctx Pointer to application context.
 * @return Handle to the created panel window.
 */
HWND panel_dashboard_create(HWND parent, SystemContext *ctx);

/**
 * @brief Refreshes dashboard placeholder elements.
 * @param panel Panel window handle.
 * @param ctx Pointer to application context.
 */
void panel_dashboard_refresh(HWND panel, const SystemContext *ctx);

#ifdef __cplusplus
}
#endif

#endif /* PANEL_DASHBOARD_H */

