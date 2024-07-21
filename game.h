#ifndef GAME_H
#define GAME_H

#define BOARD_COLUMNS 7
#define BOARD_ROWS 6
#define WIN_TOKEN_AMOUNT 4

#define SQUARE 0
#define CIRCLE 1
#define BLACK 0
#define WHITE 1

#define TOKEN_OUT_OF_BOUNDS -1
#define TOKEN_NONE 0
#define TOKEN_BLACK_SQUARE (SQUARE << 2 | BLACK << 1 | 1)   // 0b001
#define TOKEN_WHITE_SQUARE (SQUARE << 2 | WHITE << 1 | 1)   // 0b011
#define TOKEN_BLACK_CIRCLE (CIRCLE << 2 | BLACK << 1 | 1)   // 0b101
#define TOKEN_WHITE_CIRCLE (CIRCLE << 2 | WHITE << 1 | 1)   // 0b111

#define TOKEN_WHITE_MASK 0b011
#define TOKEN_CIRCLE_MASK 0b101

#define TOKEN_BLACK_SQUARE_AMOUNT 11
#define TOKEN_WHITE_SQUARE_AMOUNT 10
#define TOKEN_BLACK_CIRCLE_AMOUNT 10
#define TOKEN_WHITE_CIRCLE_AMOUNT 11

#define PLAYER_1_COLOR WHITE
#define PLAYER_1_SHAPE CIRCLE

#define PLAYER_2_COLOR BLACK
#define PLAYER_2_SHAPE SQUARE

#define GAME_STATUS_NOT_STARTED 0
#define GAME_STATUS_ONGOING 1
#define GAME_STATUS_PLAYER_1_WON 2
#define GAME_STATUS_PLAYER_2_WON 3
#define GAME_STATUS_DRAW 4

#define PLAYER_TYPE_HUMAN 0
#define PLAYER_TYPE_AI_RANDOM 1
#define PLAYER_TYPE_AI_ITERATIVE_DEEPENING_ALPHA_BETA_NO_HEURISTIC 2
#define PLAYER_TYPE_AI_ITERATIVE_DEEPENING_ALPHA_BETA_CENTER_HEURISTIC 3
#define PLAYER_TYPE_AI_ITERATIVE_DEEPENING_ALPHA_BETA_CHAIN_LENGTH_HEURISTIC 4
#define PLAYER_TYPE_AI_ITERATIVE_DEEPENING_ALPHA_BETA_SEEK_FORCED_WIN_HEURISTIC 5
#define PLAYER_TYPE_AI_ITERATIVE_DEEPENING_ALPHA_BETA_TOTAL_HEURISTIC 6
#define N_PLAYER_TYPES 7

#define MAX_DEPTH 10
#define MAX_SECONDS 2

#define CHAIN_NONE_PTR NULL
#define COLOR_NONE -1
#define SHAPE_NONE -1

#include "token.h"
#include "chain.h"

typedef struct {
    int shape;
    int x;
} Move;

typedef struct {
    int status;
    int player_1_type;
    int player_2_type;

    int** token_amounts;

    Token** board;
    int turn;
    int cur_color;
    int cur_shape;

    Chain**** color_chain_map;
    Chain**** shape_chain_map;

    Chain* longest_color_chain;
    Chain* longest_shape_chain;
} GameState;

typedef struct {
    Move move;
    int value;
} LogEntry;

int initGameState(GameState* g);

void freeGameState(GameState* g);

int isCurPlayerType(GameState* g, int player_type);

Token getTokenBeforeChain(GameState* g, Chain* c);

Token getTokenAfterChain(GameState* g, Chain* c);

int move(GameState* g, int x, Token token);

Move ai_random(GameState* g);

Move ai_iterativeDeepening(GameState* g, int (*evaluationFunction)(GameState*));

#endif