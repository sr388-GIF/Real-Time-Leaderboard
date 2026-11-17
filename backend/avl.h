#ifndef AVL_H
#define AVL_H

#include "player.h"

/* AVL tree keyed by player score (descending) with id as tie-breaker.
 * This gives us a sorted view of all players which is exactly what a
 * leaderboard needs: insertion / deletion / search are all O(log n) and
 * an in-order traversal yields players in rank order.
 *
 * Ordering rule (tie-breaking): higher score first; on equal scores the
 * smaller player id ranks higher (earlier registration wins).
 */

typedef struct AVLNode {
    Player       *player;
    struct AVLNode *left;
    struct AVLNode *right;
    int            height;
} AVLNode;

typedef struct {
    AVLNode *root;
    long long  size;       /* number of nodes in the tree */
} AVLTree;

/* Initialise an empty tree. */
void avl_init(AVLTree *tree);

/* Insert a player. Returns 0 on success, -1 if a player with the same id
 * already exists. The caller keeps ownership of the Player pointer; the
 * tree stores a clone. */
int avl_insert(AVLTree *tree, const Player *p);

/* Delete the player with the given id. Returns 0 on success, -1 if not found. */
int avl_delete(AVLTree *tree, long long id);

/* Search for a player by id. Returns a pointer to the stored Player or NULL. */
Player *avl_search(const AVLTree *tree, long long id);

/* Find the rank (1-based) of a player id. 0 if not present. */
long long avl_rank_of(const AVLTree *tree, long long id);

/* Fill `out` with the top `count` players in rank order. Returns the
 * number of players actually written (<= count). */
long long avl_top(const AVLTree *tree, long long count, Player *out[]);

/* Number of players stored. */
long long avl_size(const AVLTree *tree);

/* Free every node and the tree itself. */
void avl_destroy(AVLTree *tree);

#endif