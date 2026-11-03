/**
 * @file panel_bankers.h
 * @brief Person 2: Banker's Algorithm & Safe-State Analysis GUI Panel Placeholder.
 * 
 * Deadlock Simulation System - PBL Project
 */

#ifndef PANEL_BANKERS_H
#define PANEL_BANKERS_H

#include <windows.h>
#include "../person4_integration/coordinator.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Creates the Banker's Algorithm analysis GUI panel shell.
 * @param parent Parent window handle.
 * @param ctx Pointer to application context.
 * @return Handle to the created panel window.
 */
HWND panel_bankers_create(HWND parent, SystemContext *ctx);

/**
 * @brief Refreshes Banker's analysis panel display.
 * @param panel Panel window handle.
 * @param ctx Pointer to application context.
 */
void panel_bankers_refresh(HWND panel, const SystemContext *ctx);

#ifdef __cplusplus
}
#endif

#endif /* PANEL_BANKERS_H */

