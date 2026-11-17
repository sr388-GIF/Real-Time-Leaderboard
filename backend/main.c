#include "api.h"
#include "http_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <windows.h>

static volatile int running = 1;

void signal_handler(int sig) {
    (void)sig;
    running = 0;
    http_server_stop();
}

int main(int argc, char *argv[]) {
    int port = 8080;
    const char *db_path = "e:/Real_Time_Leaderboard/database/leaderboard.db";

    if (argc > 1) port = atoi(argv[1]);
    if (argc > 2) db_path = argv[2];

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    AppState app;
    printf("Initializing Real-Time Leaderboard System...\n");
    printf("Database path: %s\n", db_path);
    printf("Port: %d\n", port);

    if (app_init(&app, db_path) != 0) {
        fprintf(stderr, "Failed to initialize application\n");
        return 1;
    }
    printf("Application initialized successfully.\n");

    printf("Starting HTTP server on port %d...\n", port);
    if (http_server_start(&app, port) != 0) {
        fprintf(stderr, "Failed to start HTTP server\n");
        app_destroy(&app);
        return 1;
    }

    app_destroy(&app);
    printf("Server stopped.\n");
    return 0;
}