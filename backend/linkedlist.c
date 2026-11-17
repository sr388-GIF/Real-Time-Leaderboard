#include "linkedlist.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

void ll_init(LinkedList *ll) {
    if (ll) {
        ll->head = ll->tail = NULL;
        ll->size = 0;
    }
}

int ll_append(LinkedList *ll, long long player_id, const char *player_name,
              LL_OpType op_type, long long old_score, long long new_score) {
    if (!ll) return -1;
    LLNode *n = (LLNode *)malloc(sizeof(LLNode));
    if (!n) return -1;
    n->player_id = player_id;
    strncpy(n->player_name, player_name ? player_name : "", 127);
    n->player_name[127] = '\0';
    n->op_type = op_type;
    n->old_score = old_score;
    n->new_score = new_score;
    n->timestamp = time(NULL);
    n->prev = ll->tail;
    n->next = NULL;
    if (ll->tail) ll->tail->next = n;
    else ll->head = n;
    ll->tail = n;
    ll->size++;
    return 0;
}

long long ll_recent(const LinkedList *ll, long long count, LLNode *out[]) {
    if (!ll || !out || count <= 0) return 0;
    long long written = 0;
    LLNode *cur = ll->tail;  /* start from newest */
    while (cur && written < count) {
        out[written++] = cur;
        cur = cur->prev;
    }
    return written;
}

long long ll_size(const LinkedList *ll) {
    return ll ? ll->size : 0;
}

void ll_destroy(LinkedList *ll) {
    if (!ll) return;
    LLNode *cur = ll->head;
    while (cur) {
        LLNode *next = cur->next;
        free(cur);
        cur = next;
    }
    ll->head = ll->tail = NULL;
    ll->size = 0;
}