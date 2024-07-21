#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void printGameState(GameState* g) {
    for (int y = 0; y < BOARD_ROWS; y++ ) {
        for (int x = 0; x < BOARD_COLUMNS; x++) {
            Token token = g->board[y][x];
            if (token == TOKEN_NONE) {
                printf("--");
            }
            else {
                printf("%s%s", (getTokenColor(token)==BLACK ? "B" : "W"), (getTokenShape(token)==SQUARE ? "S" : "C"));
            }
            printf(" ");
        }
        printf("\n");
    }
}

int sign(int n) {
    if (n == 0)
        return 0;
    else return (n > 0 ? 1 : -1);
}

void swapMoves(Move* a, Move* b) {
    Move temp = *a;
    *a = *b;
    *b = temp;
}

void shuffleMoves(Move* array, int n) {
    srand(time(NULL)); 
    for (int i = n - 1; i > 0; i--) {
        
        int j = rand() % (i + 1);
        
        swapMoves(&array[i], &array[j]);
    }
}