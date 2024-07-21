#ifndef DRAW_H
#define DRAW_H

#define BOARD_X 80
#define BOARD_Y 150
#define BOARD_CELL_WIDTH 52
#define BOARD_CELL_HEIGHT 52
#define BOARD_TOKEN_SIZE 42
#define BOARD_BASE_HEIGHT 20

#define BOARD_COLUMN_WIDTH BOARD_CELL_WIDTH
#define BOARD_COLUMN_HEIGHT BOARD_ROWS*BOARD_CELL_HEIGHT
#define BOARD_WIDTH BOARD_COLUMNS*BOARD_COLUMN_WIDTH
#define BOARD_HEIGHT BOARD_COLUMN_HEIGHT + BOARD_BASE_HEIGHT

#define TITLE_LABEL_X 20
#define TITLE_LABEL_Y 20
#define TURN_LABEL_X 20
#define TURN_LABEL_Y 40

#define START_BUTTON_X 20
#define START_BUTTON_Y 60
#define START_BUTTON_WIDTH 50
#define START_BUTTON_HEIGHT 18

#define RESET_BUTTON_X 80
#define RESET_BUTTON_Y 60
#define RESET_BUTTON_WIDTH 50
#define RESET_BUTTON_HEIGHT 18

#define PLAYER_BUTTON_GRID_X 600
#define PLAYER_BUTTON_GRID_Y 200
#define PLAYER_BUTTON_GRID_ROW_HEIGHT 30
#define PLAYER_BUTTON_GRID_COLUMN_WIDTH 40
#define PLAYER_BUTTON_GRID_BUTTON_SIZE 24

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "game.h"

typedef struct {
        TTF_Font* font;
        SDL_Texture* title_label_texture;
        SDL_Texture* turn_label_texture;

        SDL_Texture* start_label_texture;
        SDL_Texture* reset_label_texture;

        SDL_Texture* player_1_label_texture;
        SDL_Texture* player_2_label_texture;
        SDL_Texture** player_type_label_textures;
        int n_player_type_label_textures;

        int player_1_cur_button_index;
        int player_2_cur_button_index;
} UI;

void drawBoard(SDL_Renderer* renderer, GameState* g);

void drawCircle(SDL_Renderer *renderer, int x, int y, int radius);

void drawLabel(SDL_Renderer* renderer, SDL_Texture** label_texture_ptr, int x, int y);

void updateLabel(SDL_Renderer* renderer, TTF_Font* font, SDL_Texture** label_texture_ptr, char* text);

void updateTurnLabel(SDL_Renderer* renderer, GameState* g, TTF_Font* font, SDL_Texture** turn_label_texture_ptr);

void drawStartButton(SDL_Renderer* renderer, GameState* g, UI* ui);

void drawResetButton(SDL_Renderer* renderer, UI* ui);

void drawPlayerButtonGrid(SDL_Renderer* renderer, GameState* g, UI* ui);

int boardClicked(int mouse_x, int mouse_y);

int startButtonClicked(int mouse_x, int mouse_y);

int resetButtonClicked(int mouse_x, int mouse_y);

int playerButtonGridClicked(int mouse_x, int mouse_y);

#endif