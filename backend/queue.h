#ifndef QUEUE_H
#define QUEUE_H

#include "player.h"

/* Queue for pending score updates.
 * In a real-time system we might want to batch or log score changes.
 * This queue stores pending operations to be applied in FIFO order.
 */

#define MAX_QUEUE_CAP 2048

typedef enum {
    QOP_ADD_POINTS,
    QOP_DEDUCT_POINTS,
    QOP_SET_SCORE
} QueueOpType;

typedef struct {
    QueueOpType type;
    long long   player_id;
    long long   value;       /* points to add/deduct, or absolute score for SET */
    time_t      timestamp;
} QueueEntry;

typedef struct {
    QueueEntry data[MAX_QUEUE_CAP];
    long long  head;
    long long  tail;
    long long  count;
} Queue;

void q_init(Queue *q);

/* Enqueue an operation. Returns 0 on success, -1 if full. */
int q_enqueue(Queue *q, QueueOpType type, long long player_id, long long value);

/* Dequeue the oldest operation. Returns 0 on success, -1 if empty. */
int q_dequeue(Queue *q, QueueEntry *out);

/* Peek at front without removing. */
int q_peek(const Queue *q, QueueEntry *out);

long long q_size(const Queue *q);
int q_empty(const Queue *q);
void q_clear(Queue *q);

#endif