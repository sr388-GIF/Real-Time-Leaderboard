#include "queue.h"
#include <stdlib.h>
#include <time.h>

void q_init(Queue *q) {
    if (q) {
        q->head = q->tail = q->count = 0;
    }
}

int q_enqueue(Queue *q, QueueOpType type, long long player_id, long long value) {
    if (!q || q->count >= MAX_QUEUE_CAP) return -1;
    q->data[q->tail].type = type;
    q->data[q->tail].player_id = player_id;
    q->data[q->tail].value = value;
    q->data[q->tail].timestamp = time(NULL);
    q->tail = (q->tail + 1) % MAX_QUEUE_CAP;
    q->count++;
    return 0;
}

int q_dequeue(Queue *q, QueueEntry *out) {
    if (!q || q->count == 0) return -1;
    *out = q->data[q->head];
    q->head = (q->head + 1) % MAX_QUEUE_CAP;
    q->count--;
    return 0;
}

int q_peek(const Queue *q, QueueEntry *out) {
    if (!q || q->count == 0) return -1;
    *out = q->data[q->head];
    return 0;
}

long long q_size(const Queue *q) {
    return q ? q->count : 0;
}

int q_empty(const Queue *q) {
    return q && q->count == 0;
}

void q_clear(Queue *q) {
    if (q) {
        q->head = q->tail = q->count = 0;
    }
}