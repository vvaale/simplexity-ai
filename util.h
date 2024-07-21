#ifndef UTIL_H
#define UTIL_H

#define max(a, b) ((a > b) ? a : b)

typedef struct {
    void* ptr;
    int size;
} Slice;

void printGameState(GameState* g);

int sign(int n);

void shuffleMoves(Move* array, int n);

#endif