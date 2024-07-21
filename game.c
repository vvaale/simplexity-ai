#include "game.h"
#include "util.h"
#include "heuristics.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

int initGameState(GameState* g) {
    g->status = GAME_STATUS_NOT_STARTED;

    g->player_1_type = PLAYER_TYPE_HUMAN;
    g->player_2_type = PLAYER_TYPE_AI_RANDOM;

    g->token_amounts = malloc(sizeof(int*) * 2);
    g->token_amounts[BLACK] = malloc(sizeof(int) * 2);
    g->token_amounts[WHITE] = malloc(sizeof(int) * 2);
    g->token_amounts[BLACK][SQUARE] = TOKEN_BLACK_SQUARE_AMOUNT;
    g->token_amounts[BLACK][CIRCLE] = TOKEN_BLACK_CIRCLE_AMOUNT;
    g->token_amounts[WHITE][SQUARE] = TOKEN_WHITE_SQUARE_AMOUNT;
    g->token_amounts[WHITE][CIRCLE] = TOKEN_WHITE_CIRCLE_AMOUNT;

    g->board = malloc(BOARD_ROWS * sizeof(Token*));
    for (int y = 0; y < BOARD_ROWS; y++) {
        g->board[y] = malloc(BOARD_COLUMNS * sizeof(Token));
    }

    for (int y = 0; y < BOARD_ROWS; y++) {
        for (int x = 0; x < BOARD_COLUMNS; x++) {
            g->board[y][x] = TOKEN_NONE;
        }
    }
    g->turn = 0;
    g->cur_color = PLAYER_1_COLOR;
    g->cur_shape = PLAYER_1_SHAPE;

    g->color_chain_map = malloc(sizeof(Chain***) * BOARD_ROWS);
    g->shape_chain_map = malloc(sizeof(Chain***) * BOARD_ROWS);
    for (int y = 0; y < BOARD_ROWS; y++) {
        g->color_chain_map[y] = malloc(sizeof(Chain**) * BOARD_COLUMNS);
        g->shape_chain_map[y] = malloc(sizeof(Chain**) * BOARD_COLUMNS);
        for (int x = 0; x < BOARD_COLUMNS; x++) {
            g->color_chain_map[y][x] = malloc(sizeof(Chain*) * 4);
            g->shape_chain_map[y][x] = malloc(sizeof(Chain*) * 4);
            for (int i_dir = 0; i_dir < 4; i_dir++) {
                g->color_chain_map[y][x][i_dir] = CHAIN_NONE_PTR;
                g->shape_chain_map[y][x][i_dir] = CHAIN_NONE_PTR;
            }
        }
    }

    g->longest_color_chain = CHAIN_NONE_PTR;
    g->longest_shape_chain = CHAIN_NONE_PTR;
}

void freeGameState(GameState *g) {
    free(g->token_amounts[WHITE]);
    free(g->token_amounts[BLACK]);
    free(g->token_amounts);

    Chain* chains[BOARD_ROWS*BOARD_COLUMNS*4*2];
    int n_chains = 0;

    for (int y = 0; y < BOARD_ROWS; y++) {
        free(g->board[y]);

        for(int x = 0; x < BOARD_COLUMNS; x++) {
            for (int i_dir = 0; i_dir < 4; i_dir++) {
                Chain* c;
                c = g->color_chain_map[y][x][i_dir];
                if (c != CHAIN_NONE_PTR && c->x_0 == x && c->y_0 == y) {
                    chains[n_chains] = c;
                    n_chains++;
                }

                c = g->shape_chain_map[y][x][i_dir];
                if (c != CHAIN_NONE_PTR && c->x_0 == x && c->y_0 == y) {
                    chains[n_chains] = c;
                    n_chains++;
                }
            }
            free(g->color_chain_map[y][x]);
            free(g->shape_chain_map[y][x]);
        }
        free(g->color_chain_map[y]);
        free(g->shape_chain_map[y]);
    }
    free(g->board);
    free(g->color_chain_map);
    free(g->shape_chain_map);

    for (int i = 0; i < n_chains; i++) {
        free(chains[i]);
    }
}

void copyGameState(GameState* src, GameState* dst) {
    dst->status = src->status;
    dst->player_1_type = src->player_1_type;
    dst->player_2_type = src->player_2_type;
    dst->turn = src->turn;
    dst->cur_color = src->cur_color;
    dst->cur_shape = src->cur_shape;

    for (int y = 0; y < BOARD_ROWS; y++) {
        memcpy(dst->board[y], src->board[y], sizeof(int) * BOARD_COLUMNS);

        for (int x = 0; x < BOARD_COLUMNS; x++) {
            for (int i_dir = 0; i_dir < 4; i_dir++) {
                Chain* src_chain = src->color_chain_map[y][x][i_dir];
                if (src_chain != CHAIN_NONE_PTR && src_chain->x_0 == x && src_chain->y_0 == y) {
                    Chain* dst_chain = malloc(sizeof(Chain));
                    *(dst_chain) = *(src_chain);

                    int src_chain_dir_x = getChainDirX(src_chain), src_chain_dir_y = getChainDirY(src_chain);
                    for (int i_step = 0; i_step < getChainLength(src_chain); i_step++) {
                        int new_x = src_chain->x_0 + i_step*src_chain_dir_x, new_y = src_chain->y_0 + i_step*src_chain_dir_y;
                        dst->color_chain_map[new_y][new_x][i_dir] = dst_chain;
                    }

                    if (src_chain == src->longest_color_chain) {
                        dst->longest_color_chain = dst_chain;
                    }
                }

                src_chain = src->shape_chain_map[y][x][i_dir];
                if (src_chain != CHAIN_NONE_PTR && src_chain->x_0 == x && src_chain->y_0 == y) {
                    Chain* dst_chain = malloc(sizeof(Chain));
                    *(dst_chain) = *(src_chain);

                    int src_chain_dir_x = getChainDirX(src_chain), src_chain_dir_y = getChainDirY(src_chain);
                    for (int i_step = 0; i_step < getChainLength(src_chain); i_step++) {
                        int new_x = src_chain->x_0 + i_step*src_chain_dir_x, new_y = src_chain->y_0 + i_step*src_chain_dir_y;
                        dst->shape_chain_map[new_y][new_x][i_dir] = dst_chain;
                    }

                    if (src_chain == src->longest_shape_chain) {
                        dst->longest_shape_chain = dst_chain;
                    }
                }
            }
        }
    }

    dst->token_amounts[BLACK][SQUARE] = src->token_amounts[BLACK][SQUARE];
    dst->token_amounts[BLACK][CIRCLE] = src->token_amounts[BLACK][CIRCLE];
    dst->token_amounts[WHITE][SQUARE] = src->token_amounts[WHITE][SQUARE];
    dst->token_amounts[WHITE][CIRCLE] = src->token_amounts[WHITE][CIRCLE];
}

int isCurPlayerType(GameState* g, int player_type) {
    return (g->cur_color == PLAYER_1_COLOR && g->player_1_type == player_type ||
            g->cur_color == PLAYER_2_COLOR && g->player_2_type == player_type);
}

Token getTokenBeforeChain(GameState* g, Chain* c) {
    int dir_x = getChainDirX(c), dir_y = getChainDirY(c);
    int result_x = c->x_0 - dir_x, result_y = c->y_0 - dir_y;

    if (result_y >= 0 && result_y < BOARD_ROWS && result_x >= 0 && result_x < BOARD_COLUMNS)
        return g->board[result_y][result_x];
    else
        return TOKEN_OUT_OF_BOUNDS;
}

Token getTokenAfterChain(GameState* g, Chain* c) {
    int dir_x = getChainDirX(c), dir_y = getChainDirY(c);
    int result_x = c->x_1 + dir_x, result_y = c->y_1 + dir_y;

    if (result_y >= 0 && result_y < BOARD_ROWS && result_x >= 0 && result_x < BOARD_COLUMNS)
        return g->board[result_y][result_x];
    else
        return TOKEN_OUT_OF_BOUNDS;
}

void updateGameStatus(GameState* g) {
    if (getChainLength(g->longest_shape_chain) >= WIN_TOKEN_AMOUNT) {
        if (g->longest_shape_chain->aspect == PLAYER_1_SHAPE) {
            g->status = GAME_STATUS_PLAYER_1_WON;
        }
        else if (g->longest_shape_chain->aspect == PLAYER_2_SHAPE) {
            g->status = GAME_STATUS_PLAYER_2_WON;
        }
    } else if (getChainLength(g->longest_color_chain) >= WIN_TOKEN_AMOUNT) {
        if (g->longest_color_chain->aspect == PLAYER_1_COLOR) {
            g->status = GAME_STATUS_PLAYER_1_WON;
        }
        else if (g->longest_color_chain->aspect == PLAYER_2_COLOR) {
            g->status = GAME_STATUS_PLAYER_2_WON;
        }
    } else {
        int empty_cell_found = 0;
        for (int x = 0; x < BOARD_COLUMNS && !empty_cell_found; x++) {
            if (g->board[BOARD_ROWS-1][x] == TOKEN_NONE)
                empty_cell_found = 1;
        }
        if (empty_cell_found)
            g->status = GAME_STATUS_ONGOING;
        else {
            g->status = GAME_STATUS_DRAW;
        }
    }
}

int updateChainMap(GameState* g, int x, int y) {
    Token** board = g->board;

    int dirs[4][2] = {{0, 1}, {1, 1}, {1, 0}, {1, -1}};
    
    for (int i_dir = 0; i_dir < 4; i_dir++) {
        int* dir = dirs[i_dir];
        int other_x, other_y;

        Chain* color_chains_found[2] = {CHAIN_NONE_PTR, CHAIN_NONE_PTR};
        Chain* shape_chains_found[2] = {CHAIN_NONE_PTR, CHAIN_NONE_PTR};

        for (int i_sign = 0; i_sign < 2; i_sign++) {
            int sign = (i_sign == 0) ? 1 : -1;

            other_x = x + dir[0] * sign;
            other_y = y + dir[1] * sign;
            if (other_y >= 0 && other_y < BOARD_ROWS && 
                other_x >= 0 && other_x < BOARD_COLUMNS) {
                
                if (tokensAreSameColor(board[y][x], board[other_y][other_x])) { 
                    color_chains_found[i_sign] = g->color_chain_map[other_y][other_x][i_dir];
                    updateChainEnd(color_chains_found[i_sign], other_x, other_y, x, y);
                }

                if (tokensAreSameShape(board[y][x], board[other_y][other_x])) { 
                    shape_chains_found[i_sign] = g->shape_chain_map[other_y][other_x][i_dir];
                    updateChainEnd(shape_chains_found[i_sign], other_x, other_y, x, y);
                }
            }
        }
     
        Chain* single_color_chain;
        if (color_chains_found[0] == CHAIN_NONE_PTR && color_chains_found[1] == CHAIN_NONE_PTR) {
            Chain* new_chain = malloc(sizeof(Chain));

            new_chain->x_0 = x; new_chain->y_0 = y;
            new_chain->x_1 = x; new_chain->y_1 = y; 
            new_chain->aspect = getTokenColor(board[y][x]);

            g->color_chain_map[y][x][i_dir] = new_chain;

            if (g->longest_color_chain == CHAIN_NONE_PTR) {
                g->longest_color_chain = new_chain;
            }
        }
        else if ((color_chains_found[0] == CHAIN_NONE_PTR && color_chains_found[1] != CHAIN_NONE_PTR && (single_color_chain=color_chains_found[1], 1)) ||
                 (color_chains_found[0] != CHAIN_NONE_PTR && color_chains_found[1] == CHAIN_NONE_PTR && (single_color_chain=color_chains_found[0], 1))) {
  
            g->color_chain_map[y][x][i_dir] = single_color_chain;

            if (getChainLength(g->longest_color_chain) < getChainLength(single_color_chain)) {
                g->longest_color_chain = single_color_chain;
            }
        }
        else if (color_chains_found[0] != CHAIN_NONE_PTR && color_chains_found[1] != CHAIN_NONE_PTR) {
            mergeChains(color_chains_found[0], color_chains_found[1], x, y);
            Chain* merged_chain = color_chains_found[0];

            g->color_chain_map[y][x][i_dir] = merged_chain;

            for(int i_step = 0; i_step < getChainLength(color_chains_found[1]); i_step++) {
                int dir_sign;
                if (sign(color_chains_found[1]->x_1 - color_chains_found[1]->x_0) == sign(dir[0]) && 
                    sign(color_chains_found[1]->y_1 - color_chains_found[1]->y_0) == sign(dir[1]))
                    dir_sign = 1;
                else
                    dir_sign = -1;
                
                int y_step = i_step * dir_sign * dir[1], x_step = i_step * dir_sign * dir[0];
                int new_y = color_chains_found[1]->y_0 + y_step, new_x = color_chains_found[1]->x_0 + x_step;

                g->color_chain_map[new_y][new_x][i_dir] = merged_chain;
            }

            if (getChainLength(g->longest_color_chain) < getChainLength(merged_chain)) {
                g->longest_color_chain = merged_chain;
            }

            free(color_chains_found[1]);
        }

        Chain* single_shape_chain;
        if (shape_chains_found[0] == CHAIN_NONE_PTR && shape_chains_found[1] == CHAIN_NONE_PTR) {
            Chain* new_chain = malloc(sizeof(Chain));

            new_chain->x_0 = x; new_chain->y_0 = y;
            new_chain->x_1 = x; new_chain->y_1 = y; 
            new_chain->aspect = getTokenShape(board[y][x]);

            g->shape_chain_map[y][x][i_dir] = new_chain;

            if (g->longest_shape_chain == CHAIN_NONE_PTR) {
                g->longest_shape_chain = new_chain;
            }
        }
        else if ((shape_chains_found[0] == CHAIN_NONE_PTR && shape_chains_found[1] != CHAIN_NONE_PTR && (single_shape_chain=shape_chains_found[1], 1)) ||
                 (shape_chains_found[0] != CHAIN_NONE_PTR && shape_chains_found[1] == CHAIN_NONE_PTR && (single_shape_chain=shape_chains_found[0], 1))) {
  
            g->shape_chain_map[y][x][i_dir] = single_shape_chain;

            if (getChainLength(g->longest_shape_chain) < getChainLength(single_shape_chain)) {
                g->longest_shape_chain = single_shape_chain;
            }
        }
        else if (shape_chains_found[0] != CHAIN_NONE_PTR && shape_chains_found[1] != CHAIN_NONE_PTR) {
            mergeChains(shape_chains_found[0], shape_chains_found[1], x, y);
            Chain* merged_chain = shape_chains_found[0];

            g->shape_chain_map[y][x][i_dir] = merged_chain;

            for(int i_step = 0; i_step < getChainLength(shape_chains_found[1]); i_step++) {
                int dir_sign;
                if (sign(shape_chains_found[1]->x_1 - shape_chains_found[1]->x_0) == sign(dir[0]) && 
                    sign(shape_chains_found[1]->y_1 - shape_chains_found[1]->y_0) == sign(dir[1]))
                    dir_sign = 1;
                else
                    dir_sign = -1;
                
                int y_step = i_step * dir_sign * dir[1], x_step = i_step * dir_sign * dir[0];
                int new_y = shape_chains_found[1]->y_0 + y_step, new_x = shape_chains_found[1]->x_0 + x_step;

                g->shape_chain_map[new_y][new_x][i_dir] = merged_chain;
            }

            if (getChainLength(g->longest_shape_chain) < getChainLength(merged_chain)) {
                g->longest_shape_chain = merged_chain;
            }

            free(shape_chains_found[1]);
        }
    }
}

int move(GameState* g, int x, Token token) {
    Token** board = g->board;

    int result = 0;
    int y;

    if (board[BOARD_ROWS-1][x] != TOKEN_NONE || g->token_amounts[getTokenColor(token)][getTokenShape(token)] == 0)
        result = 0;
    else {
        for (int i_y = 0; i_y < BOARD_ROWS; i_y++) {
            if (board[i_y][x] == TOKEN_NONE) {
                board[i_y][x] = token;

                result = 1;
                y = i_y;
                break;
            }
        }

        updateChainMap(g, x, y);
    
        updateGameStatus(g);

        g->token_amounts[getTokenColor(token)][getTokenShape(token)] -= 1;

        g->cur_color = 1 - g->cur_color;
        if (g->cur_color == PLAYER_1_COLOR)
            g->turn++;
    }

    return result;
}

void getAvailableMoves(GameState* g, Move** available_moves_ptr, int* n_available_moves_ptr) {
    Token** board = g->board;
    int available_xs[BOARD_COLUMNS];
    int n_available_xs = 0;

    for (int x = 0; x < BOARD_COLUMNS; x++) {
        if (board[BOARD_ROWS-1][x] == TOKEN_NONE) {
            available_xs[n_available_xs] = x;
            n_available_xs++;
        }
    }

    int n_player_1_shapes = g->token_amounts[g->cur_color][PLAYER_1_SHAPE];
    int n_player_2_shapes = g->token_amounts[g->cur_color][PLAYER_2_SHAPE];

    int n_possible_shapes = 0;
    int possible_shapes[2] = {-1, -1};
    if (n_player_1_shapes > 0) {
        possible_shapes[n_possible_shapes] = PLAYER_1_SHAPE;
        n_possible_shapes++;
    }
    if (n_player_2_shapes > 0) {
        possible_shapes[n_possible_shapes] = PLAYER_2_SHAPE;
        n_possible_shapes++;
    }

    *available_moves_ptr = malloc(sizeof(Move) * n_possible_shapes * n_available_xs);
    *n_available_moves_ptr = n_possible_shapes * n_available_xs;

    Move* available_moves = *available_moves_ptr;

    int own_shape_is_first = 1;
    if (n_possible_shapes == 2) {
        if ((possible_shapes[0] == PLAYER_1_SHAPE && g->cur_color == PLAYER_1_COLOR) || 
            (possible_shapes[0] == PLAYER_2_SHAPE && g->cur_color == PLAYER_2_COLOR))

            own_shape_is_first = 1;
        else
            own_shape_is_first = 0;
    }

    for (int i = 0; i < n_available_xs; i++) {
        for (int j = 0; j < n_possible_shapes; j++) {
            available_moves[n_possible_shapes * i + j].shape = possible_shapes[(own_shape_is_first ? j : 1-j)];
            available_moves[n_possible_shapes * i + j].x = available_xs[i];
        }
    }
}
void getSuccessors(GameState* g, Move* available_moves, int n_available_moves, GameState** successors_ptr, int* n_successors_ptr) {
    Token** board = g->board;
    
    *successors_ptr = malloc(sizeof(GameState) * n_available_moves);
    *n_successors_ptr = n_available_moves;

    GameState* successors = *successors_ptr;

    for (int i = 0; i < n_available_moves; i++) {
        initGameState(&(successors[i]));

        copyGameState(g, &(successors[i]));

        Token token = getToken(available_moves[i].shape, g->cur_color);
    
        move(&(successors[i]), available_moves[i].x, token);
    }
}

Move ai_random(GameState* g) {
    Token** board = g->board;
    
    Move* available_moves;
    int n_available_moves;
    getAvailableMoves(g, &available_moves, &n_available_moves);

    Move chosen_move = available_moves[rand() % n_available_moves];

    free(available_moves);
    
    return chosen_move;
}


int isTerminalState(GameState* g) {
    return getChainLength(g->longest_color_chain) >= 4 || getChainLength(g->longest_shape_chain) >= 4;
}

int evaluateTerminalState(GameState *g) {
    int result = 0;
    if (getChainLength(g->longest_shape_chain) >= 4) {
        if (g->longest_shape_chain->aspect == PLAYER_1_SHAPE)
            result = 9999;
        else if (g->longest_shape_chain->aspect == PLAYER_2_SHAPE)
            result = -9999;
    } 
    else if (getChainLength(g->longest_color_chain) >= 4) {
        if (g->longest_color_chain->aspect == PLAYER_1_COLOR)
            result = 9999;
        else if (g->longest_color_chain->aspect == PLAYER_2_COLOR)
            result = -9999;
    }
    
    if (result < 0)
        result += g->turn;
    if (result > 0)
        result -= g->turn;

    return result;
}

int minValue(GameState* g, int alpha, int beta, int depth, int cur_depth_limit, int (*evaluationFunction)(GameState*), clock_t start_time);

int maxValue(GameState* g, int alpha, int beta, int depth, int cur_depth_limit, int (*evaluationFunction)(GameState*), clock_t start_time) {
    if (isTerminalState(g)) {
        return evaluateTerminalState(g);
    }

    clock_t current_time = clock();
    double elapsed_seconds = (double)(current_time - start_time) / CLOCKS_PER_SEC;
    if (depth >= cur_depth_limit || elapsed_seconds > 3.0) {
        return evaluationFunction(g);
    }
    else {
        Move* available_moves;
        int n_available_moves;
        getAvailableMoves(g, &available_moves, &n_available_moves);

        GameState* successors;
        int n_successors;
        getSuccessors(g, available_moves, n_available_moves, &successors, &n_successors);
    
        int value = -99999;
        for (int i = 0; i < n_successors; i++) {
            int min_value_result = minValue(&successors[i], alpha, beta, depth+1, cur_depth_limit, evaluationFunction, start_time);
            value = (min_value_result > value ? min_value_result : value);
            
            if (value >= beta)
                break;
            
            alpha = (value > alpha ? value : alpha);
        }
        
        free(available_moves);
        for (int i = 0; i < n_successors; i++) {
            freeGameState(&(successors[i]));
        }
        free(successors);

        return value;
    }
}

int minValue(GameState* g, int alpha, int beta, int depth, int cur_depth_limit, int (*evaluationFunction)(GameState*), clock_t start_time) {
    if (isTerminalState(g)) {
        return evaluateTerminalState(g);
    }

    clock_t current_time = clock();
    double elapsed_seconds = (double)(current_time - start_time) / CLOCKS_PER_SEC;
    if (depth >= cur_depth_limit || elapsed_seconds > 3.0) {
        return evaluationFunction(g);
    }

    else {
        Move* available_moves;
        int n_available_moves;
        getAvailableMoves(g, &available_moves, &n_available_moves);

        GameState* successors;
        int n_successors;
        getSuccessors(g, available_moves, n_available_moves, &successors, &n_successors);

        int value = 99999;
        for (int i = 0; i < n_successors; i++) {
            int max_value_result = maxValue(&successors[i], alpha, beta, depth+1, cur_depth_limit, evaluationFunction, start_time);
            value = (max_value_result < value ? max_value_result : value);
            
            if (value <= alpha)
                break;
            
            beta = (value < beta ? value : beta);
        }

        free(available_moves);
        for (int i = 0; i < n_successors; i++) {
            freeGameState(&(successors[i]));
        }
        free(successors);
        
        return value;
    }
}

Move ai_iterativeDeepening(GameState* g, int (*evaluationFunction)(GameState*)) {
    Move* available_moves;
    int n_available_moves;
    getAvailableMoves(g, &available_moves, &n_available_moves);

    shuffleMoves(available_moves, n_available_moves);

    GameState* successors;
    int n_successors;
    getSuccessors(g, available_moves, n_available_moves, &successors, &n_successors);

    int* move_values = malloc(sizeof(int) * n_available_moves);

    clock_t start_time = clock();

    int alpha = -99999, beta = 99999, timed_out = 0;
    for (int cur_max_depth = 1; cur_max_depth <= MAX_DEPTH; cur_max_depth++) {
        int best_value_cur_max_depth = (g->cur_color == PLAYER_1_COLOR) ? -99999 : 99999;

        for (int i = 0; i < n_successors; i++) {
            //printf("Iterative deepening: cur_max_depth=%d, n_successor=%d\n", cur_max_depth, i);
            int result;
            if (g->cur_color == PLAYER_1_COLOR) {
                result = minValue(&successors[i], alpha, beta, 0, cur_max_depth, evaluationFunction, start_time);
            } else if (g->cur_color == PLAYER_2_COLOR) {
                result = maxValue(&successors[i], alpha, beta, 0, cur_max_depth, evaluationFunction, start_time);
            }
            
            clock_t current_time = clock();
            double elapsed_seconds = (double)(current_time - start_time) / CLOCKS_PER_SEC;
            if (elapsed_seconds > 3.0) {
                timed_out = 1;
                break;
            }
            
            move_values[i] = result;
        }
        if (timed_out)
            break;

        if ((g->cur_color == PLAYER_1_COLOR && best_value_cur_max_depth > 9000) || 
            (g->cur_color == PLAYER_2_COLOR && best_value_cur_max_depth < -9000)) {
            break;
        }
    }

    int best_move_value = ((g->cur_color == PLAYER_1_COLOR) ? -99999 : 99999), best_move_index = -1;
    for(int i = 0; i < n_available_moves; i++) {
        if ((g->cur_color == PLAYER_1_COLOR && move_values[i] > best_move_value) ||
            (g->cur_color == PLAYER_2_COLOR && move_values[i] < best_move_value)) {
            best_move_value = move_values[i];
            best_move_index = i;
        }
    }
    Move best_move = available_moves[best_move_index];

    free(available_moves);
    for (int i = 0; i < n_successors; i++) {
        freeGameState(&(successors[i]));
    }
    free(successors);
    free(move_values);

    return best_move;
}