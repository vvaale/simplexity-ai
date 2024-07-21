#include "game.h"
#include "util.h"
#include <stdlib.h>
#include <stdio.h>

int getChainLength(Chain* c) {
    if (c == CHAIN_NONE_PTR)
        return 0;
    
    int x_diff = abs(c->x_0 - c->x_1)+1, y_diff = abs(c->y_0 - c->y_1)+1;
    return (x_diff > y_diff ? x_diff : y_diff);
}

int getChainOtherX(Chain* c, int x) {
    if (c->x_0 == x)
        return c->x_1;
    else if (c->x_1 == x)
        return c->x_0;
    else return -1;
}

int getChainOtherY(Chain* c, int y) {
    if (c->y_0 == y)
        return c->y_1;
    else if (c->y_1 == y)
        return c->y_0;
    else return -1;
}

void updateChainEnd(Chain* c, int old_x, int old_y, int new_x, int new_y) {
    if (c->x_0 == old_x && c->y_0 == old_y) {
        c->x_0 = new_x;
        c->y_0 = new_y;
    }
    else if (c->x_1 == old_x && c->y_1 == old_y) {
        c->x_1 = new_x;
        c->y_1 = new_y;
    }
}

void mergeChains(Chain* c1, Chain* c2, int shared_x, int shared_y) {
    updateChainEnd(c1, shared_x, shared_y, getChainOtherX(c2, shared_x), getChainOtherY(c2, shared_y));
}

int getChainDirX(Chain* c) {
    if (c == CHAIN_NONE_PTR || c->x_0 == c->x_1)
        return 0;
    else return (((c->x_1 - c->x_0) > 0) ? 1 : -1);
}

int getChainDirY(Chain* c) {
    if (c == CHAIN_NONE_PTR || c->y_0 == c->y_1)
        return 0;
    else return (((c->y_1 - c->y_0) > 0) ? 1 : -1);
}