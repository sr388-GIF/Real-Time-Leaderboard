#include "player.h"
#include <stdlib.h>
#include <string.h>

Player *player_clone(const Player *p) {
    if (!p) return NULL;
    Player *c = (Player *)malloc(sizeof(Player));
    if (!c) return NULL;
    *c = *p;  /* shallow copy is fine since no internal pointers */
    return c;
}

void player_free(Player *p) {
    if (p) free(p);
}