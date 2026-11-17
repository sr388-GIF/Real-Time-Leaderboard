#include "database.h"
#include "linkedlist.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

/* Schema strings */
static const char *CREATE_PLAYERS =
    "CREATE TABLE IF NOT EXISTS players ("
    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "name TEXT NOT NULL UNIQUE,"
    "score INTEGER NOT NULL DEFAULT 0,"
    "created_at INTEGER NOT NULL,"
    "updated_at INTEGER NOT NULL"
    ");";

static const char *CREATE_HISTORY =
    "CREATE TABLE IF NOT EXISTS score_history ("
    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "player_id INTEGER NOT NULL,"
    "old_score INTEGER NOT NULL,"
    "new_score INTEGER NOT NULL,"
    "op_type INTEGER NOT NULL,"
    "timestamp INTEGER NOT NULL,"
    "FOREIGN KEY (player_id) REFERENCES players(id)"
    ");";

static const char *CREATE_INDEX_NAME = "CREATE INDEX IF NOT EXISTS idx_players_name ON players(name);";
static const char *CREATE_INDEX_SCORE = "CREATE INDEX IF NOT EXISTS idx_players_score ON players(score DESC);";

int db_init(Database *db, const char *path) {
    if (!db || !path) return -1;
    int rc = sqlite3_open(path, &db->db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db->db));
        return -1;
    }
    char *err = NULL;
    if (sqlite3_exec(db->db, CREATE_PLAYERS, NULL, NULL, &err) != SQLITE_OK) {
        fprintf(stderr, "Create players failed: %s\n", err);
        sqlite3_free(err);
        return -1;
    }
    if (sqlite3_exec(db->db, CREATE_HISTORY, NULL, NULL, &err) != SQLITE_OK) {
        fprintf(stderr, "Create history failed: %s\n", err);
        sqlite3_free(err);
        return -1;
    }
    sqlite3_exec(db->db, CREATE_INDEX_NAME, NULL, NULL, &err);
    sqlite3_exec(db->db, CREATE_INDEX_SCORE, NULL, NULL, &err);
    return 0;
}

void db_close(Database *db) {
    if (db && db->db) {
        sqlite3_close(db->db);
        db->db = NULL;
    }
}

/* Helper to populate Player from sqlite3_stmt */
static int player_from_stmt(sqlite3_stmt *stmt, Player *p) {
    p->id = sqlite3_column_int64(stmt, 0);
    const unsigned char *name = sqlite3_column_text(stmt, 1);
    strncpy(p->name, name ? (const char *)name : "", MAX_NAME_LEN - 1);
    p->name[MAX_NAME_LEN - 1] = '\0';
    p->score = sqlite3_column_int64(stmt, 2);
    p->created_at = (time_t)sqlite3_column_int64(stmt, 3);
    p->updated_at = (time_t)sqlite3_column_int64(stmt, 4);
    return 0;
}

int db_create_player(Database *db, const char *name, Player *out) {
    if (!db || !name || !out) return -1;
    time_t now = time(NULL);
    const char *sql = "INSERT INTO players (name, score, created_at, updated_at) VALUES (?, 0, ?, ?);";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 2, now);
    sqlite3_bind_int64(stmt, 3, now);
    int rc = sqlite3_step(stmt);
    long long id = sqlite3_last_insert_rowid(db->db);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) return -1;
    out->id = id;
    strncpy(out->name, name, MAX_NAME_LEN - 1);
    out->name[MAX_NAME_LEN - 1] = '\0';
    out->score = 0;
    out->created_at = now;
    out->updated_at = now;
    return 0;
}

int db_get_player(Database *db, long long id, Player *out) {
    if (!db || !out) return -1;
    const char *sql = "SELECT id, name, score, created_at, updated_at FROM players WHERE id = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    sqlite3_bind_int64(stmt, 1, id);
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        player_from_stmt(stmt, out);
        sqlite3_finalize(stmt);
        return 0;
    }
    sqlite3_finalize(stmt);
    return -1;
}

int db_get_player_by_name(Database *db, const char *name, Player *out) {
    if (!db || !name || !out) return -1;
    const char *sql = "SELECT id, name, score, created_at, updated_at FROM players WHERE name = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        player_from_stmt(stmt, out);
        sqlite3_finalize(stmt);
        return 0;
    }
    sqlite3_finalize(stmt);
    return -1;
}

int db_update_player(Database *db, const Player *p) {
    if (!db || !p) return -1;
    const char *sql = "UPDATE players SET score = ?, updated_at = ? WHERE id = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    sqlite3_bind_int64(stmt, 1, p->score);
    sqlite3_bind_int64(stmt, 2, p->updated_at);
    sqlite3_bind_int64(stmt, 3, p->id);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE) ? 0 : -1;
}

int db_delete_player(Database *db, long long id) {
    if (!db) return -1;
    const char *sql = "DELETE FROM players WHERE id = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    sqlite3_bind_int64(stmt, 1, id);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE) ? 0 : -1;
}

int db_list_all_players(Database *db, Player **out, long long *count) {
    if (!db || !out || !count) return -1;
    const char *sql = "SELECT id, name, score, created_at, updated_at FROM players ORDER BY score DESC, id ASC;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    long long cap = 64;
    long long n = 0;
    Player *arr = (Player *)malloc(cap * sizeof(Player));
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (n >= cap) {
            cap *= 2;
            arr = (Player *)realloc(arr, cap * sizeof(Player));
        }
        player_from_stmt(stmt, &arr[n++]);
    }
    sqlite3_finalize(stmt);
    *out = arr;
    *count = n;
    return 0;
}

/* Map LL_OpType to database op_type */
int ll_to_db_op(LL_OpType op) {
    switch (op) {
        case LL_OP_ADD_POINTS: return 0;
        case LL_OP_DEDUCT_POINTS: return 1;
        case LL_OP_SET_SCORE: return 2;
        case LL_OP_CREATE_PLAYER: return 3;
        case LL_OP_DELETE_PLAYER: return 4;
    }
    return 0;
}

int db_add_score_history(Database *db, long long player_id, long long old_score,
                         long long new_score, int op_type) {
    if (!db) return -1;
    const char *sql = "INSERT INTO score_history (player_id, old_score, new_score, op_type, timestamp) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    sqlite3_bind_int64(stmt, 1, player_id);
    sqlite3_bind_int64(stmt, 2, old_score);
    sqlite3_bind_int64(stmt, 3, new_score);
    sqlite3_bind_int(stmt, 4, op_type);
    sqlite3_bind_int64(stmt, 5, time(NULL));
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE) ? 0 : -1;
}

int db_get_score_history(Database *db, long long player_id, long long count, LLNode *out) {
    if (!db || !out) return -1;
    const char *sql = "SELECT player_id, old_score, new_score, op_type, timestamp FROM score_history WHERE player_id = ? ORDER BY timestamp DESC LIMIT ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    sqlite3_bind_int64(stmt, 1, player_id);
    sqlite3_bind_int64(stmt, 2, count);
    long long n = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && n < count) {
        out[n].player_id = sqlite3_column_int64(stmt, 0);
        out[n].old_score = sqlite3_column_int64(stmt, 1);
        out[n].new_score = sqlite3_column_int64(stmt, 2);
        out[n].op_type = (LL_OpType)sqlite3_column_int(stmt, 3);
        out[n].timestamp = (time_t)sqlite3_column_int64(stmt, 4);
        out[n].player_name[0] = '\0';
        n++;
    }
    sqlite3_finalize(stmt);
    return (int)n;
}

int db_get_total_players(Database *db, long long *out) {
    if (!db || !out) return -1;
    const char *sql = "SELECT COUNT(*) FROM players;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) *out = sqlite3_column_int64(stmt, 0);
    sqlite3_finalize(stmt);
    return 0;
}

int db_get_highest_score(Database *db, long long *out) {
    if (!db || !out) return -1;
    const char *sql = "SELECT MAX(score) FROM players;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        *out = sqlite3_column_int64(stmt, 0);
    } else {
        *out = 0;
    }
    sqlite3_finalize(stmt);
    return 0;
}

double db_get_average_score(Database *db) {
    if (!db) return 0.0;
    const char *sql = "SELECT AVG(score) FROM players;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL) != SQLITE_OK) return 0.0;
    double avg = 0.0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        avg = sqlite3_column_double(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return avg;
}