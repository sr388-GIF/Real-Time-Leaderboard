#ifndef PLAYER_H
#define PLAYER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_NAME_LEN 128

/* Player record shared across all in-memory data structures. */
typedef struct Player {
    long long id;          /* unique player id (from database) */
    char     name[MAX_NAME_LEN];
    long long score;       /* current score */
    time_t    created_at;  /* registration time */
    time_t    updated_at;  /* last score-change time */
} Player;

/* Create a shallow copy of a player (allocates a new struct). */
Player *player_clone(const Player *p);

/* Free a player clone. */
void player_free(Player *p);

#endif