#include "api.h"
#include "player.h"
#include "database.h"
#include "linkedlist.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

int app_init(AppState *app, const char *db_path) {
    if (!app || !db_path) return -1;
    memset(app, 0, sizeof(AppState));
    avl_init(&app->avl);
    ht_init(&app->ht);
    mh_init(&app->heap);
    st_init(&app->undo_stack);
    q_init(&app->pending_queue);
    ll_init(&app->activity_log);
    if (db_init(&app->db, db_path) != 0) return -1;

    /* Load all players from database into memory structures */
    Player *players = NULL;
    long long count = 0;
    if (db_list_all_players(&app->db, &players, &count) == 0) {
        for (long long i = 0; i < count; i++) {
            avl_insert(&app->avl, &players[i]);
            ht_insert(&app->ht, &players[i]);
            mh_insert(&app->heap, &players[i]);
        }
        free(players);
    }
    return 0;
}

void app_destroy(AppState *app) {
    if (!app) return;
    avl_destroy(&app->avl);
    ht_destroy(&app->ht);
    mh_destroy(&app->heap);
    ll_destroy(&app->activity_log);
    db_close(&app->db);
}

/* Internal helper: apply a score change to all structures + DB.
 * If record_undo is true, push to undo stack (for forward operations only). */
static int apply_score_change(AppState *app, long long id, long long old_score,
                              long long new_score, LL_OpType op_type, int record_undo) {
    Player *p = ht_search(&app->ht, id);
    if (!p) return -1;
    p->score = new_score;
    p->updated_at = time(NULL);

    /* AVL: delete old position, insert new */
    avl_delete(&app->avl, id);
    avl_insert(&app->avl, p);

    /* Heap: update in place */
    mh_update_score(&app->heap, id, new_score);

    /* Database: update player record */
    db_update_player(&app->db, p);

    /* Score history */
    db_add_score_history(&app->db, id, old_score, new_score, ll_to_db_op(op_type));

    /* Activity log */
    ll_append(&app->activity_log, id, p->name, op_type, old_score, new_score);

    /* Undo stack (for forward operations only) */
    if (record_undo &&
        (op_type == LL_OP_ADD_POINTS || op_type == LL_OP_DEDUCT_POINTS || op_type == LL_OP_SET_SCORE)) {
        st_push(&app->undo_stack, id, old_score, new_score);
    }
    return 0;
}

int app_create_player(AppState *app, const char *name, Player *out) {
    if (!app || !name || !out) return -1;
    /* Check for duplicate name */
    if (db_get_player_by_name(&app->db, name, out) == 0) return -1;

    Player p;
    if (db_create_player(&app->db, name, &p) != 0) return -1;

    /* Add to all in-memory structures */
    avl_insert(&app->avl, &p);
    ht_insert(&app->ht, &p);
    mh_insert(&app->heap, &p);

    /* Activity log */
    ll_append(&app->activity_log, p.id, p.name, LL_OP_CREATE_PLAYER, 0, 0);

    *out = p;
    return 0;
}

int app_get_player(AppState *app, long long id, Player *out) {
    if (!app || !out) return -1;
    Player *p = ht_search(&app->ht, id);
    if (!p) return -1;
    *out = *p;
    return 0;
}

int app_get_player_by_name(AppState *app, const char *name, Player *out) {
    if (!app || !name || !out) return -1;
    /* Search in hash table by iterating (O(n)) - acceptable for college project */
    for (int i = 0; i < HASH_SIZE; i++) {
        HashNode *cur = app->ht.table[i];
        while (cur) {
            if (strcmp(cur->player->name, name) == 0) {
                *out = *cur->player;
                return 0;
            }
            cur = cur->next;
        }
    }
    return -1;
}

int app_delete_player(AppState *app, long long id) {
    if (!app) return -1;
    Player *p = ht_search(&app->ht, id);
    if (!p) return -1;
    char name[MAX_NAME_LEN];
    strncpy(name, p->name, MAX_NAME_LEN - 1);
    name[MAX_NAME_LEN - 1] = '\0';

    /* Remove from all structures */
    avl_delete(&app->avl, id);
    ht_delete(&app->ht, id);
    mh_delete(&app->heap, id);

    /* Database */
    db_delete_player(&app->db, id);

    /* Activity log */
    ll_append(&app->activity_log, id, name, LL_OP_DELETE_PLAYER, p->score, 0);
    return 0;
}

int app_list_players(AppState *app, Player **out, long long *count) {
    if (!app || !out || !count) return -1;
    /* Get from AVL in rank order */
    long long total = avl_size(&app->avl);
    Player **tmp = (Player **)malloc(total * sizeof(Player *));
    long long n = avl_top(&app->avl, total, tmp);
    *out = (Player *)malloc(n * sizeof(Player));
    for (long long i = 0; i < n; i++) (*out)[i] = *tmp[i];
    *count = n;
    free(tmp);
    return 0;
}

int app_add_points(AppState *app, long long id, long long points) {
    if (!app || points <= 0) return -1;
    Player *p = ht_search(&app->ht, id);
    if (!p) return -1;
    long long old = p->score;
    return apply_score_change(app, id, old, old + points, LL_OP_ADD_POINTS, 1);
}

int app_deduct_points(AppState *app, long long id, long long points) {
    if (!app || points <= 0) return -1;
    Player *p = ht_search(&app->ht, id);
    if (!p) return -1;
    long long old = p->score;
    return apply_score_change(app, id, old, old - points, LL_OP_DEDUCT_POINTS, 1);
}

int app_set_score(AppState *app, long long id, long long new_score) {
    if (!app || new_score < 0) return -1;
    Player *p = ht_search(&app->ht, id);
    if (!p) return -1;
    long long old = p->score;
    return apply_score_change(app, id, old, new_score, LL_OP_SET_SCORE, 1);
}

int app_undo_last(AppState *app, Player *out) {
    if (!app || !out) return -1;
    StackEntry e;
    if (st_pop(&app->undo_stack, &e) != 0) return -1;
    /* Revert the score change - don't record this on undo stack */
    if (apply_score_change(app, e.id, e.new_score, e.old_score, LL_OP_SET_SCORE, 0) != 0) {
        /* Push back on failure */
        st_push(&app->undo_stack, e.id, e.old_score, e.new_score);
        return -1;
    }
    /* Return the reverted player */
    Player *p = ht_search(&app->ht, e.id);
    if (p) *out = *p;
    return 0;
}

long long app_get_rank(AppState *app, long long id) {
    if (!app) return 0;
    return avl_rank_of(&app->avl, id);
}

long long app_get_top(AppState *app, long long count, Player *out[]) {
    if (!app) return 0;
    return avl_top(&app->avl, count, out);
}

int app_get_stats(AppState *app, long long *total_players, long long *highest_score, double *avg_score) {
    if (!app) return -1;
    if (total_players) *total_players = avl_size(&app->avl);
    if (highest_score) {
        Player *top = mh_peek(&app->heap);
        *highest_score = top ? top->score : 0;
    }
    if (avg_score) *avg_score = db_get_average_score(&app->db);
    return 0;
}

int app_process_queue(AppState *app) {
    if (!app) return -1;
    QueueEntry e;
    while (!q_empty(&app->pending_queue)) {
        if (q_dequeue(&app->pending_queue, &e) != 0) break;
        switch (e.type) {
            case QOP_ADD_POINTS:
                app_add_points(app, e.player_id, e.value);
                break;
            case QOP_DEDUCT_POINTS:
                app_deduct_points(app, e.player_id, e.value);
                break;
            case QOP_SET_SCORE:
                app_set_score(app, e.player_id, e.value);
                break;
        }
    }
    return 0;
}

long long app_get_recent_activity(AppState *app, long long count, LLNode *out[]) {
    if (!app) return 0;
    return ll_recent(&app->activity_log, count, out);
}