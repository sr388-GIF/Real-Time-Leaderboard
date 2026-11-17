#include "stack.h"
#include <stdlib.h>
#include <time.h>

void st_init(Stack *s) {
    if (s) s->top = 0;
}

int st_push(Stack *s, long long id, long long old_score, long long new_score) {
    if (!s || s->top >= MAX_STACK_CAP) return -1;
    s->data[s->top].id = id;
    s->data[s->top].old_score = old_score;
    s->data[s->top].new_score = new_score;
    s->data[s->top].timestamp = time(NULL);
    s->top++;
    return 0;
}

int st_pop(Stack *s, StackEntry *out) {
    if (!s || s->top == 0) return -1;
    s->top--;
    *out = s->data[s->top];
    return 0;
}

int st_peek(const Stack *s, StackEntry *out) {
    if (!s || s->top == 0) return -1;
    *out = s->data[s->top - 1];
    return 0;
}

long long st_size(const Stack *s) {
    return s ? s->top : 0;
}

int st_empty(const Stack *s) {
    return s && s->top == 0;
}

void st_clear(Stack *s) {
    if (s) s->top = 0;
}