/**
 * @file main_window.h
 * @brief Main GTK window interface for Page Replacement Simulator.
 *
 * Owned by Person 3.
 */

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <gtk/gtk.h>
#include "gui/results_view.h"

/**
 * @brief Application window data structure holding references to main UI widgets.
 */
typedef struct {
    GtkWidget *window;
    GtkWidget *reference_entry;
    GtkWidget *frame_spin;
    GtkWidget *algo_combo;
    GtkWidget *run_button;
    GtkWidget *clear_button;
    GtkWidget *status_label;
    ResultsView *results_view;
} AppMainWindow;

/**
 * @brief Initializes and constructs the main application window and its controls.
 *
 * @param app GtkApplication instance or NULL.
 * @return Pointer to AppMainWindow context structure.
 */
AppMainWindow *create_main_window(GtkApplication *app);

/**
 * @brief Callback triggered when the "Run Simulation" button is clicked.
 *
 * @param button Source button.
 * @param user_data Pointer to AppMainWindow.
 */
void on_run_simulation_clicked(GtkButton *button, gpointer user_data);

/**
 * @brief Callback triggered when the "Clear" button is clicked.
 *
 * @param button Source button.
 * @param user_data Pointer to AppMainWindow.
 */
void on_clear_clicked(GtkButton *button, gpointer user_data);

#endif /* MAIN_WINDOW_H */

