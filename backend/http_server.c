#include "http_server.h"
#include "api.h"
#include "player.h"
#include "sqlite3.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <sys/stat.h>

/* strdup is not part of the C11 standard; provide a portable wrapper. */
static char *strdup(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s) + 1;
    char *p = (char *)malloc(n);
    if (p) memcpy(p, s, n);
    return p;
}

#pragma comment(lib, "ws2_32.lib")

#define MAX_REQUEST_SIZE 16384
#define MAX_RESPONSE_SIZE 32768

/* MIME type mapping */
static const char *get_mime_type(const char *path) {
    const char *ext = strrchr(path, '.');
    if (!ext) return "application/octet-stream";
    if (strcmp(ext, ".html") == 0) return "text/html; charset=utf-8";
    if (strcmp(ext, ".css") == 0) return "text/css; charset=utf-8";
    if (strcmp(ext, ".js") == 0) return "application/javascript; charset=utf-8";
    if (strcmp(ext, ".json") == 0) return "application/json; charset=utf-8";
    if (strcmp(ext, ".png") == 0) return "image/png";
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, ".svg") == 0) return "image/svg+xml";
    if (strcmp(ext, ".ico") == 0) return "image/x-icon";
    if (strcmp(ext, ".woff") == 0) return "font/woff";
    if (strcmp(ext, ".woff2") == 0) return "font/woff2";
    if (strcmp(ext, ".ttf") == 0) return "font/ttf";
    if (strcmp(ext, ".eot") == 0) return "application/vnd.ms-fontobject";
    if (strcmp(ext, ".map") == 0) return "application/json";
    return "application/octet-stream";
}

/* Serve static file from public directory */
static int serve_static_file(SOCKET client, const char *path) {
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "public%s", path);

    /* Prevent directory traversal */
    if (strstr(full_path, "..")) return 0;

    FILE *fp = fopen(full_path, "rb");
    if (!fp) return 0;

    struct stat st;
    stat(full_path, &st);

    const char *mime = get_mime_type(full_path);
    char header[512];
    sprintf(header,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %lld\r\n"
        "Cache-Control: public, max-age=31536000\r\n"
        "Connection: close\r\n"
        "\r\n",
        mime, (long long)st.st_size);
    send(client, header, (int)strlen(header), 0);

    char buffer[8192];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        send(client, buffer, (int)bytes, 0);
    }
    fclose(fp);
    return 1;
}

/* Forward declaration */
static void send_response(SOCKET client, int status, const char *content_type, const char *body);

/* Serve index.html for SPA routes */
static void serve_spa_index(SOCKET client) {
    FILE *fp = fopen("public/index.html", "rb");
    if (!fp) {
        send_response(client, 404, "text/plain", "Frontend not built");
        return;
    }
    struct stat st;
    stat("public/index.html", &st);
    char header[512];
    sprintf(header,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Content-Length: %lld\r\n"
        "Cache-Control: no-cache\r\n"
        "Connection: close\r\n"
        "\r\n",
        (long long)st.st_size);
    send(client, header, (int)strlen(header), 0);

    char buffer[8192];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        send(client, buffer, (int)bytes, 0);
    }
    fclose(fp);
}

static SOCKET server_socket = INVALID_SOCKET;
static volatile int server_running = 0;
static AppState *g_app = NULL;

/* Simple JSON builder helpers */
static char *json_strdup(const char *s) {
    if (!s) return strdup("null");
    return strdup(s);
}

static char *json_escape(const char *src) {
    if (!src) return strdup("\"\"");
    size_t len = strlen(src);
    char *dst = (char *)malloc(len * 2 + 3);
    char *p = dst;
    *p++ = '"';
    for (size_t i = 0; i < len; i++) {
        unsigned char c = src[i];
        if (c == '"' || c == '\\') {
            *p++ = '\\';
            *p++ = (char)c;
        } else if (c == '\n') {
            *p++ = '\\'; *p++ = 'n';
        } else if (c == '\r') {
            *p++ = '\\'; *p++ = 'r';
        } else if (c == '\t') {
            *p++ = '\\'; *p++ = 't';
        } else if (c < 32) {
            sprintf(p, "\\u%04x", c);
            p += 6;
        } else {
            *p++ = (char)c;
        }
    }
    *p++ = '"';
    *p = '\0';
    return dst;
}

static char *json_int(long long v) {
    char *buf = (char *)malloc(32);
    sprintf(buf, "%lld", v);
    return buf;
}

static char *json_double(double v) {
    char *buf = (char *)malloc(64);
    sprintf(buf, "%.2f", v);
    return buf;
}

/* Build JSON response for player */
static char *build_player_json(const Player *p, long long rank) {
    char *name = json_escape(p->name);
    char *id = json_int(p->id);
    char *score = json_int(p->score);
    char *rank_str = json_int(rank);
    char *created = json_int((long long)p->created_at);
    char *updated = json_int((long long)p->updated_at);

    char *result = (char *)malloc(512);
    sprintf(result,
        "{\"id\":%s,\"name\":%s,\"score\":%s,\"rank\":%s,\"created_at\":%s,\"updated_at\":%s}",
        id, name, score, rank_str, created, updated);

    free(name); free(id); free(score); free(rank_str); free(created); free(updated);
    return result;
}

static char *build_players_array_json(Player **players, long long count, int include_rank) {
    if (count == 0) return strdup("[]");
    size_t total = 2;
    for (long long i = 0; i < count; i++) {
        char *pj = build_player_json(players[i], include_rank ? i + 1 : 0);
        total += strlen(pj) + 1;
        free(pj);
    }
    char *result = (char *)malloc(total + 1);
    char *p = result;
    *p++ = '[';
    for (long long i = 0; i < count; i++) {
        char *pj = build_player_json(players[i], include_rank ? i + 1 : 0);
        strcpy(p, pj);
        p += strlen(pj);
        if (i < count - 1) *p++ = ',';
        free(pj);
    }
    *p++ = ']';
    *p = '\0';
    return result;
}

static char *build_error_json(int code, const char *msg) {
    char *m = json_escape(msg);
    char *c = json_int(code);
    char *result = (char *)malloc(256);
    sprintf(result, "{\"error\":{\"code\":%s,\"message\":%s}}", c, m);
    free(m); free(c);
    return result;
}

static char *build_success_json(const char *key, const char *value) {
    char *k = json_escape(key);
    char *v = json_escape(value);
    char *result = (char *)malloc(256);
    sprintf(result, "{\"%s\":%s}", k + 1, v);  /* skip first quote */
    /* Fix: k includes quotes */
    sprintf(result, "{\"%.*s\":%s}", (int)strlen(k) - 2, k + 1, v);
    free(k); free(v);
    return result;
}

static char *build_stats_json(long long total, long long highest, double avg) {
    char *t = json_int(total);
    char *h = json_int(highest);
    char *a = json_double(avg);
    char *result = (char *)malloc(256);
    sprintf(result, "{\"total_players\":%s,\"highest_score\":%s,\"average_score\":%s}", t, h, a);
    free(t); free(h); free(a);
    return result;
}

static char *build_activity_json(LLNode **nodes, long long count) {
    if (count == 0) return strdup("[]");
    size_t total = 2;
    char **items = (char **)malloc(count * sizeof(char *));
    for (long long i = 0; i < count; i++) {
        LLNode *n = nodes[i];
        char *op_str;
        switch (n->op_type) {
            case LL_OP_ADD_POINTS: op_str = "add_points"; break;
            case LL_OP_DEDUCT_POINTS: op_str = "deduct_points"; break;
            case LL_OP_SET_SCORE: op_str = "set_score"; break;
            case LL_OP_CREATE_PLAYER: op_str = "create_player"; break;
            case LL_OP_DELETE_PLAYER: op_str = "delete_player"; break;
            default: op_str = "unknown"; break;
        }
        char *name = json_escape(n->player_name);
        char *pid = json_int(n->player_id);
        char *old_s = json_int(n->old_score);
        char *new_s = json_int(n->new_score);
        char *op = json_escape(op_str);
        char *ts = json_int((long long)n->timestamp);
        items[i] = (char *)malloc(512);
        sprintf(items[i], "{\"player_id\":%s,\"player_name\":%s,\"operation\":%s,\"old_score\":%s,\"new_score\":%s,\"timestamp\":%s}",
            pid, name, op, old_s, new_s, ts);
        total += strlen(items[i]) + 1;
        free(name); free(pid); free(old_s); free(new_s); free(op); free(ts);
    }
    char *result = (char *)malloc(total + 1);
    char *p = result;
    *p++ = '[';
    for (long long i = 0; i < count; i++) {
        strcpy(p, items[i]);
        p += strlen(items[i]);
        if (i < count - 1) *p++ = ',';
        free(items[i]);
    }
    *p++ = ']';
    *p = '\0';
    free(items);
    return result;
}

/* Send HTTP response */
static void send_response(SOCKET client, int status, const char *content_type, const char *body) {
    char header[512];
    int body_len = body ? (int)strlen(body) : 0;
    sprintf(header,
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %d\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Connection: close\r\n"
        "\r\n",
        status,
        (status == 200) ? "OK" : (status == 201) ? "Created" : (status == 400) ? "Bad Request" :
        (status == 404) ? "Not Found" : (status == 409) ? "Conflict" : "Internal Server Error",
        content_type, body_len);
    send(client, header, (int)strlen(header), 0);
    if (body_len > 0) send(client, body, body_len, 0);
}

/* Parse JSON-like body for simple key-value pairs */
static char *extract_json_value(const char *body, const char *key) {
    if (!body || !key) return NULL;
    char pattern[128];
    sprintf(pattern, "\"%s\"", key);
    const char *p = strstr(body, pattern);
    if (!p) return NULL;
    p += strlen(pattern);
    while (*p && isspace(*p)) p++;
    if (*p != ':') return NULL;
    p++;
    while (*p && isspace(*p)) p++;
    if (*p == '"') {
        p++;
        const char *start = p;
        while (*p && *p != '"') p++;
        if (*p != '"') return NULL;
        size_t len = p - start;
        char *val = (char *)malloc(len + 1);
        strncpy(val, start, len);
        val[len] = '\0';
        return val;
    } else if (isdigit(*p) || *p == '-') {
        const char *start = p;
        while (*p && (isdigit(*p) || *p == '-')) p++;
        size_t len = p - start;
        char *val = (char *)malloc(len + 1);
        strncpy(val, start, len);
        val[len] = '\0';
        return val;
    }
    return NULL;
}

static long long extract_json_int(const char *body, const char *key) {
    char *s = extract_json_value(body, key);
    long long v = s ? atoll(s) : 0;
    free(s);
    return v;
}

/* Route handlers */
static void handle_get_players(SOCKET client, const char *query) {
    long long limit = 100;
    if (query) {
        const char *l = strstr(query, "limit=");
        if (l) limit = atoll(l + 6);
    }
    Player *players = NULL;
    long long count = 0;
    if (app_list_players(g_app, &players, &count) != 0) {
        send_response(client, 500, "application/json", build_error_json(500, "Failed to list players"));
        return;
    }
    long long n = count < limit ? count : limit;
    Player **ptrs = (Player **)malloc(n * sizeof(Player *));
    for (long long i = 0; i < n; i++) ptrs[i] = &players[i];
    char *json = build_players_array_json(ptrs, n, 1);
    send_response(client, 200, "application/json", json);
    free(json);
    free(ptrs);
    free(players);
}

static void handle_get_player(SOCKET client, long long id) {
    Player p;
    if (app_get_player(g_app, id, &p) != 0) {
        send_response(client, 404, "application/json", build_error_json(404, "Player not found"));
        return;
    }
    long long rank = app_get_rank(g_app, id);
    char *json = build_player_json(&p, rank);
    send_response(client, 200, "application/json", json);
    free(json);
}

static void handle_post_player(SOCKET client, const char *body) {
    char *name = extract_json_value(body, "name");
    if (!name || strlen(name) == 0) {
        free(name);
        send_response(client, 400, "application/json", build_error_json(400, "Name is required"));
        return;
    }
    Player p;
    if (app_create_player(g_app, name, &p) != 0) {
        free(name);
        send_response(client, 409, "application/json", build_error_json(409, "Player name already exists"));
        return;
    }
    free(name);
    long long rank = app_get_rank(g_app, p.id);
    char *json = build_player_json(&p, rank);
    send_response(client, 201, "application/json", json);
    free(json);
}

static void handle_delete_player(SOCKET client, long long id) {
    if (app_delete_player(g_app, id) != 0) {
        send_response(client, 404, "application/json", build_error_json(404, "Player not found"));
        return;
    }
    send_response(client, 200, "application/json", "{\"message\":\"Player deleted\"}");
}

static void handle_post_score(SOCKET client, const char *body, const char *action) {
    long long id = extract_json_int(body, "id");
    long long value = extract_json_int(body, "value");
    if (id <= 0 || value <= 0) {
        send_response(client, 400, "application/json", build_error_json(400, "Invalid id or value"));
        return;
    }
    int rc = 0;
    if (strcmp(action, "add") == 0) rc = app_add_points(g_app, id, value);
    else if (strcmp(action, "deduct") == 0) rc = app_deduct_points(g_app, id, value);
    else if (strcmp(action, "set") == 0) rc = app_set_score(g_app, id, value);
    if (rc != 0) {
        send_response(client, 404, "application/json", build_error_json(404, "Player not found or invalid operation"));
        return;
    }
    Player p;
    app_get_player(g_app, id, &p);
    long long rank = app_get_rank(g_app, id);
    char *json = build_player_json(&p, rank);
    send_response(client, 200, "application/json", json);
    free(json);
}

static void handle_post_undo(SOCKET client) {
    Player p;
    if (app_undo_last(g_app, &p) != 0) {
        send_response(client, 400, "application/json", build_error_json(400, "Nothing to undo"));
        return;
    }
    long long rank = app_get_rank(g_app, p.id);
    char *json = build_player_json(&p, rank);
    send_response(client, 200, "application/json", json);
    free(json);
}

static void handle_get_leaderboard(SOCKET client, const char *query) {
    long long limit = 10;
    if (query) {
        const char *l = strstr(query, "limit=");
        if (l) limit = atoll(l + 6);
    }
    Player *out[100];
    long long n = app_get_top(g_app, limit < 100 ? limit : 100, out);
    char *json = build_players_array_json(out, n, 1);
    send_response(client, 200, "application/json", json);
    free(json);
}

static void handle_get_stats(SOCKET client) {
    long long total, highest;
    double avg;
    app_get_stats(g_app, &total, &highest, &avg);
    char *json = build_stats_json(total, highest, avg);
    send_response(client, 200, "application/json", json);
    free(json);
}

static void handle_get_activity(SOCKET client, const char *query) {
    long long limit = 50;
    if (query) {
        const char *l = strstr(query, "limit=");
        if (l) limit = atoll(l + 6);
    }
    LLNode *nodes[100];
    long long n = app_get_recent_activity(g_app, limit < 100 ? limit : 100, nodes);
    char *json = build_activity_json(nodes, n);
    send_response(client, 200, "application/json", json);
    free(json);
}

static void handle_search(SOCKET client, const char *query) {
    char *q = NULL;
    if (query) {
        const char *qparam = strstr(query, "q=");
        if (qparam) {
            qparam += 2;
            const char *end = strchr(qparam, '&');
            size_t len = end ? end - qparam : strlen(qparam);
            q = (char *)malloc(len + 1);
            strncpy(q, qparam, len);
            q[len] = '\0';
            /* simple URL decode */
            for (size_t i = 0; i < len; i++) {
                if (q[i] == '+') q[i] = ' ';
            }
        }
    }
    if (!q || strlen(q) == 0) {
        free(q);
        send_response(client, 200, "application/json", "[]");
        return;
    }
    /* Search in hash table by name (O(n) - acceptable) */
    Player *found = NULL;
    for (int i = 0; i < HASH_SIZE; i++) {
        HashNode *cur = g_app->ht.table[i];
        while (cur) {
            if (strstr(cur->player->name, q)) {
                found = cur->player;
                break;
            }
            cur = cur->next;
        }
        if (found) break;
    }
    if (!found) {
        free(q);
        send_response(client, 200, "application/json", "[]");
        return;
    }
    long long rank = app_get_rank(g_app, found->id);
    char *json = build_player_json(found, rank);
    char *arr = (char *)malloc(strlen(json) + 3);
    sprintf(arr, "[%s]", json);
    send_response(client, 200, "application/json", arr);
    free(json);
    free(arr);
    free(q);
}

/* Main request handler */
static void handle_request(SOCKET client, const char *request) {
    /* Parse method and path */
    char method[16], path[256], version[16];
    sscanf(request, "%15s %255s %15s", method, path, version);

    /* Split path and query */
    char *query = strchr(path, '?');
    if (query) *query++ = '\0';

    /* Find body */
    const char *body = strstr(request, "\r\n\r\n");
    if (body) body += 4;

    printf("[%s] PATH='%s' QUERY='%s'\n", method, path, query ? query : "");
    fflush(stdout);

    if (strcmp(method, "OPTIONS") == 0) {
        send_response(client, 200, "text/plain", "");
        return;
    }

    if (strcmp(path, "/api/players") == 0 && strcmp(method, "GET") == 0) {
        handle_get_players(client, query);
    } else if (strncmp(path, "/api/players/", 13) == 0 && strcmp(method, "GET") == 0) {
        long long id = atoll(path + 13);
        handle_get_player(client, id);
    } else if (strcmp(path, "/api/players") == 0 && strcmp(method, "POST") == 0) {
        handle_post_player(client, body);
    } else if (strncmp(path, "/api/players/", 13) == 0 && strcmp(method, "DELETE") == 0) {
        long long id = atoll(path + 13);
        handle_delete_player(client, id);
    } else if (strcmp(path, "/api/score/add") == 0 && strcmp(method, "POST") == 0) {
        handle_post_score(client, body, "add");
    } else if (strcmp(path, "/api/score/deduct") == 0 && strcmp(method, "POST") == 0) {
        handle_post_score(client, body, "deduct");
    } else if (strcmp(path, "/api/score/set") == 0 && strcmp(method, "POST") == 0) {
        handle_post_score(client, body, "set");
    } else if (strcmp(path, "/api/score/undo") == 0 && strcmp(method, "POST") == 0) {
        handle_post_undo(client);
    } else if (strcmp(path, "/api/leaderboard") == 0 && strcmp(method, "GET") == 0) {
        handle_get_leaderboard(client, query);
    } else if (strcmp(path, "/api/stats") == 0 && strcmp(method, "GET") == 0) {
        handle_get_stats(client);
    } else if (strcmp(path, "/api/activity") == 0 && strcmp(method, "GET") == 0) {
        handle_get_activity(client, query);
    } else if (strcmp(path, "/api/search") == 0 && strcmp(method, "GET") == 0) {
        handle_search(client, query);
    } else if (strncmp(path, "/api/", 5) == 0) {
        send_response(client, 404, "application/json", build_error_json(404, "API endpoint not found"));
    } else {
        /* Serve static files or SPA index.html */
        if (strcmp(path, "/") == 0) {
            serve_spa_index(client);
        } else if (serve_static_file(client, path)) {
            /* served */
        } else {
            /* For SPA routes, serve index.html */
            serve_spa_index(client);
        }
    }
}

static DWORD WINAPI client_thread(LPVOID arg) {
    SOCKET client = (SOCKET)(uintptr_t)arg;
    char buffer[MAX_REQUEST_SIZE];
    int bytes = recv(client, buffer, MAX_REQUEST_SIZE - 1, 0);
    if (bytes > 0) {
        buffer[bytes] = '\0';
        handle_request(client, buffer);
    }
    closesocket(client);
    return 0;
}

int http_server_start(AppState *app, int port) {
    g_app = app;

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        fprintf(stderr, "WSAStartup failed\n");
        return -1;
    }

    server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == INVALID_SOCKET) {
        fprintf(stderr, "Socket creation failed\n");
        WSACleanup();
        return -1;
    }

    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {
        fprintf(stderr, "Bind failed: %d\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        return -1;
    }

    if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
        fprintf(stderr, "Listen failed\n");
        closesocket(server_socket);
        WSACleanup();
        return -1;
    }

    server_running = 1;
    printf("HTTP server started on port %d\n", port);

    while (server_running) {
        struct sockaddr_in client_addr;
        int addr_len = sizeof(client_addr);
        SOCKET client = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
        if (client == INVALID_SOCKET) {
            if (!server_running) break;
            continue;
        }
        HANDLE hThread = CreateThread(NULL, 0, client_thread, (LPVOID)(uintptr_t)client, 0, NULL);
        if (hThread) CloseHandle(hThread);
        else closesocket(client);
    }

    return 0;
}

void http_server_stop(void) {
    server_running = 0;
    if (server_socket != INVALID_SOCKET) {
        closesocket(server_socket);
        server_socket = INVALID_SOCKET;
    }
    WSACleanup();
}