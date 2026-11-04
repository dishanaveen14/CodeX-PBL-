/**
 * @file results_view.c
 * @brief Implementation skeleton for results display view.
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

    view->content_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(view->content_box), 12);
    gtk_container_add(GTK_CONTAINER(view->container), view->content_box);

    view->summary_label = gtk_label_new("Enter parameters and click 'Run Simulation' to see results.");
    gtk_box_pack_start(GTK_BOX(view->content_box), view->summary_label, FALSE, FALSE, 5);

    gtk_widget_show_all(view->container);
    return view;
}

void clear_results_view(ResultsView *view) {
    if (!view || !view->content_box) {
        return;
    }

    /* Remove existing dynamically added child widgets */
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

void display_simulation_results(
    ResultsView *view,
    SimulationResult **results,
    int result_count
) {
    if (!view || !results || result_count <= 0) {
        return;
    }

    clear_results_view(view);

    /*
     * TODO [Person 3]: Implement complete GTK visualization:
     * 1. If result_count == 1:
     *    - Create a GtkGrid or GtkTreeView showing the step-by-step frame table:
     *      Columns: Page | Frame 1 | Frame 2 | ... | Frame N | Status (Hit/Fault)
     *    - Color code Hit (e.g. green) and Fault (e.g. red).
     *    - Display statistics box: Total References, Hits, Faults, Hit Ratio, Fault Ratio.
     * 2. If result_count > 1 (Compare All):
     *    - Display separate step tables for FIFO, LRU, and Optimal (or tabs/sections).
     *    - Display comparative summary table:
     *      Algorithm | Page Hits | Page Faults | Hit Ratio (%) | Fault Ratio (%)
     * 3. For now, display a placeholder text summary of the results to confirm pipeline works.
     */

    char summary_buffer[512];
    if (result_count == 1 && results[0]) {
        snprintf(summary_buffer, sizeof(summary_buffer),
                 "[%s Algorithm Result]\nTotal References: %d | Hits: %d | Faults: %d\nHit Ratio: %.2f%% | Fault Ratio: %.2f%%",
                 results[0]->algorithm_name,
                 results[0]->total_references,
                 results[0]->total_hits,
                 results[0]->total_faults,
                 results[0]->hit_ratio,
                 results[0]->fault_ratio);
    } else {
        snprintf(summary_buffer, sizeof(summary_buffer),
                 "[Comparison of %d Algorithms]\n(Detailed step-by-step and comparison tables to be rendered by Person 3)",
                 result_count);
    }

    gtk_label_set_text(GTK_LABEL(view->summary_label), summary_buffer);

    for (int r = 0; r < result_count; ++r) {
        if (!results[r]) continue;
        char header_text[64];
        snprintf(header_text, sizeof(header_text), "Algorithm: %s", results[r]->algorithm_name);
        GtkWidget *lbl = gtk_label_new(header_text);
        gtk_box_pack_start(GTK_BOX(view->content_box), lbl, FALSE, FALSE, 2);
        gtk_widget_show(lbl);
    }
}

void free_results_view(ResultsView *view) {
    if (view) {
        free(view);
    }
}

