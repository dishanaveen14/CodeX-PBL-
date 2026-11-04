/**
 * @file main.c
 * @brief Application entry point for GTK-based Page Replacement Simulator.
 *
 * Owned by Person 3 (GUI).
 */

#include <gtk/gtk.h>
#include "gui/main_window.h"

int main(int argc, char *argv[]) {
    /* Initialize GTK framework */
    gtk_init(&argc, &argv);

    /* Construct main application window */
    AppMainWindow *app_win = create_main_window(NULL);
    if (!app_win || !app_win->window) {
        g_printerr("Failed to initialize GTK application window.\n");
        return 1;
    }

    /* Terminate GTK main loop when window is closed */
    g_signal_connect(app_win->window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    /* Display window and start GTK event loop */
    gtk_widget_show_all(app_win->window);
    gtk_main();

    return 0;
}

