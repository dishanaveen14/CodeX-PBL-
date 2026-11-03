/**
 * @file panel_setup.c
 * @brief Person 1: Process and Resource Setup GUI Panel Implementation.
 * 
 * Deadlock Simulation System - PBL Project
 * Assigned to: Person 1 (Process and Resource Management)
 */

#include "../../include/gui/panel_setup.h"
#include "../../include/person1_process_resource/process_mgmt.h"
#include "../../include/person1_process_resource/resource_mgmt.h"
#include "../../include/person1_process_resource/matrix_mgmt.h"
#include "../../include/common/error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PANEL_SETUP_CLASS "DeadlockSimPanelSetup"

/* Control IDs */
enum {
    IDC_BTN_ADD_PROCESS = 1001,
    IDC_BTN_REMOVE_PROCESS,
    IDC_BTN_ADD_RESOURCE,
    IDC_BTN_LOAD_PRESET_SAFE,
    IDC_BTN_LOAD_PRESET_DEADLOCK,
    IDC_BTN_RESET_STATE,
    IDC_BTN_SET_ALLOC,
    IDC_BTN_SET_MAX,
    IDC_BTN_SUBMIT_REQ,
    IDC_BTN_RELEASE_RES,
    IDC_COMBO_PROC,
    IDC_COMBO_RES,
    IDC_EDIT_PROC_NAME,
    IDC_EDIT_RES_NAME,
    IDC_EDIT_RES_TOTAL,
    IDC_EDIT_ALLOC_VAL,
    IDC_EDIT_MAX_VAL,
    IDC_EDIT_REQ_VEC,
    IDC_EDIT_MATRIX_DISPLAY,
    IDC_STATIC_FEEDBACK
};

static SystemContext *g_setup_ctx = NULL;
static HWND g_hwnd_panel = NULL;
static HWND g_edit_proc_name = NULL;
static HWND g_edit_res_name = NULL;
static HWND g_edit_res_total = NULL;
static HWND g_combo_proc = NULL;
static HWND g_combo_res = NULL;
static HWND g_edit_alloc_val = NULL;
static HWND g_edit_max_val = NULL;
static HWND g_edit_req_vec = NULL;
static HWND g_edit_display = NULL;
static HWND g_static_feedback = NULL;

static void set_feedback(const char *msg, bool is_error) {
    if (g_static_feedback) {
        char buf[512];
        snprintf(buf, sizeof(buf), "[%s] %s", is_error ? "ERROR" : "INFO", msg);
        SetWindowText(g_static_feedback, buf);
    }
}

static void update_selectors(void) {
    if (!g_setup_ctx || !g_combo_proc || !g_combo_res) return;

    SystemState *st = &g_setup_ctx->live_state;

    /* Update Process Dropdown */
    SendMessage(g_combo_proc, CB_RESETCONTENT, 0, 0);
    for (int i = 0; i < st->process_count; i++) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%s (ID: %d)", st->processes[i].name, i);
        SendMessage(g_combo_proc, CB_ADDSTRING, 0, (LPARAM)buf);
    }
    if (st->process_count > 0) {
        SendMessage(g_combo_proc, CB_SETCURSEL, 0, 0);
    }

    /* Update Resource Dropdown */
    SendMessage(g_combo_res, CB_RESETCONTENT, 0, 0);
    for (int j = 0; j < st->resource_count; j++) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%s (Total: %d)", st->resources[j].name, st->total[j]);
        SendMessage(g_combo_res, CB_ADDSTRING, 0, (LPARAM)buf);
    }
    if (st->resource_count > 0) {
        SendMessage(g_combo_res, CB_SETCURSEL, 0, 0);
    }
}

static void render_matrix_text(void) {
    if (!g_setup_ctx || !g_edit_display) return;

    SystemState *st = &g_setup_ctx->live_state;
    char text[4096];
    int offset = 0;

    offset += snprintf(text + offset, sizeof(text) - offset,
        "========================================================================================\r\n"
        " SYSTEM STATE: Active Processes: %d / %d | Resource Types: %d / %d\r\n"
        "========================================================================================\r\n\r\n",
        st->process_count, MAX_PROCESSES, st->resource_count, MAX_RESOURCES);

    /* 1. Resources & Available Vector */
    offset += snprintf(text + offset, sizeof(text) - offset, "RESOURCE INVENTORY:\r\n");
    offset += snprintf(text + offset, sizeof(text) - offset, "  Resource:  ");
    for (int j = 0; j < st->resource_count; j++) {
        offset += snprintf(text + offset, sizeof(text) - offset, " %6s ", st->resources[j].name);
    }
    offset += snprintf(text + offset, sizeof(text) - offset, "\r\n  Total:     ");
    for (int j = 0; j < st->resource_count; j++) {
        offset += snprintf(text + offset, sizeof(text) - offset, " %6d ", st->total[j]);
    }
    offset += snprintf(text + offset, sizeof(text) - offset, "\r\n  Available: ");
    for (int j = 0; j < st->resource_count; j++) {
        offset += snprintf(text + offset, sizeof(text) - offset, " %6d ", st->available[j]);
    }
    offset += snprintf(text + offset, sizeof(text) - offset, "\r\n\r\n");

    /* 2. Allocation, Maximum, and Need Matrices Side by Side */
    offset += snprintf(text + offset, sizeof(text) - offset,
        "%-6s | %-18s | %-18s | %-18s | %-14s\r\n",
        "PID", "ALLOCATION", "MAXIMUM DEMAND", "NEED (Max - Alloc)", "REQUEST (Wait)");
    offset += snprintf(text + offset, sizeof(text) - offset,
        "----------------------------------------------------------------------------------------\r\n");

    for (int i = 0; i < st->process_count; i++) {
        offset += snprintf(text + offset, sizeof(text) - offset, "%-6s | ", st->processes[i].name);

        /* Allocation */
        char a_buf[64] = "";
        for (int j = 0; j < st->resource_count; j++) {
            char temp[16];
            snprintf(temp, sizeof(temp), "%d ", st->allocation[i][j]);
            strcat(a_buf, temp);
        }
        offset += snprintf(text + offset, sizeof(text) - offset, "%-18s | ", a_buf);

        /* Maximum */
        char m_buf[64] = "";
        for (int j = 0; j < st->resource_count; j++) {
            char temp[16];
            snprintf(temp, sizeof(temp), "%d ", st->maximum[i][j]);
            strcat(m_buf, temp);
        }
        offset += snprintf(text + offset, sizeof(text) - offset, "%-18s | ", m_buf);

        /* Need */
        char n_buf[64] = "";
        for (int j = 0; j < st->resource_count; j++) {
            char temp[16];
            snprintf(temp, sizeof(temp), "%d ", st->need[i][j]);
            strcat(n_buf, temp);
        }
        offset += snprintf(text + offset, sizeof(text) - offset, "%-18s | ", n_buf);

        /* Outstanding request */
        char r_buf[64] = "";
        for (int j = 0; j < st->resource_count; j++) {
            char temp[16];
            snprintf(temp, sizeof(temp), "%d ", st->outstanding_requests[i][j]);
            strcat(r_buf, temp);
        }
        offset += snprintf(text + offset, sizeof(text) - offset, "%-14s\r\n", r_buf);
    }

    if (st->process_count == 0) {
        offset += snprintf(text + offset, sizeof(text) - offset, "  (No processes created yet. Use controls on left to create or load preset.)\r\n");
    }

    SetWindowText(g_edit_display, text);
}

void panel_setup_refresh(HWND panel, const SystemContext *ctx) {
    (void)panel;
    if (ctx) {
        g_setup_ctx = (SystemContext*)ctx;
    }
    update_selectors();
    render_matrix_text();
}

static void on_add_process(void) {
    if (!g_setup_ctx) return;
    char name[MAX_NAME_LEN] = {0};
    GetWindowText(g_edit_proc_name, name, sizeof(name));

    int pid = -1;
    ErrorCode err = sys_process_create(&g_setup_ctx->live_state, (strlen(name) > 0) ? name : NULL, &pid);
    if (err == ERR_SUCCESS) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Created process '%s' (ID %d).", g_setup_ctx->live_state.processes[pid].name, pid);
        set_feedback(msg, false);
        SetWindowText(g_edit_proc_name, "");
        panel_setup_refresh(g_hwnd_panel, g_setup_ctx);
    } else {
        set_feedback(error_code_to_string(err), true);
    }
}

static void on_remove_process(void) {
    if (!g_setup_ctx) return;
    int cur_sel = (int)SendMessage(g_combo_proc, CB_GETCURSEL, 0, 0);
    if (cur_sel == CB_ERR || cur_sel < 0) {
        set_feedback("Select a process to remove.", true);
        return;
    }

    ErrorCode err = sys_process_remove(&g_setup_ctx->live_state, cur_sel);
    if (err == ERR_SUCCESS) {
        set_feedback("Process removed successfully. Allocated resources reclaimed.", false);
        panel_setup_refresh(g_hwnd_panel, g_setup_ctx);
    } else {
        set_feedback(error_code_to_string(err), true);
    }
}

static void on_add_resource(void) {
    if (!g_setup_ctx) return;
    char name[MAX_NAME_LEN] = {0};
    char total_str[32] = {0};
    GetWindowText(g_edit_res_name, name, sizeof(name));
    GetWindowText(g_edit_res_total, total_str, sizeof(total_str));

    int total_inst = atoi(total_str);
    if (total_inst <= 0 && strlen(total_str) > 0 && total_str[0] != '0') {
        set_feedback("Total instances must be a non-negative integer.", true);
        return;
    }

    int rid = -1;
    ErrorCode err = sys_resource_create(&g_setup_ctx->live_state, (strlen(name) > 0) ? name : NULL, total_inst, &rid);
    if (err == ERR_SUCCESS) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Created resource type '%s' with %d instances (ID %d).",
            g_setup_ctx->live_state.resources[rid].name, total_inst, rid);
        set_feedback(msg, false);
        SetWindowText(g_edit_res_name, "");
        SetWindowText(g_edit_res_total, "10");
        panel_setup_refresh(g_hwnd_panel, g_setup_ctx);
    } else {
        set_feedback(error_code_to_string(err), true);
    }
}

static void on_set_allocation(void) {
    if (!g_setup_ctx) return;
    int pid = (int)SendMessage(g_combo_proc, CB_GETCURSEL, 0, 0);
    int rid = (int)SendMessage(g_combo_res, CB_GETCURSEL, 0, 0);
    if (pid < 0 || rid < 0) {
        set_feedback("Select both a process and a resource first.", true);
        return;
    }

    char val_str[32] = {0};
    GetWindowText(g_edit_alloc_val, val_str, sizeof(val_str));
    int val = atoi(val_str);

    ErrorCode err = sys_matrix_set_allocation(&g_setup_ctx->live_state, pid, rid, val);
    if (err == ERR_SUCCESS) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Updated Allocation[%s][%s] = %d.",
            g_setup_ctx->live_state.processes[pid].name,
            g_setup_ctx->live_state.resources[rid].name, val);
        set_feedback(msg, false);
        panel_setup_refresh(g_hwnd_panel, g_setup_ctx);
    } else {
        set_feedback(error_code_to_string(err), true);
    }
}

static void on_set_maximum(void) {
    if (!g_setup_ctx) return;
    int pid = (int)SendMessage(g_combo_proc, CB_GETCURSEL, 0, 0);
    int rid = (int)SendMessage(g_combo_res, CB_GETCURSEL, 0, 0);
    if (pid < 0 || rid < 0) {
        set_feedback("Select both a process and a resource first.", true);
        return;
    }

    char val_str[32] = {0};
    GetWindowText(g_edit_max_val, val_str, sizeof(val_str));
    int val = atoi(val_str);

    ErrorCode err = sys_matrix_set_maximum(&g_setup_ctx->live_state, pid, rid, val);
    if (err == ERR_SUCCESS) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Updated Maximum[%s][%s] = %d.",
            g_setup_ctx->live_state.processes[pid].name,
            g_setup_ctx->live_state.resources[rid].name, val);
        set_feedback(msg, false);
        panel_setup_refresh(g_hwnd_panel, g_setup_ctx);
    } else {
        set_feedback(error_code_to_string(err), true);
    }
}

static void on_submit_request(void) {
    if (!g_setup_ctx) return;
    int pid = (int)SendMessage(g_combo_proc, CB_GETCURSEL, 0, 0);
    if (pid < 0) {
        set_feedback("Select a process to request resources for.", true);
        return;
    }

    SystemState *st = &g_setup_ctx->live_state;
    if (st->resource_count == 0) {
        set_feedback("No resources configured.", true);
        return;
    }

    char vec_str[128] = {0};
    GetWindowText(g_edit_req_vec, vec_str, sizeof(vec_str));

    int req[MAX_RESOURCES] = {0};
    char *token = strtok(vec_str, ", ");
    int idx = 0;
    while (token != NULL && idx < st->resource_count) {
        req[idx++] = atoi(token);
        token = strtok(NULL, ", ");
    }

    /* Preliminary validation check (Person 1 duty) */
    ErrorCode err = sys_request_validate_preliminary(st, pid, req);
    if (err != ERR_SUCCESS) {
        set_feedback(error_code_to_string(err), true);
        return;
    }

    /* Commit allocation */
    err = sys_allocation_apply(st, pid, req);
    if (err == ERR_SUCCESS) {
        set_feedback("Resource request validated and applied to live allocation state.", false);
        panel_setup_refresh(g_hwnd_panel, g_setup_ctx);
    } else {
        set_feedback(error_code_to_string(err), true);
    }
}

static void on_release_resources(void) {
    if (!g_setup_ctx) return;
    int pid = (int)SendMessage(g_combo_proc, CB_GETCURSEL, 0, 0);
    if (pid < 0) {
        set_feedback("Select a process to release resources from.", true);
        return;
    }

    ErrorCode err = sys_resources_release(&g_setup_ctx->live_state, pid, NULL);
    if (err == ERR_SUCCESS) {
        set_feedback("All held resources released back to Available vector.", false);
        panel_setup_refresh(g_hwnd_panel, g_setup_ctx);
    } else {
        set_feedback(error_code_to_string(err), true);
    }
}

static void on_load_preset(int preset_id) {
    if (!g_setup_ctx) return;
    ErrorCode err = sys_load_sample_configuration(&g_setup_ctx->live_state, preset_id);
    if (err == ERR_SUCCESS) {
        set_feedback(g_setup_ctx->live_state.status_message, false);
        panel_setup_refresh(g_hwnd_panel, g_setup_ctx);
    } else {
        set_feedback("Failed to load preset configuration.", true);
    }
}

static void on_reset_state(void) {
    if (!g_setup_ctx) return;
    system_state_reset(&g_setup_ctx->live_state);
    set_feedback("System state reset to empty configuration.", false);
    panel_setup_refresh(g_hwnd_panel, g_setup_ctx);
}

static LRESULT CALLBACK SetupPanelProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            switch (wmId) {
                case IDC_BTN_ADD_PROCESS:         on_add_process(); break;
                case IDC_BTN_REMOVE_PROCESS:      on_remove_process(); break;
                case IDC_BTN_ADD_RESOURCE:         on_add_resource(); break;
                case IDC_BTN_SET_ALLOC:            on_set_allocation(); break;
                case IDC_BTN_SET_MAX:              on_set_maximum(); break;
                case IDC_BTN_SUBMIT_REQ:           on_submit_request(); break;
                case IDC_BTN_RELEASE_RES:          on_release_resources(); break;
                case IDC_BTN_LOAD_PRESET_SAFE:     on_load_preset(0); break;
                case IDC_BTN_LOAD_PRESET_DEADLOCK: on_load_preset(1); break;
                case IDC_BTN_RESET_STATE:          on_reset_state(); break;
                default: break;
            }
            return 0;
        }

        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            SetBkMode(hdcStatic, OPAQUE);
            SetBkColor(hdcStatic, GetSysColor(COLOR_BTNFACE));
            SetTextColor(hdcStatic, GetSysColor(COLOR_WINDOWTEXT));
            return (LRESULT)GetSysColorBrush(COLOR_BTNFACE);
        }

        case WM_CTLCOLORBTN: {
            HDC hdcBtn = (HDC)wParam;
            SetBkMode(hdcBtn, OPAQUE);
            SetBkColor(hdcBtn, GetSysColor(COLOR_BTNFACE));
            return (LRESULT)GetSysColorBrush(COLOR_BTNFACE);
        }

        case WM_ERASEBKGND: {
            HDC hdc = (HDC)wParam;
            RECT rc;
            GetClientRect(hwnd, &rc);
            FillRect(hdc, &rc, GetSysColorBrush(COLOR_BTNFACE));
            return 1;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rect;
            GetClientRect(hwnd, &rect);
            FillRect(hdc, &rect, GetSysColorBrush(COLOR_BTNFACE));
            EndPaint(hwnd, &ps);
            return 0;
        }

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

static void register_class_once(HINSTANCE hInstance) {
    static bool registered = false;
    if (!registered) {
        WNDCLASSEX wc;
        memset(&wc, 0, sizeof(wc));
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = SetupPanelProc;
        wc.hInstance = hInstance;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
        wc.lpszClassName = PANEL_SETUP_CLASS;
        RegisterClassEx(&wc);
        registered = true;
    }
}

HWND panel_setup_create(HWND parent, SystemContext *ctx) {
    g_setup_ctx = ctx;
    HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(parent, GWLP_HINSTANCE);
    register_class_once(hInstance);

    HWND panel = CreateWindowEx(
        0,
        PANEL_SETUP_CLASS,
        NULL,
        WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        0, 0, 100, 100,
        parent,
        NULL,
        hInstance,
        NULL
    );
    g_hwnd_panel = panel;

    /* Left Column Controls (Width = 320) */

    /* Group 1: Process Creation */
    CreateWindow("BUTTON", "1. Process Management", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                 15, 10, 310, 85, panel, NULL, hInstance, NULL);

    CreateWindow("STATIC", "Name:", WS_CHILD | WS_VISIBLE, 25, 32, 45, 18, panel, NULL, hInstance, NULL);
    g_edit_proc_name = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                                      75, 30, 80, 22, panel, (HMENU)IDC_EDIT_PROC_NAME, hInstance, NULL);
    CreateWindow("BUTTON", "Add", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                 165, 30, 70, 22, panel, (HMENU)IDC_BTN_ADD_PROCESS, hInstance, NULL);
    CreateWindow("BUTTON", "Remove", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                 240, 30, 75, 22, panel, (HMENU)IDC_BTN_REMOVE_PROCESS, hInstance, NULL);
    CreateWindow("STATIC", "(Select in dropdown below to remove)", WS_CHILD | WS_VISIBLE,
                 25, 60, 280, 18, panel, NULL, hInstance, NULL);

    /* Group 2: Resource Creation */
    CreateWindow("BUTTON", "2. Resource Management", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                 15, 100, 310, 85, panel, NULL, hInstance, NULL);

    CreateWindow("STATIC", "Name:", WS_CHILD | WS_VISIBLE, 25, 122, 45, 18, panel, NULL, hInstance, NULL);
    g_edit_res_name = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                                     75, 120, 45, 22, panel, (HMENU)IDC_EDIT_RES_NAME, hInstance, NULL);
    CreateWindow("STATIC", "Total:", WS_CHILD | WS_VISIBLE, 130, 122, 40, 18, panel, NULL, hInstance, NULL);
    g_edit_res_total = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "10", WS_CHILD | WS_VISIBLE | ES_NUMBER,
                                      175, 120, 50, 22, panel, (HMENU)IDC_EDIT_RES_TOTAL, hInstance, NULL);
    CreateWindow("BUTTON", "Create", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                 235, 120, 80, 22, panel, (HMENU)IDC_BTN_ADD_RESOURCE, hInstance, NULL);
    CreateWindow("STATIC", "Auto-names: A, B, C... or custom", WS_CHILD | WS_VISIBLE,
                 25, 155, 280, 18, panel, NULL, hInstance, NULL);

    /* Group 3: Matrix Entry (Cell by Cell Editor) */
    CreateWindow("BUTTON", "3. Matrix Value Editor", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                 15, 190, 310, 145, panel, NULL, hInstance, NULL);

    CreateWindow("STATIC", "Process:", WS_CHILD | WS_VISIBLE, 25, 212, 60, 18, panel, NULL, hInstance, NULL);
    g_combo_proc = CreateWindow("COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
                                90, 210, 220, 150, panel, (HMENU)IDC_COMBO_PROC, hInstance, NULL);

    CreateWindow("STATIC", "Resource:", WS_CHILD | WS_VISIBLE, 25, 242, 60, 18, panel, NULL, hInstance, NULL);
    g_combo_res = CreateWindow("COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
                               90, 240, 220, 150, panel, (HMENU)IDC_COMBO_RES, hInstance, NULL);

    CreateWindow("STATIC", "Alloc Val:", WS_CHILD | WS_VISIBLE, 25, 272, 60, 18, panel, NULL, hInstance, NULL);
    g_edit_alloc_val = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "0", WS_CHILD | WS_VISIBLE | ES_NUMBER,
                                      90, 270, 50, 22, panel, (HMENU)IDC_EDIT_ALLOC_VAL, hInstance, NULL);
    CreateWindow("BUTTON", "Set Alloc", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                 145, 270, 80, 22, panel, (HMENU)IDC_BTN_SET_ALLOC, hInstance, NULL);

    CreateWindow("STATIC", "Max Val:", WS_CHILD | WS_VISIBLE, 25, 302, 60, 18, panel, NULL, hInstance, NULL);
    g_edit_max_val = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "0", WS_CHILD | WS_VISIBLE | ES_NUMBER,
                                    90, 300, 50, 22, panel, (HMENU)IDC_EDIT_MAX_VAL, hInstance, NULL);
    CreateWindow("BUTTON", "Set Max", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                 145, 300, 80, 22, panel, (HMENU)IDC_BTN_SET_MAX, hInstance, NULL);

    /* Group 4: Resource Request and Release */
    CreateWindow("BUTTON", "4. Resource Request && Release", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                 15, 340, 310, 115, panel, NULL, hInstance, NULL);

    CreateWindow("STATIC", "Req Vector (e.g. 1,0,2):", WS_CHILD | WS_VISIBLE | SS_NOPREFIX, 25, 362, 170, 18, panel, NULL, hInstance, NULL);
    g_edit_req_vec = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "0, 0, 0", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                                    25, 385, 285, 22, panel, (HMENU)IDC_EDIT_REQ_VEC, hInstance, NULL);

    CreateWindow("BUTTON", "Submit Request", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                 25, 415, 135, 24, panel, (HMENU)IDC_BTN_SUBMIT_REQ, hInstance, NULL);
    CreateWindow("BUTTON", "Release All", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                 175, 415, 135, 24, panel, (HMENU)IDC_BTN_RELEASE_RES, hInstance, NULL);

    /* Group 5: Sample Presets & Quick Actions */
    CreateWindow("BUTTON", "5. Quick Load Presets", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                 15, 460, 310, 120, panel, NULL, hInstance, NULL);

    CreateWindow("BUTTON", "Load 5-Process Safe Preset", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                 25, 485, 285, 24, panel, (HMENU)IDC_BTN_LOAD_PRESET_SAFE, hInstance, NULL);
    CreateWindow("BUTTON", "Load Deadlock Preset", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                 25, 515, 285, 24, panel, (HMENU)IDC_BTN_LOAD_PRESET_DEADLOCK, hInstance, NULL);
    CreateWindow("BUTTON", "Reset State to Empty", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                 25, 545, 285, 24, panel, (HMENU)IDC_BTN_RESET_STATE, hInstance, NULL);

    /* Right Column: Live Matrix & Vector Display (Width = remaining) */
    CreateWindow("STATIC", "Live Matrix && Vector Inspection (Auto-recalculated Need && Available):",
                 WS_CHILD | WS_VISIBLE | SS_NOPREFIX, 340, 10, 680, 20, panel, NULL, hInstance, NULL);

    /* Monospaced Font for Tables */
    HFONT hFont = CreateFont(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                             ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");

    g_edit_display = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        "EDIT",
        "",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
        340, 30, 690, 480,
        panel,
        (HMENU)IDC_EDIT_MATRIX_DISPLAY,
        hInstance,
        NULL
    );
    SendMessage(g_edit_display, WM_SETFONT, (WPARAM)hFont, TRUE);

    /* Bottom: Feedback / Validation message box */
    CreateWindow("BUTTON", "Validation & System Messages", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                 340, 515, 690, 65, panel, NULL, hInstance, NULL);

    g_static_feedback = CreateWindow(
        "STATIC",
        "[INFO] Person 1 Process & Resource Management ready. Load a preset or create processes.",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        355, 538, 660, 30,
        panel,
        (HMENU)IDC_STATIC_FEEDBACK,
        hInstance,
        NULL
    );

    /* Initial population */
    panel_setup_refresh(panel, ctx);

    return panel;
}
