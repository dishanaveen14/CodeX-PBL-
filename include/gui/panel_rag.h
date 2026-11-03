/**
 * @file panel_rag.h
 * @brief Resource Allocation Graph (RAG) Visualizer GUI Panel Placeholder.
 * 
 * Deadlock Simulation System - PBL Project
 */

#ifndef PANEL_RAG_H
#define PANEL_RAG_H

#include <windows.h>
#include "../person4_integration/coordinator.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Creates the Resource Allocation Graph (RAG) visualization panel shell.
 * @param parent Parent window handle.
 * @param ctx Pointer to application context.
 * @return Handle to the created panel window.
 */
HWND panel_rag_create(HWND parent, SystemContext *ctx);

/**
 * @brief Refreshes RAG visualization panel display.
 * @param panel Panel window handle.
 * @param ctx Pointer to application context.
 */
void panel_rag_refresh(HWND panel, const SystemContext *ctx);

#ifdef __cplusplus
}
#endif

#endif /* PANEL_RAG_H */

