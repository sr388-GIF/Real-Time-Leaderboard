#ifndef STACK_H
#define STACK_H

#include "player.h"

/* Stack for undo functionality.
 * Each entry stores enough information to reverse a score update:
 * - player id
 * - old score
 * - new score
 * - timestamp
 */

#define MAX_STACK_CAP 1024

typedef struct {
    long long id;
    long long old_score;
    long long new_score;
    time_t    timestamp;
} StackEntry;

typedef struct {
    StackEntry data[MAX_STACK_CAP];
    long long  top;   /* index of next free slot = count */
} Stack;

void st_init(Stack *s);

/* Push an entry. Returns 0 on success, -1 if full. */
int st_push(Stack *s, long long id, long long old_score, long long new_score);

/* Pop the most recent entry. Returns 0 on success, -1 if empty. */
int st_pop(Stack *s, StackEntry *out);

/* Peek at top without removing. */
int st_peek(const Stack *s, StackEntry *out);

long long st_size(const Stack *s);
int st_empty(const Stack *s);
void st_clear(Stack *s);

#endif