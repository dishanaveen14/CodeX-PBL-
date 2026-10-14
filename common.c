/* common.c - shared helper(s). Owner: Team Lead */
#include "common.h"

/* Append a block [start,end) for process pid (-1 = idle).
 * Zero/negative-length blocks are ignored. If the previous block has the
 * same pid and ends exactly at 'start', it is extended instead (merging). */
void gantt_add(Gantt *g, int pid, int start, int end)
{
    if (g == 0 || end <= start)
        return;

    if (g->count > 0) {
        GanttEntry *last = &g->e[g->count - 1];
        if (last->pid == pid && last->end == start) {
            last->end = end;
            return;
        }
    }
    if (g->count >= MAX_GANTT)
        return;                     /* chart full: drop (metrics are unaffected) */

    g->e[g->count].pid   = pid;
    g->e[g->count].start = start;
    g->e[g->count].end   = end;
    g->count++;
}
