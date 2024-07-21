#include "game.h"
#include "util.h"
#include <stdlib.h>
#include <stdio.h>


Token getToken(int shape, int color) {
    return shape << 2 | color << 1 | 1;
}

int getTokenColor(Token t) {
    if (t != TOKEN_NONE)
        return (t >> 1) & 1;
    else return COLOR_NONE;
}

int getTokenShape(Token t) {
    if (t != TOKEN_NONE)
        return (t >> 2) & 1;
    else return SHAPE_NONE;
}

int tokensAreSameColor(Token t1, Token t2) {
    return t1 != TOKEN_NONE && t2 != TOKEN_NONE && (t1 & 0b010) == (t2 & 0b010);
}

int tokensAreSameShape(Token t1, Token t2) {
    return t1 != TOKEN_NONE && t2 != TOKEN_NONE && (t1 & 0b100) == (t2 & 0b100);
}
