#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "game.h"
#include "draw.h"
#include "heuristics.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define EVALUATION_MODE_ACTIVE 0

typedef struct {
    int player_1_wins;
    int player_2_wins;
    int draws;
    int max_battles;
    int cur_battles;
    int total_turns;
} EvaluationData;

int main(int argc, char* argv[]) {
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }
    
    if (TTF_Init() != 0) {
        printf("TTF_Init Error: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }
    
    SDL_Window* window = SDL_CreateWindow(
        "Simplexity",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        SDL_Log("Could not create window: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_Log("Could not create renderer: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    int quit = 0;
    SDL_Event e;

    GameState* g = malloc(sizeof(GameState));
    initGameState(g);

    UI ui;
    ui.font = TTF_OpenFont("./IBMPlexMono-Regular.ttf", 16);

    updateLabel(renderer, ui.font, &(ui.title_label_texture), "Simplexity");
    updateLabel(renderer, ui.font, &(ui.start_label_texture), "Start");
    updateLabel(renderer, ui.font, &(ui.reset_label_texture), "Reset");
    updateTurnLabel(renderer, g, ui.font, &(ui.turn_label_texture));
    updateLabel(renderer, ui.font, &(ui.player_1_label_texture), "P1");
    updateLabel(renderer, ui.font, &(ui.player_2_label_texture), "P2");

    ui.n_player_type_label_textures = N_PLAYER_TYPES;
    ui.player_type_label_textures = malloc(sizeof(SDL_Texture*) * ui.n_player_type_label_textures);
    
    updateLabel(renderer, ui.font, &(ui.player_type_label_textures[0]), "Human");
    updateLabel(renderer, ui.font, &(ui.player_type_label_textures[1]), "Random");
    updateLabel(renderer, ui.font, &(ui.player_type_label_textures[2]), "I.D. A");
    updateLabel(renderer, ui.font, &(ui.player_type_label_textures[3]), "I.D. B");
    updateLabel(renderer, ui.font, &(ui.player_type_label_textures[4]), "I.D. C");
    updateLabel(renderer, ui.font, &(ui.player_type_label_textures[5]), "I.D. D");
    updateLabel(renderer, ui.font, &(ui.player_type_label_textures[6]), "I.D. E");

    ui.player_1_cur_button_index = 0;
    ui.player_2_cur_button_index = 0;

    int t = 0, waiting_post_ai_move = 0;

    int evaluation_mode_activated = EVALUATION_MODE_ACTIVE;

    EvaluationData evaluation_data;
    evaluation_data.player_1_wins = 0;
    evaluation_data.player_2_wins = 0;
    evaluation_data.draws = 0;
    evaluation_data.cur_battles = 0;
    evaluation_data.total_turns = 0;
    evaluation_data.max_battles = 10;
    
    if (evaluation_mode_activated)
        printf("[Evaluation mode is active. Max battles: %d]\n", evaluation_data.max_battles);
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN) {
                int mouse_x = e.button.x, mouse_y = e.button.y;
                
                if (e.button.button == SDL_BUTTON_LEFT) {
                    if (g->status == GAME_STATUS_ONGOING && isCurPlayerType(g, PLAYER_TYPE_HUMAN) && boardClicked(mouse_x, mouse_y)) {

                        int clicked_column = ((float)(mouse_x - BOARD_X) / (BOARD_WIDTH)) * 7;

                        Token token = getToken(g->cur_shape, g->cur_color);
                        int result = move(g, clicked_column, token);
 
                        if (result) {
                            g->cur_shape = g->cur_color == PLAYER_1_COLOR ? PLAYER_1_SHAPE : PLAYER_2_SHAPE;

                            updateTurnLabel(renderer, g, ui.font, &(ui.turn_label_texture));
                        }
                    }
                    else if (g->status == GAME_STATUS_NOT_STARTED && startButtonClicked(mouse_x, mouse_y)) {
                        if (evaluation_mode_activated && g->player_1_type != PLAYER_TYPE_HUMAN && g->player_2_type != PLAYER_TYPE_HUMAN)
                            printf("[Evaluation in progress, starting match %d of %d]\n", 
                                evaluation_data.cur_battles+1, evaluation_data.max_battles);

                        g->status = GAME_STATUS_ONGOING;
                        updateTurnLabel(renderer, g, ui.font, &(ui.turn_label_texture));
                    }
                    else if (g->status == GAME_STATUS_NOT_STARTED && playerButtonGridClicked(mouse_x, mouse_y)) {
                        
                        int x0 = PLAYER_BUTTON_GRID_X + PLAYER_BUTTON_GRID_COLUMN_WIDTH;
                        int y0 = PLAYER_BUTTON_GRID_Y + PLAYER_BUTTON_GRID_ROW_HEIGHT;
                        
                        int player = ((float)(mouse_x - x0) / (PLAYER_BUTTON_GRID_COLUMN_WIDTH*2)) * 2;
                        int player_type = ((float)(mouse_y - y0) / (PLAYER_BUTTON_GRID_ROW_HEIGHT*N_PLAYER_TYPES)) * N_PLAYER_TYPES;

                        if (player == 0) {
                            g->player_1_type = player_type;
                        }
                        else if (player == 1) {
                            g->player_2_type = player_type;
                        }
                        updateTurnLabel(renderer, g, ui.font, &(ui.turn_label_texture));
                    }
                    else if (resetButtonClicked(mouse_x, mouse_y)) {
                        int player_1_type = g->player_1_type, player_2_type = g->player_2_type;
                        freeGameState(g);
                        initGameState(g);

                        g->player_1_type = player_1_type;
                        g->player_2_type = player_2_type;
                        
                        updateTurnLabel(renderer, g, ui.font, &(ui.turn_label_texture));

                        if (evaluation_mode_activated) {
                            evaluation_data.cur_battles = 0;
                            evaluation_data.player_1_wins = 0;
                            evaluation_data.player_2_wins = 0;
                            evaluation_data.draws = 0;
                            evaluation_data.total_turns = 0;
                        }
                    }
                }
                else if (e.button.button == SDL_BUTTON_RIGHT) {
                    if (g->status == GAME_STATUS_ONGOING && isCurPlayerType(g, PLAYER_TYPE_HUMAN)) {
                        g->cur_shape = (1 - g->cur_shape);

                        updateTurnLabel(renderer, g, ui.font, &(ui.turn_label_texture));
                    }
                }
            }
        }

        if (g->status == GAME_STATUS_ONGOING && !isCurPlayerType(g, PLAYER_TYPE_HUMAN) && !waiting_post_ai_move) {
            Move m;
            if (isCurPlayerType(g, PLAYER_TYPE_AI_RANDOM)) {
                m = ai_random(g);
            }
            else if (isCurPlayerType(g, PLAYER_TYPE_AI_ITERATIVE_DEEPENING_ALPHA_BETA_NO_HEURISTIC)) {
                m = ai_iterativeDeepening(g, noHeuristic);
            }
            else if (isCurPlayerType(g, PLAYER_TYPE_AI_ITERATIVE_DEEPENING_ALPHA_BETA_CENTER_HEURISTIC)) {
                m = ai_iterativeDeepening(g, centerHeuristic);
            }
            else if (isCurPlayerType(g, PLAYER_TYPE_AI_ITERATIVE_DEEPENING_ALPHA_BETA_CHAIN_LENGTH_HEURISTIC)) {
                m = ai_iterativeDeepening(g, chainLengthHeuristic);
            }
            else if (isCurPlayerType(g, PLAYER_TYPE_AI_ITERATIVE_DEEPENING_ALPHA_BETA_SEEK_FORCED_WIN_HEURISTIC)) {
                m = ai_iterativeDeepening(g, seekForcedWinHeuristic);
            }
            else if (isCurPlayerType(g, PLAYER_TYPE_AI_ITERATIVE_DEEPENING_ALPHA_BETA_TOTAL_HEURISTIC)) {
                m = ai_iterativeDeepening(g, totalHeuristic);
            }
            
            Token token = getToken(m.shape, g->cur_color);

            printf("AI %s | MOVE SELECTED: x=%d, %s\n", (g->cur_color==BLACK ? "black" : "white"), m.x, (m.shape==SQUARE ? "square" : "circle"));
            move(g, m.x, token);
            waiting_post_ai_move = 1;
            t = 10;

            g->cur_shape = g->cur_color == PLAYER_1_COLOR ? PLAYER_1_SHAPE : PLAYER_2_SHAPE;

            updateTurnLabel(renderer, g, ui.font, &(ui.turn_label_texture));
        }
        if (waiting_post_ai_move) {
            t -= 1;
            if (t == 0)
                waiting_post_ai_move = 0;
        }

        if (evaluation_mode_activated && 
            (g->status == GAME_STATUS_PLAYER_1_WON || g->status == GAME_STATUS_PLAYER_2_WON || g->status == GAME_STATUS_DRAW) &&
            g->player_1_type != PLAYER_TYPE_HUMAN && g->player_2_type != PLAYER_TYPE_HUMAN) {

            if (g->status == GAME_STATUS_PLAYER_1_WON) {
                evaluation_data.player_1_wins++;
            }
            else if (g->status == GAME_STATUS_PLAYER_2_WON) {
                evaluation_data.player_2_wins++;
            }
            else if (g->status == GAME_STATUS_DRAW) {
                evaluation_data.draws++;
            }

            evaluation_data.cur_battles++;
            evaluation_data.total_turns += g->turn;

            int player_1_type = g->player_1_type, player_2_type = g->player_2_type;
            freeGameState(g);
            initGameState(g);

            g->player_1_type = player_1_type;
            g->player_2_type = player_2_type;
            
            if (evaluation_data.cur_battles < evaluation_data.max_battles) {
                printf("[Evaluation in progress, starting match %d of %d (currently: P1=%d, D=%d, P2=%d)]\n", 
                    evaluation_data.cur_battles+1,
                    evaluation_data.max_battles,
                    evaluation_data.player_1_wins,
                    evaluation_data.draws,
                    evaluation_data.player_2_wins);

                g->status = GAME_STATUS_ONGOING;
                updateTurnLabel(renderer, g, ui.font, &(ui.turn_label_texture));
            }
            else {
                int total_matches_played = (evaluation_data.player_1_wins+evaluation_data.player_2_wins+evaluation_data.draws);
                printf("[Evaluation complete]\nPlayer 1 wins: %d\nDraws: %d\nPlayer 2 wins: %d\nAverage turns: %.2f\n",
                    evaluation_data.player_1_wins,
                    evaluation_data.draws,
                    evaluation_data.player_2_wins,
                    (((double) evaluation_data.total_turns) / total_matches_played));
                
                evaluation_data.cur_battles = 0;
                evaluation_data.player_1_wins = 0;
                evaluation_data.player_2_wins = 0;
                evaluation_data.draws = 0;
                evaluation_data.total_turns = 0;
            }
        }
        
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderClear(renderer);
        
        drawBoard(renderer, g);

        drawLabel(renderer, &(ui.title_label_texture), TITLE_LABEL_X, TITLE_LABEL_Y);
        drawLabel(renderer, &(ui.turn_label_texture), TURN_LABEL_X, TURN_LABEL_Y);

        drawStartButton(renderer, g, &ui);
        drawResetButton(renderer, &ui);
        drawPlayerButtonGrid(renderer, g, &ui);
        
        SDL_RenderPresent(renderer);

        SDL_Delay(10);  // Delay to limit CPU usage
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

