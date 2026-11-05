#ifndef GUI_H
#define GUI_H

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Launch native Graphical User Interface (GUI) for the CPU Scheduler */
void launch_gui(Process initial_procs[], int initial_count);

#ifdef __cplusplus
}
#endif

#endif /* GUI_H */

