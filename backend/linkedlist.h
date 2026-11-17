#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include "player.h"

/* Doubly linked list for activity history.
 * Stores a chronological log of score changes for the dashboard.
 * Each node: player id, old score, new score, operation type, timestamp.
 */

typedef enum {
    LL_OP_ADD_POINTS,
    LL_OP_DEDUCT_POINTS,
    LL_OP_SET_SCORE,
    LL_OP_CREATE_PLAYER,
    LL_OP_DELETE_PLAYER
} LL_OpType;

typedef struct LLNode {
    long long   player_id;
    char        player_name[128];
    LL_OpType   op_type;
    long long   old_score;
    long long   new_score;
    time_t      timestamp;
    struct LLNode *prev;
    struct LLNode *next;
} LLNode;

typedef struct {
    LLNode *head;
    LLNode *tail;
    long long size;
} LinkedList;

void ll_init(LinkedList *ll);

/* Append a new activity entry. Returns 0 on success, -1 on OOM. */
int ll_append(LinkedList *ll, long long player_id, const char *player_name,
              LL_OpType op_type, long long old_score, long long new_score);

/* Populate `out` array with most recent `count` entries (newest first).
 * Returns number of entries written (<= count). */
long long ll_recent(const LinkedList *ll, long long count, LLNode *out[]);

/* Number of entries in list. */
long long ll_size(const LinkedList *ll);

/* Free all nodes. */
void ll_destroy(LinkedList *ll);

#endif