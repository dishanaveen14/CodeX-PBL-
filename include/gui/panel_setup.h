/**
 * @file panel_setup.h
 * @brief Person 1: Process and Resource Setup GUI Panel Placeholder.
 * 
 * Deadlock Simulation System - PBL Project
 */

#ifndef PANEL_SETUP_H
#define PANEL_SETUP_H

#include <windows.h>
#include "../person4_integration/coordinator.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Creates the Process and Resource Management GUI panel shell.
 * @param parent Parent window handle.
 * @param ctx Pointer to application context.
 * @return Handle to the created panel window.
 */
HWND panel_setup_create(HWND parent, SystemContext *ctx);

/**
 * @brief Refreshes setup panel display.
 * @param panel Panel window handle.
 * @param ctx Pointer to application context.
 */
void panel_setup_refresh(HWND panel, const SystemContext *ctx);

#ifdef __cplusplus
}
#endif

#endif /* PANEL_SETUP_H */

