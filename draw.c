#include <stdio.h>
#include "draw.h"
#include "game.h"

void drawCircle(SDL_Renderer *renderer, int x, int y, int radius) {
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w; // horizontal offset
            int dy = radius - h; // vertical offset
            if ((dx*dx + dy*dy) <= (radius * radius)) {
                SDL_RenderDrawPoint(renderer, x + dx, y + dy);
            }
        }
    }
}

void drawBoard(SDL_Renderer* renderer, GameState* g) {
    Token** board = g->board;
    
    SDL_Rect base_rect = {BOARD_X, BOARD_Y + BOARD_COLUMN_HEIGHT, BOARD_WIDTH, 10};
    SDL_Rect column_rects_even[4];
    SDL_Rect column_rects_odd[3];

    int i_odd = 0, i_even = 0;
    for (int i = 0; i < 7; i++) {
        SDL_Rect column_rect = {BOARD_X + i*BOARD_COLUMN_WIDTH, BOARD_Y, BOARD_COLUMN_WIDTH, BOARD_COLUMN_HEIGHT};
        if (i % 2 == 0) 
            column_rects_even[i_even++] = column_rect;
        else
            column_rects_odd[i_odd++] = column_rect;
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    SDL_RenderFillRect(renderer, &base_rect);

    SDL_SetRenderDrawColor(renderer, 100, 100, 245, 255);
    SDL_RenderFillRects(renderer, column_rects_odd, 3);

    SDL_SetRenderDrawColor(renderer, 100, 130, 230, 255);
    SDL_RenderFillRects(renderer, column_rects_even, 4);

    for (int y = 0; y < BOARD_ROWS; y++) {
        for (int x = 0; x < BOARD_COLUMNS; x++) {
            Token token = board[y][x];
            if (token == TOKEN_NONE)
                continue;
            
            if ((token & TOKEN_WHITE_MASK) == TOKEN_WHITE_MASK) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            }
            else {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            }

            if ((token & TOKEN_CIRCLE_MASK) == TOKEN_CIRCLE_MASK) {
                int circle_x = BOARD_X + x*BOARD_COLUMN_WIDTH + BOARD_TOKEN_SIZE/2 + (BOARD_CELL_WIDTH - BOARD_TOKEN_SIZE)/2;
                int circle_y = BOARD_Y + (BOARD_ROWS-1 - y)*BOARD_CELL_HEIGHT + BOARD_TOKEN_SIZE/2 + (BOARD_CELL_HEIGHT - BOARD_TOKEN_SIZE)/2;

                drawCircle(renderer, circle_x, circle_y, BOARD_TOKEN_SIZE/2);
            }
            else {
                SDL_Rect rect;
                rect.x = BOARD_X + x*BOARD_COLUMN_WIDTH + (BOARD_CELL_WIDTH - BOARD_TOKEN_SIZE)/2;
                rect.y = BOARD_Y + (BOARD_ROWS-1 - y)*BOARD_CELL_HEIGHT + (BOARD_CELL_HEIGHT - BOARD_TOKEN_SIZE)/2;
                rect.w = BOARD_TOKEN_SIZE;
                rect.h = BOARD_TOKEN_SIZE;
                
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }
}

void drawLabel(SDL_Renderer* renderer, SDL_Texture** label_texture_ptr, int x, int y) {
    int texW = 0, texH = 0;
    SDL_QueryTexture(*label_texture_ptr, NULL, NULL, &texW, &texH);
    SDL_Rect dstrect = { x, y, texW, texH };

    SDL_RenderCopy(renderer, *label_texture_ptr, NULL, &dstrect);
}

void updateLabel(SDL_Renderer* renderer, TTF_Font* font, SDL_Texture** label_texture_ptr, char* text) {
    SDL_Color text_color = {0, 0, 0, 255};
    SDL_Surface* surface_message = TTF_RenderText_Blended(font, text, text_color);

    *label_texture_ptr = SDL_CreateTextureFromSurface(renderer, surface_message);

    SDL_FreeSurface(surface_message);
}

char* getStatusString(GameState* g) {
    switch(g->status) {
        case GAME_STATUS_NOT_STARTED: return "not started"; break;
        case GAME_STATUS_ONGOING: return "ongoing"; break;
        case GAME_STATUS_PLAYER_1_WON: return "player 1 won"; break;
        case GAME_STATUS_PLAYER_2_WON: return "player 2 won"; break;
        case GAME_STATUS_DRAW: return "draw"; break;
        default: return "...";
    }
}

void updateTurnLabel(SDL_Renderer* renderer, GameState* g, TTF_Font* font, SDL_Texture** turn_label_texture_ptr) {
    char turn_label_text[250];
    sprintf(turn_label_text, "Turn %2d | %s to move | Cur. shape: %s | Status: %s",
        g->turn, (g->cur_color == BLACK ? "BLACK" : "WHITE"), (g->cur_shape == SQUARE ? "SQUARE" : "CIRCLE"), getStatusString(g));

    updateLabel(renderer,  font, turn_label_texture_ptr, turn_label_text);
}

void drawButton(SDL_Renderer* renderer, int x, int y, int w, int h, SDL_Color color, SDL_Texture** label_texture_ptr) {
    SDL_Rect button_rect = (SDL_Rect){x, y, w, h};

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    SDL_RenderFillRect(renderer, &button_rect);

    if (label_texture_ptr != NULL)
        drawLabel(renderer, label_texture_ptr, x + 2, y + 2);
}

void drawStartButton(SDL_Renderer* renderer, GameState* g, UI* ui) {
    SDL_Texture** start_label_texture_ptr = &(ui->start_label_texture);
    SDL_Color color;
    if (g->status == GAME_STATUS_NOT_STARTED)
        color = (SDL_Color){255, 255, 255, 255};
    else
        color = (SDL_Color){255, 200, 255, 255};
    
    drawButton(renderer, START_BUTTON_X, START_BUTTON_Y, START_BUTTON_WIDTH, START_BUTTON_HEIGHT, color, start_label_texture_ptr);
}

void drawResetButton(SDL_Renderer* renderer, UI* ui) {
    SDL_Texture** reset_label_texture_ptr = &(ui->reset_label_texture);
    drawButton(renderer, RESET_BUTTON_X, RESET_BUTTON_Y, RESET_BUTTON_WIDTH, RESET_BUTTON_HEIGHT,
            (SDL_Color){255,255,255,255}, reset_label_texture_ptr);
}

void drawPlayerButtonGrid(SDL_Renderer* renderer, GameState* g, UI* ui) {

    SDL_Texture** player_1_label_texture_ptr = &(ui->player_1_label_texture);
    SDL_Texture** player_2_label_texture_ptr = &(ui->player_2_label_texture);
    SDL_Texture** player_type_label_textures = ui->player_type_label_textures;
    int n_player_type_label_textures = ui->n_player_type_label_textures;
    //int player_1_cur_button_index = ui->player_1_cur_button_index;
    //int player_2_cur_button_index = ui->player_2_cur_button_index;

    drawLabel(renderer, player_1_label_texture_ptr, 
            PLAYER_BUTTON_GRID_X + 1*PLAYER_BUTTON_GRID_COLUMN_WIDTH,
            PLAYER_BUTTON_GRID_Y);
    drawLabel(renderer, player_2_label_texture_ptr, 
            PLAYER_BUTTON_GRID_X + 2*PLAYER_BUTTON_GRID_COLUMN_WIDTH,
            PLAYER_BUTTON_GRID_Y);
    
    for (int i = 0; i < n_player_type_label_textures; i++) {
        SDL_Texture* t = player_type_label_textures[i];
        drawLabel(renderer, &t, PLAYER_BUTTON_GRID_X - 28, PLAYER_BUTTON_GRID_Y + (i+1)*PLAYER_BUTTON_GRID_ROW_HEIGHT);

        for (int j = 0; j < 2; j++) {
            int button_index = (j == 0 ? g->player_1_type : g->player_2_type);

            SDL_Color color;
            if (i == button_index)
                color = (SDL_Color){255, 200, 255, 255};
            else
                color = (SDL_Color){255, 255, 255, 255};

            drawButton(renderer, PLAYER_BUTTON_GRID_X + (j+1)*PLAYER_BUTTON_GRID_COLUMN_WIDTH,
                    PLAYER_BUTTON_GRID_Y + (i+1)*PLAYER_BUTTON_GRID_ROW_HEIGHT, 
                    PLAYER_BUTTON_GRID_BUTTON_SIZE, PLAYER_BUTTON_GRID_BUTTON_SIZE,
                    color, NULL);
        }
    }
}

int startButtonClicked(int mouse_x, int mouse_y) {
    return (mouse_x >= START_BUTTON_X && mouse_x <= START_BUTTON_X+START_BUTTON_WIDTH &&
            mouse_y >= START_BUTTON_Y && mouse_y <= START_BUTTON_Y+START_BUTTON_HEIGHT);
}

int resetButtonClicked(int mouse_x, int mouse_y) {
    return (mouse_x >= RESET_BUTTON_X && mouse_x <= RESET_BUTTON_X+RESET_BUTTON_WIDTH &&
            mouse_y >= RESET_BUTTON_Y && mouse_y <= RESET_BUTTON_Y+RESET_BUTTON_HEIGHT);
}

int playerButtonGridClicked(int mouse_x, int mouse_y) {
    return (mouse_x >= PLAYER_BUTTON_GRID_X + 1*PLAYER_BUTTON_GRID_COLUMN_WIDTH &&
            mouse_x <= PLAYER_BUTTON_GRID_X + 3*PLAYER_BUTTON_GRID_COLUMN_WIDTH &&
            mouse_y >= PLAYER_BUTTON_GRID_Y + 1*PLAYER_BUTTON_GRID_ROW_HEIGHT &&
            mouse_y <= PLAYER_BUTTON_GRID_Y + (N_PLAYER_TYPES+1)*PLAYER_BUTTON_GRID_ROW_HEIGHT);
}

int boardClicked(int mouse_x, int mouse_y) {
    return (mouse_x > BOARD_X && mouse_y > BOARD_Y && mouse_x < BOARD_X+BOARD_WIDTH && mouse_y < BOARD_Y+BOARD_HEIGHT);
}