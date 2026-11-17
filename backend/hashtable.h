#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "player.h"

/* Hash table for O(1) average-case lookup of player by ID.
 * This is crucial for rapid updates when we have the Player ID but not
 * the score, allowing us to find the Player to then update in the AVL tree.
 */

#define HASH_SIZE 1024

typedef struct HashNode {
    Player          *player;
    struct HashNode *next;
} HashNode;

typedef struct {
    HashNode *table[HASH_SIZE];
} HashTable;

/* Initialize empty hash table. */
void ht_init(HashTable *ht);

/* Insert a player clone. Returns 0 on success, -1 on ID collision. */
int ht_insert(HashTable *ht, const Player *p);

/* Search by ID. Returns pointer to player (does not take ownership). */
Player *ht_search(const HashTable *ht, long long id);

/* Delete node by ID. Returns 0 on success, -1 if not found. */
int ht_delete(HashTable *ht, long long id);

/* Clear table and free all allocated nodes/player structs. */
void ht_destroy(HashTable *ht);

#endif