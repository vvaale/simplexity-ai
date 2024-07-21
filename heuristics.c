#include "game.h"
#include "util.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

int noHeuristic(GameState* g) {
    return 0;
}

int centerHeuristic(GameState* g) {
    int count_color_1 = 0, count_color_2 = 0;
    for (int y = 0; y < BOARD_ROWS-3; y++) {
        for (int x = 1; x < BOARD_COLUMNS-1; x++) {
            Token token = g->board[y][x];
            int color = getTokenColor(token);

            if (color == PLAYER_1_COLOR)
                count_color_1 += ((x == 1 || x == BOARD_COLUMNS-2) ? 1 : 2);
            else if (color == PLAYER_2_COLOR)
                count_color_2 += ((x == 1 || x == BOARD_COLUMNS-2) ? 1 : 2);
        }
    }

    return (count_color_1 - count_color_2);
}

int seekForcedWinHeuristic(GameState* g) {
    int spaces[2][2] = {{0, 0}, {0, 0}};
    int n_spaces = 0, done = 0;

    for (int y = 0; y < BOARD_ROWS && !done; y++) {
        for (int x = 0; x < BOARD_COLUMNS && !done; x++) {
            for (int i_dir = 0; i_dir < 4 && !done; i_dir++) {
                Chain* color_chain = g->color_chain_map[y][x][i_dir];
                
                if (color_chain != CHAIN_NONE_PTR && color_chain->x_0 == x && color_chain->y_0 == y && color_chain->aspect == g->cur_color) {
                    int length = getChainLength(color_chain);

                    if (length == 3) {
                        int dir_x = getChainDirX(color_chain), dir_y = getChainDirY(color_chain);
                        int before_chain_start_x = color_chain->x_0 - dir_x;
                        int before_chain_start_y = color_chain->y_0 - dir_y;
                        int after_chain_end_x = color_chain->x_1 + dir_x;
                        int after_chain_end_y = color_chain->y_1 + dir_y;

                        if (before_chain_start_x >= 0 && before_chain_start_x < BOARD_COLUMNS &&
                            before_chain_start_y >= 0 && before_chain_start_y < BOARD_ROWS &&
                            g->board[before_chain_start_y][before_chain_start_x] == TOKEN_NONE &&
                            (before_chain_start_y == 0 || g->board[before_chain_start_y-1][before_chain_start_x] != TOKEN_NONE) &&
                            (n_spaces == 0 || (n_spaces == 1 && spaces[0][0] != before_chain_start_x && spaces[0][1] != before_chain_start_y))) {

                            spaces[n_spaces][0] = before_chain_start_x;
                            spaces[n_spaces][1] = before_chain_start_y;
                            n_spaces++;
                            if (n_spaces == 2) {
                                done = 1;
                                break;
                            }
                        }

                        if (after_chain_end_x >= 0 && after_chain_end_x < BOARD_COLUMNS &&
                            after_chain_end_y >= 0 && after_chain_end_y < BOARD_ROWS &&
                            g->board[after_chain_end_y][after_chain_end_x] == TOKEN_NONE &&
                            (after_chain_end_y == 0 || g->board[after_chain_end_y-1][after_chain_end_x] != TOKEN_NONE) &&
                            (n_spaces == 0 || (n_spaces == 1 && spaces[0][0] != after_chain_end_x && spaces[0][1] != after_chain_end_y))) {
        
                            spaces[n_spaces][0] = after_chain_end_x;
                            spaces[n_spaces][1] = after_chain_end_y;
                            n_spaces++;
                            if (n_spaces == 2) {
                                done = 1;
                                break;
                            }
                        }
                    }
                } 

                Chain* shape_chain = g->shape_chain_map[y][x][i_dir];
                if (shape_chain != CHAIN_NONE_PTR && shape_chain->x_0 == x && shape_chain->y_0 == y &&
                    shape_chain->aspect == (g->cur_color==PLAYER_1_COLOR ? PLAYER_1_SHAPE : PLAYER_2_SHAPE)) {
                    int length = getChainLength(shape_chain);

                    if (length == 3) {
                        int dir_x = getChainDirX(shape_chain), dir_y = getChainDirY(shape_chain);
                        int before_chain_start_x = shape_chain->x_0 - dir_x;
                        int before_chain_start_y = shape_chain->y_0 - dir_y;
                        int after_chain_end_x = shape_chain->x_1 + dir_x;
                        int after_chain_end_y = shape_chain->y_1 + dir_y;

                        if (before_chain_start_x >= 0 && before_chain_start_x < BOARD_COLUMNS &&
                            before_chain_start_y >= 0 && before_chain_start_y < BOARD_ROWS &&
                            g->board[before_chain_start_y][before_chain_start_x] == TOKEN_NONE &&
                            (before_chain_start_y == 0 || g->board[before_chain_start_y-1][before_chain_start_x] != TOKEN_NONE) &&
                            (n_spaces == 0 || (n_spaces == 1 && spaces[0][0] != before_chain_start_x && spaces[0][1] != before_chain_start_y))) {

                            spaces[n_spaces][0] = before_chain_start_x;
                            spaces[n_spaces][1] = before_chain_start_y;
                            n_spaces++;
                            if (n_spaces == 2) {
                                done = 1;
                                break;
                            }
                        }

                        if (after_chain_end_x >= 0 && after_chain_end_x < BOARD_COLUMNS &&
                            after_chain_end_y >= 0 && after_chain_end_y < BOARD_ROWS &&
                            g->board[after_chain_end_y][after_chain_end_x] == TOKEN_NONE &&
                            (after_chain_end_y == 0 || g->board[after_chain_end_y-1][after_chain_end_x] != TOKEN_NONE) &&
                            (n_spaces == 0 || (n_spaces == 1 && spaces[0][0] != after_chain_end_x && spaces[0][1] != after_chain_end_y))) {
        
                            spaces[n_spaces][0] = after_chain_end_x;
                            spaces[n_spaces][1] = after_chain_end_y;
                            n_spaces++;
                            if (n_spaces == 2) {
                                done = 1;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    if (n_spaces >= 2) {
        return (g->cur_color == PLAYER_1_COLOR ? 5000 : -5000);
    }
    else return 0;
}

int chainLengthHeuristic(GameState* g) {
    int total_length_player_1 = 0, total_length_player_2 = 0, n_chains_1 = 0, n_chains_2 = 0;
    for (int y = 0; y < BOARD_ROWS; y++) {
        for (int x = 0; x < BOARD_COLUMNS; x++) {
            for (int i_dir = 0; i_dir < 4; i_dir++) {
                Chain* c;
                c = g->color_chain_map[y][x][i_dir];
                if (c != CHAIN_NONE_PTR && c->x_0 == x && c->y_0 == y &&
                    (getTokenBeforeChain(g, c) == TOKEN_NONE || getTokenAfterChain(g, c) == TOKEN_NONE)) {

                    int color = c->aspect, length = getChainLength(c);
                    
                    if (color == PLAYER_1_COLOR) {
                        total_length_player_1 += (length < 4 ? length : 4);
                        n_chains_1++;
                    }
                    else if (color == PLAYER_2_COLOR) {
                        total_length_player_2 += (length < 4 ? length : 4);
                        n_chains_2++;
                    }
                }

                c = g->shape_chain_map[y][x][i_dir];
                if (c != CHAIN_NONE_PTR && c->x_0 == x && c->y_0 == y &&
                    (getTokenBeforeChain(g, c) == TOKEN_NONE || getTokenAfterChain(g, c) == TOKEN_NONE)) {

                    int shape = c->aspect, length = getChainLength(c);
                    
                    if (shape == PLAYER_1_SHAPE) {
                        total_length_player_1 += (length < 4 ? length : 4);
                        n_chains_1++;
                    }
                    else if (shape == PLAYER_2_SHAPE) {
                        total_length_player_2 += (length < 4 ? length : 4);
                        n_chains_2++;
                    }
                }
            }
        }
    }
    return (total_length_player_1 - n_chains_1) - (total_length_player_2 - n_chains_2);
}

int totalHeuristic(GameState* g) {
    return 2 * centerHeuristic(g) + seekForcedWinHeuristic(g) + chainLengthHeuristic(g);
}