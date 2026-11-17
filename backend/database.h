#ifndef DATABASE_H
#define DATABASE_H

#include "player.h"
#include "linkedlist.h"
#include "sqlite3.h"

/* Database schema:
 *
 * CREATE TABLE players (
 *     id INTEGER PRIMARY KEY AUTOINCREMENT,
 *     name TEXT NOT NULL UNIQUE,
 *     score INTEGER NOT NULL DEFAULT 0,
 *     created_at INTEGER NOT NULL,      -- unix timestamp
 *     updated_at INTEGER NOT NULL       -- unix timestamp
 * );
 *
 * CREATE TABLE score_history (
 *     id INTEGER PRIMARY KEY AUTOINCREMENT,
 *     player_id INTEGER NOT NULL,
 *     old_score INTEGER NOT NULL,
 *     new_score INTEGER NOT NULL,
 *     op_type INTEGER NOT NULL,         -- 0=add, 1=deduct, 2=set, 3=create, 4=delete
 *     timestamp INTEGER NOT NULL,
 *     FOREIGN KEY (player_id) REFERENCES players(id)
 * );
 */

typedef struct {
    sqlite3 *db;
} Database;

int db_init(Database *db, const char *path);
void db_close(Database *db);

/* Player CRUD */
int db_create_player(Database *db, const char *name, Player *out);
int db_get_player(Database *db, long long id, Player *out);
int db_get_player_by_name(Database *db, const char *name, Player *out);
int db_update_player(Database *db, const Player *p);
int db_delete_player(Database *db, long long id);
int db_list_all_players(Database *db, Player **out, long long *count);

/* Score history */
int db_add_score_history(Database *db, long long player_id, long long old_score,
                         long long new_score, int op_type);
int db_get_score_history(Database *db, long long player_id, long long count,
                         LLNode *out);  /* We'll define a compatible struct */

/* Dashboard stats */
int db_get_total_players(Database *db, long long *out);
int db_get_highest_score(Database *db, long long *out);
double db_get_average_score(Database *db);

/* Convert LL_OpType to the integer stored in score_history.op_type */
int ll_to_db_op(LL_OpType op);

#endif