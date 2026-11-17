#include "hashtable.h"
#include "player.h"
#include <stdlib.h>
#include <string.h>

static unsigned int hash_id(long long id) {
    /* FNV-1a hash on 64-bit integer */
    unsigned int h = 2166136261u;
    unsigned char *p = (unsigned char *)&id;
    for (int i = 0; i < 8; i++) {
        h ^= p[i];
        h *= 16777619u;
    }
    return h % HASH_SIZE;
}

void ht_init(HashTable *ht) {
    if (ht) memset(ht->table, 0, sizeof(ht->table));
}

int ht_insert(HashTable *ht, const Player *p) {
    if (!ht || !p) return -1;
    unsigned int idx = hash_id(p->id);
    HashNode *cur = ht->table[idx];
    while (cur) {
        if (cur->player->id == p->id) return -1;  /* duplicate */
        cur = cur->next;
    }
    HashNode *n = (HashNode *)malloc(sizeof(HashNode));
    if (!n) return -1;
    n->player = player_clone(p);
    n->next = ht->table[idx];
    ht->table[idx] = n;
    return 0;
}

Player *ht_search(const HashTable *ht, long long id) {
    if (!ht) return NULL;
    unsigned int idx = hash_id(id);
    HashNode *cur = ht->table[idx];
    while (cur) {
        if (cur->player->id == id) return cur->player;
        cur = cur->next;
    }
    return NULL;
}

int ht_delete(HashTable *ht, long long id) {
    if (!ht) return -1;
    unsigned int idx = hash_id(id);
    HashNode *cur = ht->table[idx];
    HashNode *prev = NULL;
    while (cur) {
        if (cur->player->id == id) {
            if (prev) prev->next = cur->next;
            else ht->table[idx] = cur->next;
            player_free(cur->player);
            free(cur);
            return 0;
        }
        prev = cur;
        cur = cur->next;
    }
    return -1;
}

void ht_destroy(HashTable *ht) {
    if (!ht) return;
    for (int i = 0; i < HASH_SIZE; i++) {
        HashNode *cur = ht->table[i];
        while (cur) {
            HashNode *next = cur->next;
            player_free(cur->player);
            free(cur);
            cur = next;
        }
        ht->table[i] = NULL;
    }
}