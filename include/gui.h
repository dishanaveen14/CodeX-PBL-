/**
 * @file gui.h
 * @brief Header for Graphical User Interface.
 *
 * Integrated by: Student 4 (Integration and Main Application)
 * Course: Operating Systems - POSIX IPC Project
 */

#ifndef GUI_H
#define GUI_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Launches the simple graphical user interface (GUI).
 *
 * On Windows: Displays a clean Win32 window with interactive buttons for
 *             all 6 IPC mechanisms, parameter inputs, and real-time output viewer.
 * On Linux/POSIX: Displays an interactive visual ANSI TUI dashboard or launches
 *                 the web dashboard bridge.
 *
 * @param argc Command-line argument count.
 * @param argv Command-line argument vector.
 * @return Exit status code.
 */
int launch_gui(int argc, char *argv[]);

#ifdef __cplusplus
}
#endif

#endif /* GUI_H */
