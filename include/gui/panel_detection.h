/**
 * @file panel_detection.h
 * @brief Person 3: Deadlock Detection and Recovery GUI Panel Placeholder.
 * 
 * Deadlock Simulation System - PBL Project
 */

#ifndef PANEL_DETECTION_H
#define PANEL_DETECTION_H

#include <windows.h>
#include "../person4_integration/coordinator.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Creates the Deadlock Detection & Recovery GUI panel shell.
 * @param parent Parent window handle.
 * @param ctx Pointer to application context.
 * @return Handle to the created panel window.
 */
HWND panel_detection_create(HWND parent, SystemContext *ctx);

/**
 * @brief Refreshes detection panel display.
 * @param panel Panel window handle.
 * @param ctx Pointer to application context.
 */
void panel_detection_refresh(HWND panel, const SystemContext *ctx);

#ifdef __cplusplus
}
#endif

#endif /* PANEL_DETECTION_H */

