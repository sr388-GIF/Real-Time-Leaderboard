#ifndef API_H
#define API_H

#include "player.h"
#include "avl.h"
#include "hashtable.h"
#include "maxheap.h"
#include "stack.h"
#include "queue.h"
#include "linkedlist.h"
#include "database.h"

/* Global application state */
typedef struct {
    AVLTree     avl;
    HashTable   ht;
    MaxHeap     heap;
    Stack       undo_stack;
    Queue       pending_queue;
    LinkedList  activity_log;
    Database    db;
} AppState;

/* Initialize all data structures and load from database */
int app_init(AppState *app, const char *db_path);

/* Clean up everything */
void app_destroy(AppState *app);

/* Player operations (sync all structures + DB) */
int app_create_player(AppState *app, const char *name, Player *out);
int app_get_player(AppState *app, long long id, Player *out);
int app_get_player_by_name(AppState *app, const char *name, Player *out);
int app_delete_player(AppState *app, long long id);
int app_list_players(AppState *app, Player **out, long long *count);

/* Score operations */
int app_add_points(AppState *app, long long id, long long points);
int app_deduct_points(AppState *app, long long id, long long points);
int app_set_score(AppState *app, long long id, long long new_score);
int app_undo_last(AppState *app, Player *out);

/* Leaderboard queries */
long long app_get_rank(AppState *app, long long id);
long long app_get_top(AppState *app, long long count, Player *out[]);

/* Dashboard stats */
int app_get_stats(AppState *app, long long *total_players, long long *highest_score, double *avg_score);

/* Process pending queue */
int app_process_queue(AppState *app);

/* Activity history */
long long app_get_recent_activity(AppState *app, long long count, LLNode *out[]);

#endif