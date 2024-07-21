#ifndef CHAIN_H
#define CHAIN_H

typedef struct {
    int x_0;
    int y_0;
    int x_1;
    int y_1;
    
    int aspect;
} Chain;

int getChainLength(Chain* c);

int getChainOtherX(Chain* c, int x);

int getChainOtherY(Chain* c, int y);

void updateChainEnd(Chain* c, int old_x, int old_y, int new_x, int new_y);

void mergeChains(Chain* c1, Chain* c2, int shared_x, int shared_y);

int getChainDirX(Chain* c);

int getChainDirY(Chain* c);

#endif