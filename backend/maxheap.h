#ifndef MAXHEAP_H
#define MAXHEAP_H

#include "player.h"

/* Max-heap keyed by player score (with id tie-breaker).
 * Used for efficiently extracting the top 10 players repeatedly.
 * We keep a separate heap from the AVL tree because:
 * - AVL is used for ordered traversal + rank queries
 * - Heap is used for fast top-k extraction without traversal
 * Both structures are updated in sync on score changes.
 */

#define MAX_HEAP_CAP 8192

typedef struct {
    Player **data;
    long long size;
    long long capacity;
} MaxHeap;

void mh_init(MaxHeap *h);

/* Insert a player clone. Returns 0 on success, -1 if full. */
int mh_insert(MaxHeap *h, const Player *p);

/* Remove and return the player with highest score. Returns NULL if empty. */
Player *mh_extract_max(MaxHeap *h);

/* Peek at the max element without removing. Returns NULL if empty. */
Player *mh_peek(const MaxHeap *h);

/* Update a player's score inside the heap.
 * Finds the player by id and bubbles up/down to restore heap property.
 * Returns 0 on success, -1 if not found. */
int mh_update_score(MaxHeap *h, long long id, long long new_score);

/* Delete player by id. Returns 0 on success, -1 if not found. */
int mh_delete(MaxHeap *h, long long id);

/* Fill `out` with top `count` players (max heap contents sorted).
 * Returns number of players written (<= count). */
long long mh_top(const MaxHeap *h, long long count, Player *out[]);

long long mh_size(const MaxHeap *h);
void mh_destroy(MaxHeap *h);

#endif