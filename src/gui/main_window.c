/**
 * @file main_window.c
 * @brief Implementation of the main GTK application window and interaction controls.
 *
 * Owned by Person 3.
 */

#include "gui/main_window.h"
#include "gui/input_validation.h"
#include "gui/results_view.h"
#include "core/simulation.h"
#include "utils/memory.h"
#include <stdio.h>
#include <stdlib.h>

void on_run_simulation_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    AppMainWindow *app_win = (AppMainWindow *)user_data;
    if (!app_win) {
        return;
    }

    /* 1. Retrieve raw reference string */
    const char *raw_refs = gtk_entry_get_text(GTK_ENTRY(app_win->reference_entry));

    /* 2. Retrieve frame count */
    int frame_count = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(app_win->frame_spin));

    /* 3. Retrieve algorithm choice */
    int combo_active = gtk_combo_box_get_active(GTK_COMBO_BOX(app_win->algo_combo));
    AlgorithmChoice choice = ALGORITHM_FIFO;
    switch (combo_active) {
        case 0: choice = ALGORITHM_FIFO; break;
        case 1: choice = ALGORITHM_LRU; break;
        case 2: choice = ALGORITHM_OPTIMAL; break;
        case 3: choice = ALGORITHM_ALL; break;
        default: choice = ALGORITHM_FIFO; break;
    }

    /* 4. Validate frame count */
    ValidationStatus frame_status = validate_frame_count(frame_count);
    if (frame_status != VALIDATION_SUCCESS) {
        char err_markup[256];
        snprintf(err_markup, sizeof(err_markup), "<span foreground='#c62828'><b>Error:</b> %s</span>",
                 validation_status_message(frame_status));
        gtk_label_set_markup(GTK_LABEL(app_win->status_label), err_markup);
        clear_results_view(app_win->results_view);
        return;
    }

    /* 5. Parse and validate reference string */
    int *references = NULL;
    int reference_count = 0;
    ValidationStatus ref_status = parse_and_validate_reference_string(raw_refs, &references, &reference_count);
    if (ref_status != VALIDATION_SUCCESS) {
        char err_markup[256];
        snprintf(err_markup, sizeof(err_markup), "<span foreground='#c62828'><b>Error:</b> %s</span>",
                 validation_status_message(ref_status));
        gtk_label_set_markup(GTK_LABEL(app_win->status_label), err_markup);
        clear_results_view(app_win->results_view);
        return;
    }

    /* 6. Execute simulation via common C simulation engine */
    int result_count = 0;
    SimulationResult **results = run_simulation(references, reference_count, frame_count, choice, &result_count);

    if (!results || result_count == 0) {
        gtk_label_set_markup(GTK_LABEL(app_win->status_label),
                             "<span foreground='#c62828'><b>Error:</b> Simulation execution failed to allocate or run.</span>");
        clear_results_view(app_win->results_view);
        free_parsed_references(references);
        return;
    }

    /* 7. Display results in results view */
    display_simulation_results(app_win->results_view, results, result_count);

    char success_msg[256];
    snprintf(success_msg, sizeof(success_msg),
             "<span foreground='#2e7d32'><b>Success:</b> Simulation completed (%s, %d references, %d frames).</span>",
             algorithm_choice_to_string(choice), reference_count, frame_count);
    gtk_label_set_markup(GTK_LABEL(app_win->status_label), success_msg);

    /* 8. Free allocated memory according to ownership contract */
    free_parsed_references(references);
    free_simulation_results(results, result_count);
}

void on_clear_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    AppMainWindow *app_win = (AppMainWindow *)user_data;
    if (!app_win) {
        return;
    }

    gtk_entry_set_text(GTK_ENTRY(app_win->reference_entry), "");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(app_win->frame_spin), 3);
    gtk_combo_box_set_active(GTK_COMBO_BOX(app_win->algo_combo), 0);
    gtk_label_set_text(GTK_LABEL(app_win->status_label),
                       "Ready. Enter reference string and frames, then click 'Run Simulation'.");

    if (app_win->results_view) {
        clear_results_view(app_win->results_view);
    }
}

AppMainWindow *create_main_window(GtkApplication *app) {
    AppMainWindow *app_win = (AppMainWindow *)safe_calloc(1, sizeof(AppMainWindow));
    if (!app_win) {
        return NULL;
    }

    /* 1. Window Setup */
    if (app) {
        app_win->window = gtk_application_window_new(app);
    } else {
        app_win->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    }

    gtk_window_set_title(GTK_WINDOW(app_win->window), "Page Replacement Algorithm Simulator");
    gtk_window_set_default_size(GTK_WINDOW(app_win->window), 920, 680);
    gtk_container_set_border_width(GTK_CONTAINER(app_win->window), 14);

    /* 2. Top-level vertical box */
    GtkWidget *main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(app_win->window), main_vbox);

    /* Header Title */
    GtkWidget *title_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title_label),
                         "<span size='x-large' weight='bold'>Simulation &amp; Comparative Analysis of Page Replacement Algorithms</span>");
    gtk_box_pack_start(GTK_BOX(main_vbox), title_label, FALSE, FALSE, 4);

    /* 3. Input Controls Grid in a Frame */
    GtkWidget *input_frame = gtk_frame_new("Simulation Configuration");
    gtk_box_pack_start(GTK_BOX(main_vbox), input_frame, FALSE, FALSE, 2);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 12);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);
    gtk_container_add(GTK_CONTAINER(input_frame), grid);

    /* Page-reference string input */
    GtkWidget *ref_label = gtk_label_new("Page Reference String:");
    gtk_widget_set_halign(ref_label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), ref_label, 0, 0, 1, 1);

    app_win->reference_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app_win->reference_entry), "e.g. 7 0 1 2 0 3 0 4 2 3");
    gtk_widget_set_hexpand(app_win->reference_entry, TRUE);
    gtk_grid_attach(GTK_GRID(grid), app_win->reference_entry, 1, 0, 3, 1);

    /* Enter key triggers simulation run */
    g_signal_connect_swapped(app_win->reference_entry, "activate", G_CALLBACK(on_run_simulation_clicked), app_win);

    /* Frame count input */
    GtkWidget *frame_label = gtk_label_new("Number of Frames:");
    gtk_widget_set_halign(frame_label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), frame_label, 0, 1, 1, 1);

    GtkAdjustment *adj = gtk_adjustment_new(3.0, (double)MIN_FRAMES, (double)MAX_FRAMES, 1.0, 1.0, 0.0);
    app_win->frame_spin = gtk_spin_button_new(adj, 1.0, 0);
    gtk_grid_attach(GTK_GRID(grid), app_win->frame_spin, 1, 1, 1, 1);

    /* Algorithm Selection combo */
    GtkWidget *algo_label = gtk_label_new("Algorithm Selection:");
    gtk_widget_set_halign(algo_label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), algo_label, 2, 1, 1, 1);

    app_win->algo_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app_win->algo_combo), "FIFO (First-In First-Out)");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app_win->algo_combo), "LRU (Least Recently Used)");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app_win->algo_combo), "Optimal (Belady's Min)");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app_win->algo_combo), "Compare All Algorithms");
    gtk_combo_box_set_active(GTK_COMBO_BOX(app_win->algo_combo), 0);
    gtk_grid_attach(GTK_GRID(grid), app_win->algo_combo, 3, 1, 1, 1);

    /* 4. Action Buttons Box */
    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(main_vbox), button_box, FALSE, FALSE, 2);

    app_win->run_button = gtk_button_new_with_label("Run Simulation");
    gtk_box_pack_start(GTK_BOX(button_box), app_win->run_button, FALSE, FALSE, 0);

    app_win->clear_button = gtk_button_new_with_label("Clear");
    gtk_box_pack_start(GTK_BOX(button_box), app_win->clear_button, FALSE, FALSE, 0);

    /* 5. Status / Error Message Label */
    app_win->status_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(app_win->status_label),
                         "Ready. Enter reference string and frame count, then click 'Run Simulation'.");
    gtk_widget_set_halign(app_win->status_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(main_vbox), app_win->status_label, FALSE, FALSE, 2);

    /* 6. Results Display Area (Scrolled) */
    app_win->results_view = create_results_view();
    if (app_win->results_view) {
        gtk_widget_set_vexpand(app_win->results_view->container, TRUE);
        gtk_box_pack_start(GTK_BOX(main_vbox), app_win->results_view->container, TRUE, TRUE, 2);
    }

    /* Signal bindings */
    g_signal_connect(app_win->run_button, "clicked", G_CALLBACK(on_run_simulation_clicked), app_win);
    g_signal_connect(app_win->clear_button, "clicked", G_CALLBACK(on_clear_clicked), app_win);

    return app_win;
}
