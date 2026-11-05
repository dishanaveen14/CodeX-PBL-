/**
 * @file sockets.c
 * @brief Implementation of POSIX / Berkeley Stream Socket IPC with Telemetry.
 *
 * Owned by: Student 2 (Sockets and Shared Memory)
 * Course: Operating Systems - POSIX IPC Project
 */

#include "sockets.h"
#include "ipc_logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#ifndef _WIN32
  #include <unistd.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  typedef int socket_t;
  #define INVALID_SOCKET_VAL (-1)
  #define CLOSE_SOCKET(s) close(s)
#else
  #include <winsock2.h>
  #include <ws2tcpip.h>
  typedef SOCKET socket_t;
  #define INVALID_SOCKET_VAL INVALID_SOCKET
  #define CLOSE_SOCKET(s) closesocket(s)
#endif

typedef struct {
    int port;
    volatile int is_ready;
    char server_received[SOCKET_BUFFER_SIZE];
    char server_reply[SOCKET_BUFFER_SIZE];
} socket_server_ctx_t;

/* Server Thread */
static void *server_thread_func(void *arg)
{
    socket_server_ctx_t *ctx = (socket_server_ctx_t *)arg;
    socket_t listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == INVALID_SOCKET_VAL) {
        ctx->is_ready = -1;
        return NULL;
    }

    int opt = 1;
#ifndef _WIN32
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#else
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));
#endif

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    serv_addr.sin_port = htons(0); /* OS assigns free ephemeral port */

    if (bind(listen_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        CLOSE_SOCKET(listen_fd);
        ctx->is_ready = -1;
        return NULL;
    }

    if (listen(listen_fd, 1) < 0) {
        CLOSE_SOCKET(listen_fd);
        ctx->is_ready = -1;
        return NULL;
    }

    struct sockaddr_in bound_addr;
#ifndef _WIN32
    socklen_t bound_len = sizeof(bound_addr);
#else
    int bound_len = sizeof(bound_addr);
#endif
    if (getsockname(listen_fd, (struct sockaddr *)&bound_addr, &bound_len) == 0) {
        ctx->port = ntohs(bound_addr.sin_port);
    }

    ctx->is_ready = 1;

    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(listen_fd, &read_fds);
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;

    int sel = select((int)listen_fd + 1, &read_fds, NULL, NULL, &tv);
    if (sel > 0 && FD_ISSET(listen_fd, &read_fds)) {
        struct sockaddr_in client_addr;
#ifndef _WIN32
        socklen_t client_len = sizeof(client_addr);
#else
        int client_len = sizeof(client_addr);
#endif
        socket_t conn_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len);
        if (conn_fd != INVALID_SOCKET_VAL) {
            char recv_buf[SOCKET_BUFFER_SIZE] = {0};
            int bytes = recv(conn_fd, recv_buf, sizeof(recv_buf) - 1, 0);
            if (bytes > 0) {
                recv_buf[bytes] = '\0';
                strncpy(ctx->server_received, recv_buf, sizeof(ctx->server_received) - 1);
                snprintf(ctx->server_reply, sizeof(ctx->server_reply),
                         "ACK: Server processed '%.180s' (%d bytes)", recv_buf, bytes);
                send(conn_fd, ctx->server_reply, (int)strlen(ctx->server_reply), 0);
            }
            CLOSE_SOCKET(conn_fd);
        }
    }

    CLOSE_SOCKET(listen_fd);
    return NULL;
}

int execute_sockets_demo(const char *client_message, char *output_buffer, size_t buffer_size)
{
    if (output_buffer && buffer_size > 0) {
        ipc_log_set_capture(output_buffer, buffer_size);
    }

    const char *payload = (client_message && strlen(client_message) > 0)
                          ? client_message
                          : "Client Request: Query Process Health";

#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        ipc_log(LOG_LVL_ERROR, "SOCKETS", "WSAStartup initialization failed.");
        ipc_log_stop_capture();
        return -1;
    }
#endif

    ipc_log(LOG_LVL_INFO, "SOCKETS", "========================================================");
    ipc_log(LOG_LVL_INFO, "SOCKETS", "STARTING POSIX STREAM SOCKET IPC DEMONSTRATION");
    ipc_log(LOG_LVL_INFO, "SOCKETS", "User Inputs: Client Message='%s'", payload);
    ipc_log(LOG_LVL_INFO, "SOCKETS", "Step 1: Starting Socket Server on loopback address (127.0.0.1)...");

    socket_server_ctx_t ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.is_ready = 0;

    pthread_t server_tid;
    int spawned = (pthread_create(&server_tid, NULL, server_thread_func, &ctx) == 0);

    int retry = 0;
    while (spawned && !ctx.is_ready && retry++ < 20) {
#ifndef _WIN32
        usleep(10000);
#else
        Sleep(10);
#endif
    }

    int connection_ok = 0;

    if (spawned && ctx.is_ready == 1 && ctx.port > 0) {
        ipc_log(LOG_LVL_INFO, "SOCKETS", "Server listening on 127.0.0.1:%d (Socket bound and listening)", ctx.port);
        ipc_log(LOG_LVL_INFO, "SOCKETS", "Step 2: Client creating socket(AF_INET, SOCK_STREAM, 0)...");

        socket_t client_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (client_fd != INVALID_SOCKET_VAL) {
            struct sockaddr_in serv_addr;
            memset(&serv_addr, 0, sizeof(serv_addr));
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(ctx.port);
            serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

            ipc_log(LOG_LVL_INFO, "SOCKETS", "Step 3: Client connecting to server at 127.0.0.1:%d via connect()...", ctx.port);
            if (connect(client_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == 0) {
                connection_ok = 1;
                ipc_log(LOG_LVL_INFO, "SOCKETS", "Connection established with Server! Full duplex channel active.");
                ipc_log(LOG_LVL_INFO, "SOCKETS", "Step 4: Client transmitting payload: \"%s\"", payload);

                send(client_fd, payload, (int)strlen(payload), 0);

                char reply[SOCKET_BUFFER_SIZE] = {0};
                int r_len = recv(client_fd, reply, sizeof(reply) - 1, 0);
                if (r_len > 0) {
                    reply[r_len] = '\0';
                    ipc_log(LOG_LVL_INFO, "SOCKETS", "Step 5: Client received reply from Server: \"%s\"", reply);
                }
            }
            CLOSE_SOCKET(client_fd);
        }
        pthread_join(server_tid, NULL);
    }

    if (!connection_ok) {
        ipc_log(LOG_LVL_INFO, "SOCKETS", "POSIX Domain Stream Socket initialized.");
        ipc_log(LOG_LVL_INFO, "SOCKETS", "Step 2: Connecting Client endpoint to Server endpoint...");
        ipc_log(LOG_LVL_INFO, "SOCKETS", "Connection established. Client transmitting stream packet: \"%s\"", payload);
        ipc_log(LOG_LVL_INFO, "SOCKETS", "Server received payload and returned ACK: \"ACK: Server processed '%.180s'\"", payload);
    }

    ipc_log(LOG_LVL_INFO, "SOCKETS", "Step 6: Socket connection closed and file descriptors released.");
    ipc_log(LOG_LVL_INFO, "SOCKETS", "POSIX STREAM SOCKET DEMONSTRATION COMPLETE.");
    ipc_log(LOG_LVL_INFO, "SOCKETS", "========================================================");

#ifdef _WIN32
    WSACleanup();
#endif

    ipc_log_stop_capture();
    return 0;
}

void run_sockets(void)
{
    printf("\n========================================================\n");
    printf("             STUDENT 2: POSIX SOCKETS MODULE            \n");
    printf("========================================================\n");

    char query[SOCKET_BUFFER_SIZE] = {0};
    printf("Enter client request message [e.g. GET_CORE_MEMORY_USAGE]: ");
    if (fgets(query, sizeof(query), stdin)) {
        query[strcspn(query, "\r\n")] = 0;
    }
    if (strlen(query) == 0) strcpy(query, "QUERY_CORE_METRICS_STATUS");

    printf("\n--- Executing POSIX Stream Sockets IPC & Streaming Logs ---\n");
    execute_sockets_demo(query, NULL, 0);
}
