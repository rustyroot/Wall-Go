#include "moves_recording.h"
#ifndef APPSTATE_H
#define APPSTATE_H APPSTATE_H

#include <SDL3/SDL.h>
#include "moves_recording.h"

#define NB_PLAYERS 3

typedef struct appstate_s {

    // window and renderer pointers
    SDL_Window *window;
    SDL_Renderer *renderer;

    // rendering purpose
    bool has_to_be_render;

    // const display parameters
    float zoom_speed;
    float zoom_min;
    float zoom_max;

    // display parameters
    int window_width;
    int window_height;
    float inboard_margin_cells_size_ratio;
    float wall_width_incell_margin_ratio;
    float wall_length_cells_size_ratio;
    float incell_margin_cells_size_ratio;
    float frame_width_window_max_ratio;
    float zoom; // between 0 and 1
    bool is_fullscreen;
    float cells_size;
    float inboard_margin;
    float horizontal_margin;
    float vertical_margin;
    float board_width;
    float board_height;
    float incell_margin;
    float wall_width;
    float wall_length;
    float piece_radius;

    // const game parameters
    int nb_players;
    int board_nb_horizontal_cells;
    int board_nb_vertical_cells;

    // Colors
    SDL_Color background_color;
    SDL_Color text_color;
    SDL_Color board_color;
    SDL_Color empty_slot_color;
    SDL_Color red;
    SDL_Color red_highlight;
    SDL_Color blue;
    SDL_Color blue_highlight;
    SDL_Color green;
    SDL_Color green_highlight;
    SDL_Color yellow;
    SDL_Color yellow_highlight;
    SDL_Color transparent;
    SDL_Color BUG_color; // default color for undefined behaviour

    // game logic
    int** cells;
    int** horizontal_walls;
    int** vertical_walls;
    int selected_piece_i;
    int selected_piece_j;
    bool avaible_moves[5][5];
    int player_turn;
    int* scores;
    bool game_over;
    record_t* record;

    // phases flags
    bool is_first_placement_phase;
    bool is_second_placement_phase;
    bool is_placement_phase;
    bool is_there_a_selected_piece;
    bool is_wall_placement_phase;
} appstate_t;

bool init_appstate (void **appstate);
void free_appstate (void **appstate);

#endif