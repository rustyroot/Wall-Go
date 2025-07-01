#include "../include/appstate.h"

bool init_appstate (void **appstate) {

    appstate_t *as = (appstate_t *)SDL_malloc(sizeof(appstate_t));
    if (!as) return false;

    // rendering purpose
    as->has_to_be_render = true;

    // const game parameters
    as->nb_players = NB_PLAYERS;
    as->board_nb_horizontal_cells = 7;
    as->board_nb_vertical_cells = 7;

    // const display parameters
    as->zoom_speed = 1.05; // has to be close to 1, zoom_speed < 1 will resulat in inverted zoom behaviour
    as->zoom_min = 0.1;
    as->zoom_max = 0.9;

    // display parameters
    as->window_width = 640;     // default width
    as->window_height = 640;    // default height
    as->inboard_margin_cells_size_ratio = 0.4;
    as->wall_width_incell_margin_ratio = 0.72;
    as->wall_length_cells_size_ratio = 0.78;
    as->incell_margin_cells_size_ratio = 0.2;
    as->frame_width_window_max_ratio = 0.01;
    as->zoom = 0.9; // default zoom
    as->is_fullscreen = false;

    // game logic
    as->player_turn = 1;
    as->scores = NULL;

    // Colors
    as->background_color.r = 237; as->background_color.g = 239, as->background_color.b = 226, as->background_color.a = 255;
    as->text_color.r = 44; as->text_color.g = 34, as->text_color.b = 40, as->text_color.a = 255;
    as->board_color.r = 181; as->board_color.g = 154, as->board_color.b = 102, as->board_color.a = 255;
    as->empty_slot_color.r = 136; as->empty_slot_color.g = 90, as->empty_slot_color.b = 68, as->empty_slot_color.a = 255;
    as->red.r = 192; as->red.g = 41, as->red.b = 49, as->red.a = 255;
    as->red_highlight.r = 230; as->red_highlight.g = 133, as->red_highlight.b = 86, as->red_highlight.a = 255;
    as->blue.r = 32; as->blue.g = 140, as->blue.b = 178, as->blue.a = 255;
    as->blue_highlight.r = 130; as->blue_highlight.g = 220, as->blue_highlight.b = 215, as->blue_highlight.a = 255;
    as->green.r = 102; as->green.g = 166, as->green.b = 80, as->green.a = 255;
    as->green_highlight.r = 185; as->green_highlight.g = 216, as->green_highlight.b = 80, as->green_highlight.a = 255;
    as->yellow.r = 220; as->yellow.g = 152, as->yellow.b = 36, as->yellow.a = 255;
    as->yellow_highlight.r = 239; as->yellow_highlight.g = 203, as->yellow_highlight.b = 132, as->yellow_highlight.a = 255;
    as->transparent.r = 0; as->transparent.g = 0, as->transparent.b = 0, as->transparent.a = 0;
    as->BUG_color.r = 255; as->BUG_color.g = 0, as->BUG_color.b = 255, as->BUG_color.a = 255;

    // cells 
    as->cells = SDL_malloc(sizeof(int*) * as->board_nb_horizontal_cells);
    if (!as->cells) return false;
    for (int i = 0; i < as->board_nb_horizontal_cells; i++) {
        as->cells[i] = SDL_malloc(sizeof(int) * as->board_nb_vertical_cells);
        if (!as->cells[i]) return false;
        for (int j = 0; j < as->board_nb_vertical_cells; j++) {
            as->cells[i][j] = 0;
        }
    }

    if (as->nb_players <= 2) {
        as->cells[1][1] = 1;
        as->cells[5][5] = 1;
        as->cells[1][5] = 2;
        as->cells[5][1] = 2;
    }   

    // Horizontal walls
    as->horizontal_walls = SDL_malloc(sizeof(int*) * (as->board_nb_horizontal_cells));
    if (!as->horizontal_walls) return false;
    for (int i = 0; i < as->board_nb_horizontal_cells; i++) {
        as->horizontal_walls[i] = SDL_malloc(sizeof(int) * (as->board_nb_vertical_cells-1));
        if (!as->horizontal_walls[i]) return false;
        for (int j = 0; j < as->board_nb_vertical_cells-1; j++) {
            as->horizontal_walls[i][j] = 0;
        }
    }

    // Vertical_walls
    as->vertical_walls = SDL_malloc(sizeof(int*) * (as->board_nb_horizontal_cells-1));
    if (!as->vertical_walls) return false;
    for (int i = 0; i < as->board_nb_horizontal_cells-1; i++) {
        as->vertical_walls[i] = SDL_malloc(sizeof(int) * (as->board_nb_vertical_cells));
        if (!as->vertical_walls[i]) return false;
        for (int j = 0; j < as->board_nb_vertical_cells; j++) {
            as->vertical_walls[i][j] = 0;
        }
    }

    as->record = NULL;

    // phases flags
    as->is_placement_phase = true;
    as->is_first_placement_phase = true;
    as->is_second_placement_phase = false;
    as->is_there_a_selected_piece = false;
    as-> is_wall_placement_phase = false;
    as->game_over = false;

    // Create window
    if (!(as->window = SDL_CreateWindow("Walls", as->window_width, as->window_height, SDL_WINDOW_RESIZABLE))) {
        SDL_Log("Couldn't create window : %s", SDL_GetError());
        return false;
    }

    // Create renderer
    if (!(as->renderer = SDL_CreateRenderer( as->window, "gpu" ))) {
       SDL_Log("Couldn't create renderer : %s", SDL_GetError());
       return false;
    }    

    *appstate = as;

    return true;
}

void free_appstate(void **appstate) {
    appstate_t* as = (appstate_t*)appstate;

    for (int i = 0; i < as->board_nb_horizontal_cells; i++) 
        SDL_free(as->cells[i]);
    SDL_free(as->cells);

    for (int i = 0; i < as->board_nb_horizontal_cells; i++) {
        SDL_free(as->horizontal_walls[i]);
    }
    SDL_free(as->horizontal_walls);

    for (int i = 0; i < as->board_nb_horizontal_cells-1; i++) {
        SDL_free(as->vertical_walls[i]);
    }
    SDL_free(as->vertical_walls);

    destroy_record(as->record);

    SDL_free(as->scores);

    SDL_DestroyRenderer(as->renderer);
    SDL_DestroyWindow(as->window);

    SDL_free(as);
}