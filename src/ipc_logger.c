/**
 * @file ipc_logger.c
 * @brief Implementation of Unified IPC Logging to Console, File (sim.log), and GUI Buffer.
 */

#include "ipc_logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <pthread.h>

#ifdef _WIN32
  #include <windows.h>
#else
  #include <sys/time.h>
#endif

#define SIM_LOG_FILE "sim.log"

static pthread_mutex_t g_log_mutex = PTHREAD_MUTEX_INITIALIZER;
static char *g_capture_buf = NULL;
static size_t g_capture_max = 0;
static size_t g_capture_pos = 0;

void ipc_log_set_capture(char *buf, size_t max_size)
{
    pthread_mutex_lock(&g_log_mutex);
    g_capture_buf = buf;
    g_capture_max = max_size;
    g_capture_pos = 0;
    if (g_capture_buf && g_capture_max > 0) {
        g_capture_buf[0] = '\0';
    }
    pthread_mutex_unlock(&g_log_mutex);
}

void ipc_log_stop_capture(void)
{
    pthread_mutex_lock(&g_log_mutex);
    g_capture_buf = NULL;
    g_capture_max = 0;
    g_capture_pos = 0;
    pthread_mutex_unlock(&g_log_mutex);
}

static void get_timestamp_str(char *out, size_t out_len)
{
#ifdef _WIN32
    SYSTEMTIME st;
    GetLocalTime(&st);
    snprintf(out, out_len, "%04d-%02d-%02d %02d:%02d:%02d.%03d",
             st.wYear, st.wMonth, st.wDay,
             st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm tm_info;
    localtime_r(&tv.tv_sec, &tm_info);
    char date_buf[32];
    strftime(date_buf, sizeof(date_buf), "%Y-%m-%d %H:%M:%S", &tm_info);
    snprintf(out, out_len, "%s.%06ld", date_buf, (long)tv.tv_usec);
#endif
}

void ipc_log(const char *level, const char *module, const char *fmt, ...)
{
    pthread_mutex_lock(&g_log_mutex);

    char time_str[64];
    get_timestamp_str(time_str, sizeof(time_str));

    char msg_buf[1024];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msg_buf, sizeof(msg_buf), fmt, ap);
    va_end(ap);

    /* Formatted entry: YYYY-MM-DD HH:MM:SS.mmm [LEVEL] [MODULE  ] msg */
    char full_log[1200];
    snprintf(full_log, sizeof(full_log), "%s [%-5s] [%-8s] %s\n",
             time_str, level ? level : "INFO", module ? module : "CORE", msg_buf);

    /* 1. Print to Console */
    printf("%s", full_log);
    fflush(stdout);

    /* 2. Write to sim.log */
    FILE *fp = fopen(SIM_LOG_FILE, "a");
    if (fp) {
        fputs(full_log, fp);
        fclose(fp);
    }

    /* 3. Append to Capture Buffer (for GUI) */
    if (g_capture_buf && g_capture_max > 0) {
        size_t len = strlen(full_log);
        if (g_capture_pos + len < g_capture_max - 1) {
            memcpy(g_capture_buf + g_capture_pos, full_log, len);
            g_capture_pos += len;
            g_capture_buf[g_capture_pos] = '\0';
        }
    }

    pthread_mutex_unlock(&g_log_mutex);
}

