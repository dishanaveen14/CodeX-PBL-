/**
 * @file results_view.h
 * @brief GUI result presentation module for frame tables and statistics.
 *
 * Owned by Person 3.
 */

#ifndef RESULTS_VIEW_H
#define RESULTS_VIEW_H

#include <gtk/gtk.h>
#include "core/simulation_types.h"

/**
 * @brief Encapsulates the results view container widgets.
 */
typedef struct {
    GtkWidget *container;        /**< Outer container (scrolled window or box) */
    GtkWidget *content_box;      /**< Inner box containing dynamically populated tables */
    GtkWidget *summary_label;    /**< Label displaying quick metrics or status */
} ResultsView;

/**
 * @brief Creates the initial results display widget container.
 *
 * @return Allocated ResultsView pointer.
 */
ResultsView *create_results_view(void);

/**
 * @brief Displays the step-by-step frame table and statistics for simulation results.
 *
 * Supports displaying either a single algorithm or comparative results for all algorithms.
 *
 * @param view Pointer to ResultsView.
 * @param results Array of SimulationResult pointers.
 * @param result_count Number of results in the array (1 or 3).
 */
void display_simulation_results(
    ResultsView *view,
    SimulationResult **results,
    int result_count
);

/**
 * @brief Clears all previous simulation results from the display view.
 *
 * @param view Pointer to ResultsView.
 */
void clear_results_view(ResultsView *view);

/**
 * @brief Frees the ResultsView wrapper structure.
 *
 * @param view Pointer to ResultsView.
 */
void free_results_view(ResultsView *view);

#endif /* RESULTS_VIEW_H */

