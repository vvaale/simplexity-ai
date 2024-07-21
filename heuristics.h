#ifndef HEURISTICS_H
#define HEURISTICS_H

#include "game.h"
#include "util.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

int noHeuristic(GameState* g);

int centerHeuristic(GameState* g);

int chainLengthHeuristic(GameState* g);

int seekForcedWinHeuristic(GameState* g);

int totalHeuristic(GameState* g);

#endif
