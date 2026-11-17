#include "maxheap.h"
#include "player.h"
#include <stdlib.h>
#include <string.h>

static int cmp(const Player *a, const Player *b) {
    if (a->score != b->score)
        return (a->score > b->score) ? 1 : -1;
    if (a->id != b->id)
        return (a->id < b->id) ? 1 : -1;
    return 0;
}

void mh_init(MaxHeap *h) {
    if (!h) return;
    h->data = (Player **)malloc(MAX_HEAP_CAP * sizeof(Player *));
    h->size = 0;
    h->capacity = MAX_HEAP_CAP;
}

static void swap(Player **a, Player **b) {
    Player *t = *a; *a = *b; *b = t;
}

static void bubble_up(MaxHeap *h, long long idx) {
    while (idx > 0) {
        long long parent = (idx - 1) / 2;
        if (cmp(h->data[idx], h->data[parent]) <= 0) break;
        swap(&h->data[idx], &h->data[parent]);
        idx = parent;
    }
}

int mh_insert(MaxHeap *h, const Player *p) {
    if (!h || h->size >= h->capacity) return -1;
    h->data[h->size] = player_clone(p);
    bubble_up(h, h->size);
    h->size++;
    return 0;
}

static void bubble_down(MaxHeap *h, long long idx) {
    while (1) {
        long long left = 2 * idx + 1;
        long long right = 2 * idx + 2;
        long long largest = idx;
        if (left < h->size && cmp(h->data[left], h->data[largest]) > 0)
            largest = left;
        if (right < h->size && cmp(h->data[right], h->data[largest]) > 0)
            largest = right;
        if (largest == idx) break;
        swap(&h->data[idx], &h->data[largest]);
        idx = largest;
    }
}

Player *mh_extract_max(MaxHeap *h) {
    if (!h || h->size == 0) return NULL;
    Player *max = h->data[0];
    h->size--;
    h->data[0] = h->data[h->size];
    h->data[h->size] = NULL;
    bubble_down(h, 0);
    return max;
}

Player *mh_peek(const MaxHeap *h) {
    if (!h || h->size == 0) return NULL;
    return h->data[0];
}

int mh_update_score(MaxHeap *h, long long id, long long new_score) {
    if (!h) return -1;
    long long idx = -1;
    for (long long i = 0; i < h->size; i++) {
        if (h->data[i]->id == id) {
            idx = i;
            break;
        }
    }
    if (idx == -1) return -1;
    long long old_score = h->data[idx]->score;
    h->data[idx]->score = new_score;
    if (new_score > old_score) bubble_up(h, idx);
    else bubble_down(h, idx);
    return 0;
}

int mh_delete(MaxHeap *h, long long id) {
    if (!h) return -1;
    long long idx = -1;
    for (long long i = 0; i < h->size; i++) {
        if (h->data[i]->id == id) {
            idx = i;
            break;
        }
    }
    if (idx == -1) return -1;
    player_free(h->data[idx]);
    h->size--;
    h->data[idx] = h->data[h->size];
    h->data[h->size] = NULL;
    if (idx < h->size) {
        bubble_up(h, idx);
        bubble_down(h, idx);
    }
    return 0;
}

long long mh_top(const MaxHeap *h, long long count, Player *out[]) {
    if (!h) return 0;
    long long n = h->size < count ? h->size : count;
    Player **tmp = (Player **)malloc(h->size * sizeof(Player *));
    for (long long i = 0; i < h->size; i++) tmp[i] = h->data[i];
    /* simple sort for top n */
    for (long long i = 0; i < n; i++) {
        long long best = i;
        for (long long j = i + 1; j < h->size; j++) {
            if (cmp(tmp[j], tmp[best]) > 0) best = j;
        }
        if (best != i) swap(&tmp[i], &tmp[best]);
        out[i] = tmp[i];
    }
    free(tmp);
    return n;
}

long long mh_size(const MaxHeap *h) {
    return h ? h->size : 0;
}

void mh_destroy(MaxHeap *h) {
    if (!h) return;
    for (long long i = 0; i < h->size; i++) {
        player_free(h->data[i]);
    }
    free(h->data);
    h->data = NULL;
    h->size = 0;
    h->capacity = 0;
}