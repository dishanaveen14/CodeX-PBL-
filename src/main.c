/**
 * @file main.c
 * @brief Application Entry Point.
 * 
 * Deadlock Simulation System - PBL Project
 * Person 4: Architecture, Integration, GUI Coordination, and Final Testing.
 */

#include <windows.h>
#include <stdio.h>
#include "../include/common/types.h"
#include "../include/person4_integration/coordinator.h"
#include "../include/gui/gui_app.h"
#include "../include/utils/logger.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    (void)hPrevInstance;
    (void)lpCmdLine;

    LOG_INFO("MAIN", "Initializing Deadlock Simulation System (Project Skeleton)...");

    /* 1. Initialize Master System Context */
    SystemContext sys_ctx;
    ErrorCode err = coordinator_init(&sys_ctx);
    if (err != ERR_SUCCESS) {
        LOG_ERROR("MAIN", "Failed to initialize system coordinator context.");
        MessageBox(NULL, "Failed to initialize system context.", "Fatal Error", MB_ICONERROR | MB_OK);
        return 1;
    }

    /* 2. Initialize GUI Shell */
    GuiApp app;
    err = gui_app_init(&app, hInstance, nCmdShow, &sys_ctx);
    if (err != ERR_SUCCESS) {
        LOG_ERROR("MAIN", "Failed to initialize GUI application window.");
        MessageBox(NULL, "Failed to initialize GUI window.", "Fatal Error", MB_ICONERROR | MB_OK);
        return 1;
    }

    LOG_INFO("MAIN", "Application initialized successfully. Entering GUI message loop.");

    /* 3. Run Application Message Loop */
    int exit_code = gui_app_run(&app);

    LOG_INFO("MAIN", "Application exiting with code: %d.", exit_code);
    return exit_code;
}

/* Fallback entry point for console builds */
int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    return WinMain(GetModuleHandle(NULL), NULL, GetCommandLineA(), SW_SHOWNORMAL);
}

