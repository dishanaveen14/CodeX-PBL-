/**
 * @file results_view.c
 * @brief Implementation of GUI result presentation for page replacement simulation.
 *
 * Owned by Person 3.
 */

#include "gui/results_view.h"
#include "utils/memory.h"
#include <stdio.h>
#include <stdlib.h>

ResultsView *create_results_view(void) {
    ResultsView *view = (ResultsView *)safe_calloc(1, sizeof(ResultsView));
    if (!view) {
        return NULL;
    }

    view->container = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(view->container),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);

    view->content_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_container_set_border_width(GTK_CONTAINER(view->content_box), 12);
    gtk_container_add(GTK_CONTAINER(view->container), view->content_box);

    view->summary_label = gtk_label_new("Enter parameters and click 'Run Simulation' to view results.");
    gtk_box_pack_start(GTK_BOX(view->content_box), view->summary_label, FALSE, FALSE, 5);

    gtk_widget_show_all(view->container);
    return view;
}

void clear_results_view(ResultsView *view) {
    if (!view || !view->content_box) {
        return;
    }

    /* Remove dynamically generated result widgets, retaining only summary_label */
    GList *children = gtk_container_get_children(GTK_CONTAINER(view->content_box));
    for (GList *iter = children; iter != NULL; iter = g_list_next(iter)) {
        GtkWidget *widget = GTK_WIDGET(iter->data);
        if (widget != view->summary_label) {
            gtk_widget_destroy(widget);
        }
    }
    g_list_free(children);

    gtk_label_set_text(GTK_LABEL(view->summary_label), "Results cleared. Ready for next simulation.");
}

/**
 * @brief Helper to create a formatted statistics card widget for an algorithm result.
 */
static GtkWidget *create_statistics_card(const SimulationResult *result) {
    GtkWidget *frame = gtk_frame_new(NULL);
    char title_buf[64];
    snprintf(title_buf, sizeof(title_buf), "<b>%s Performance Metrics</b>", result->algorithm_name);

    GtkWidget *label_title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label_title), title_buf);
    gtk_frame_set_label_widget(GTK_FRAME(frame), label_title);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 6);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 16);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);

    /* Total References */
    GtkWidget *lbl_ref_title = gtk_label_new("Total References:");
    gtk_widget_set_halign(lbl_ref_title, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), lbl_ref_title, 0, 0, 1, 1);

    char val_buf[64];
    snprintf(val_buf, sizeof(val_buf), "<b>%d</b>", result->total_references);
    GtkWidget *lbl_ref_val = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_ref_val), val_buf);
    gtk_widget_set_halign(lbl_ref_val, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), lbl_ref_val, 1, 0, 1, 1);

    /* Page Hits */
    GtkWidget *lbl_hits_title = gtk_label_new("Page Hits:");
    gtk_widget_set_halign(lbl_hits_title, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), lbl_hits_title, 0, 1, 1, 1);

    snprintf(val_buf, sizeof(val_buf), "<span foreground='#2e7d32'><b>%d</b></span>", result->total_hits);
    GtkWidget *lbl_hits_val = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_hits_val), val_buf);
    gtk_widget_set_halign(lbl_hits_val, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), lbl_hits_val, 1, 1, 1, 1);

    /* Page Faults */
    GtkWidget *lbl_faults_title = gtk_label_new("Page Faults:");
    gtk_widget_set_halign(lbl_faults_title, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), lbl_faults_title, 2, 0, 1, 1);

    snprintf(val_buf, sizeof(val_buf), "<span foreground='#c62828'><b>%d</b></span>", result->total_faults);
    GtkWidget *lbl_faults_val = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_faults_val), val_buf);
    gtk_widget_set_halign(lbl_faults_val, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), lbl_faults_val, 3, 0, 1, 1);

    /* Hit Ratio */
    GtkWidget *lbl_hit_ratio_title = gtk_label_new("Hit Ratio:");
    gtk_widget_set_halign(lbl_hit_ratio_title, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), lbl_hit_ratio_title, 2, 1, 1, 1);

    snprintf(val_buf, sizeof(val_buf), "<span foreground='#2e7d32'><b>%.2f%%</b></span>", result->hit_ratio);
    GtkWidget *lbl_hit_ratio_val = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_hit_ratio_val), val_buf);
    gtk_widget_set_halign(lbl_hit_ratio_val, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), lbl_hit_ratio_val, 3, 1, 1, 1);

    /* Fault Ratio */
    GtkWidget *lbl_fault_ratio_title = gtk_label_new("Fault Ratio:");
    gtk_widget_set_halign(lbl_fault_ratio_title, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), lbl_fault_ratio_title, 4, 0, 1, 1);

    snprintf(val_buf, sizeof(val_buf), "<span foreground='#c62828'><b>%.2f%%</b></span>", result->fault_ratio);
    GtkWidget *lbl_fault_ratio_val = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_fault_ratio_val), val_buf);
    gtk_widget_set_halign(lbl_fault_ratio_val, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), lbl_fault_ratio_val, 5, 0, 1, 1);

    gtk_container_add(GTK_CONTAINER(frame), grid);
    return frame;
}

/**
 * @brief Helper to create the step-by-step frame contents table.
 */
static GtkWidget *create_step_table(const SimulationResult *result) {
    GtkWidget *frame = gtk_frame_new(NULL);
    char frame_title[64];
    snprintf(frame_title, sizeof(frame_title), "<b>%s Step-by-Step Frame States</b>", result->algorithm_name);

    GtkWidget *lbl_title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_title), frame_title);
    gtk_frame_set_label_widget(GTK_FRAME(frame), lbl_title);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 6);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 14);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);

    /* Table Column Headers */
    GtkWidget *hdr_page = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(hdr_page), "<b>Page</b>");
    gtk_grid_attach(GTK_GRID(grid), hdr_page, 0, 0, 1, 1);

    for (int f = 0; f < result->frame_count; ++f) {
        char col_name[32];
        snprintf(col_name, sizeof(col_name), "<b>Frame %d</b>", f + 1);
        GtkWidget *hdr_frame = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(hdr_frame), col_name);
        gtk_grid_attach(GTK_GRID(grid), hdr_frame, f + 1, 0, 1, 1);
    }

    GtkWidget *hdr_status = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(hdr_status), "<b>Status</b>");
    gtk_grid_attach(GTK_GRID(grid), hdr_status, result->frame_count + 1, 0, 1, 1);

    /* Separator below headers */
    GtkWidget *sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_grid_attach(GTK_GRID(grid), sep, 0, 1, result->frame_count + 2, 1);

    /* Populate Rows for each step */
    for (int i = 0; i < result->total_references; ++i) {
        int row = i + 2;

        /* Page requested */
        char page_str[16];
        snprintf(page_str, sizeof(page_str), "%d", result->steps[i].page);
        GtkWidget *lbl_p = gtk_label_new(page_str);
        gtk_grid_attach(GTK_GRID(grid), lbl_p, 0, row, 1, 1);

        /* Frame contents */
        for (int f = 0; f < result->frame_count; ++f) {
            char f_str[16];
            if (result->steps[i].frames[f] == EMPTY_FRAME) {
                snprintf(f_str, sizeof(f_str), "—");
            } else {
                snprintf(f_str, sizeof(f_str), "%d", result->steps[i].frames[f]);
            }
            GtkWidget *lbl_f = gtk_label_new(f_str);
            gtk_grid_attach(GTK_GRID(grid), lbl_f, f + 1, row, 1, 1);
        }

        /* Status: Hit (Green) / Fault (Red) */
        GtkWidget *lbl_st = gtk_label_new(NULL);
        if (result->steps[i].status == PAGE_HIT) {
            gtk_label_set_markup(GTK_LABEL(lbl_st), "<span foreground='#2e7d32' weight='bold'>Hit</span>");
        } else {
            gtk_label_set_markup(GTK_LABEL(lbl_st), "<span foreground='#c62828' weight='bold'>Fault</span>");
        }
        gtk_grid_attach(GTK_GRID(grid), lbl_st, result->frame_count + 1, row, 1, 1);
    }

    gtk_container_add(GTK_CONTAINER(frame), grid);
    return frame;
}

/**
 * @brief Helper to create the comparative analysis table for Compare All mode.
 */
static GtkWidget *create_comparison_table(SimulationResult **results, int result_count) {
    GtkWidget *frame = gtk_frame_new(NULL);
    GtkWidget *lbl_title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_title), "<span size='large' weight='bold'>Comparative Performance Summary</span>");
    gtk_frame_set_label_widget(GTK_FRAME(frame), lbl_title);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 18);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 12);

    /* Headers */
    GtkWidget *h_algo = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(h_algo), "<b>Algorithm</b>");
    gtk_widget_set_halign(h_algo, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), h_algo, 0, 0, 1, 1);

    GtkWidget *h_refs = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(h_refs), "<b>Total References</b>");
    gtk_grid_attach(GTK_GRID(grid), h_refs, 1, 0, 1, 1);

    GtkWidget *h_hits = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(h_hits), "<b>Page Hits</b>");
    gtk_grid_attach(GTK_GRID(grid), h_hits, 2, 0, 1, 1);

    GtkWidget *h_faults = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(h_faults), "<b>Page Faults</b>");
    gtk_grid_attach(GTK_GRID(grid), h_faults, 3, 0, 1, 1);

    GtkWidget *h_hr = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(h_hr), "<b>Hit Ratio</b>");
    gtk_grid_attach(GTK_GRID(grid), h_hr, 4, 0, 1, 1);

    GtkWidget *h_fr = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(h_fr), "<b>Fault Ratio</b>");
    gtk_grid_attach(GTK_GRID(grid), h_fr, 5, 0, 1, 1);

    GtkWidget *sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_grid_attach(GTK_GRID(grid), sep, 0, 1, 6, 1);

    for (int r = 0; r < result_count; ++r) {
        if (!results[r]) continue;
        int row = r + 2;

        /* Algorithm Name */
        char buf[64];
        snprintf(buf, sizeof(buf), "<b>%s</b>", results[r]->algorithm_name);
        GtkWidget *l_algo = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(l_algo), buf);
        gtk_widget_set_halign(l_algo, GTK_ALIGN_START);
        gtk_grid_attach(GTK_GRID(grid), l_algo, 0, row, 1, 1);

        /* Total References */
        snprintf(buf, sizeof(buf), "%d", results[r]->total_references);
        GtkWidget *l_refs = gtk_label_new(buf);
        gtk_grid_attach(GTK_GRID(grid), l_refs, 1, row, 1, 1);

        /* Hits */
        snprintf(buf, sizeof(buf), "<span foreground='#2e7d32'><b>%d</b></span>", results[r]->total_hits);
        GtkWidget *l_hits = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(l_hits), buf);
        gtk_grid_attach(GTK_GRID(grid), l_hits, 2, row, 1, 1);

        /* Faults */
        snprintf(buf, sizeof(buf), "<span foreground='#c62828'><b>%d</b></span>", results[r]->total_faults);
        GtkWidget *l_faults = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(l_faults), buf);
        gtk_grid_attach(GTK_GRID(grid), l_faults, 3, row, 1, 1);

        /* Hit Ratio */
        snprintf(buf, sizeof(buf), "<span foreground='#2e7d32'><b>%.2f%%</b></span>", results[r]->hit_ratio);
        GtkWidget *l_hr = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(l_hr), buf);
        gtk_grid_attach(GTK_GRID(grid), l_hr, 4, row, 1, 1);

        /* Fault Ratio */
        snprintf(buf, sizeof(buf), "<span foreground='#c62828'><b>%.2f%%</b></span>", results[r]->fault_ratio);
        GtkWidget *l_fr = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(l_fr), buf);
        gtk_grid_attach(GTK_GRID(grid), l_fr, 5, row, 1, 1);
    }

    gtk_container_add(GTK_CONTAINER(frame), grid);
    return frame;
}

void display_simulation_results(
    ResultsView *view,
    SimulationResult **results,
    int result_count
) {
    if (!view || !results || result_count <= 0) {
        return;
    }

    clear_results_view(view);

    if (result_count == 1 && results[0]) {
        /* Single algorithm result view */
        char quick_buf[128];
        snprintf(quick_buf, sizeof(quick_buf),
                 "Showing results for <b>%s</b> algorithm (%d references, %d frames):",
                 results[0]->algorithm_name,
                 results[0]->total_references,
                 results[0]->frame_count);
        gtk_label_set_markup(GTK_LABEL(view->summary_label), quick_buf);

        /* 1. Statistics Summary Card */
        GtkWidget *card = create_statistics_card(results[0]);
        gtk_box_pack_start(GTK_BOX(view->content_box), card, FALSE, FALSE, 0);

        /* 2. Step-by-Step Frame Table */
        GtkWidget *table = create_step_table(results[0]);
        gtk_box_pack_start(GTK_BOX(view->content_box), table, FALSE, FALSE, 6);
    } else {
        /* Comparative view for multiple algorithms (Compare All) */
        char cmp_buf[128];
        snprintf(cmp_buf, sizeof(cmp_buf),
                 "Comparison of <b>%d algorithms</b> on reference string (%d references, %d frames):",
                 result_count,
                 results[0] ? results[0]->total_references : 0,
                 results[0] ? results[0]->frame_count : 0);
        gtk_label_set_markup(GTK_LABEL(view->summary_label), cmp_buf);

        /* 1. Comparison Summary Table */
        GtkWidget *comp_table = create_comparison_table(results, result_count);
        gtk_box_pack_start(GTK_BOX(view->content_box), comp_table, FALSE, FALSE, 4);

        /* 2. Tabbed detail views for each algorithm */
        GtkWidget *notebook = gtk_notebook_new();
        gtk_box_pack_start(GTK_BOX(view->content_box), notebook, TRUE, TRUE, 6);

        for (int r = 0; r < result_count; ++r) {
            if (!results[r]) continue;

            GtkWidget *tab_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
            gtk_container_set_border_width(GTK_CONTAINER(tab_vbox), 8);

            GtkWidget *card = create_statistics_card(results[r]);
            gtk_box_pack_start(GTK_BOX(tab_vbox), card, FALSE, FALSE, 0);

            GtkWidget *step_tbl = create_step_table(results[r]);
            gtk_box_pack_start(GTK_BOX(tab_vbox), step_tbl, FALSE, FALSE, 0);

            char tab_title[64];
            snprintf(tab_title, sizeof(tab_title), "%s Details", results[r]->algorithm_name);
            GtkWidget *tab_lbl = gtk_label_new(tab_title);

            gtk_notebook_append_page(GTK_NOTEBOOK(notebook), tab_vbox, tab_lbl);
        }
    }

    gtk_widget_show_all(view->container);
}

void free_results_view(ResultsView *view) {
    if (view) {
        free(view);
    }
}
