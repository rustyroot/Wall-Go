#include "../include/graphical_logic.h"

void update_display_parameters (void **appstate) {
    // know :   window_width, 
    //          window_height, 
    //          zoom,
    //          board_nb_horizontal_cells, 
    //          board_nb_vertical_cells, 
    //          inboard_margin_cells_size_ratio,
    //          wall_width_incell_margin_ratio,
    //          wall_length_cells_size_ratio,
    //          incell_margin_cells_size_ratio

    appstate_t* as = (appstate_t*)appstate;
    as->cells_size = SDL_min(as->window_width/(as->board_nb_horizontal_cells+2*as->inboard_margin_cells_size_ratio), as->window_height/(as->board_nb_vertical_cells+2*as->inboard_margin_cells_size_ratio)) * as->zoom;
    as->inboard_margin = as->inboard_margin_cells_size_ratio * as->cells_size;
    as->board_width = (as->board_nb_horizontal_cells+2*as->inboard_margin_cells_size_ratio) * as->cells_size;
    as->board_height = (as->board_nb_vertical_cells+2*as->inboard_margin_cells_size_ratio) * as->cells_size;
    as->horizontal_margin = 0.5 * (as->window_width - as->board_width);
    as->vertical_margin = 0.5 * (as->window_height - as->board_height);
    as->incell_margin = as->incell_margin_cells_size_ratio * as->cells_size;
    as->wall_width = as->wall_width_incell_margin_ratio*as->incell_margin;
    as->wall_length = SDL_min(as->wall_length_cells_size_ratio*as->cells_size, as->cells_size - as->wall_width);
    as->piece_radius = (as->cells_size - 2*as->incell_margin)/2;
}

SDL_Color set_color (void **appstate, int COLOR, bool is_highlighted) {
    appstate_t* as = (appstate_t*)appstate;
    if (is_highlighted) {
        switch (COLOR) {
            case 0 :    // empty slot
                return as->empty_slot_color;
            case 1 :    // blue
                return as->blue_highlight;
            case 2 :    // red
                return as->red_highlight;
            case 3 :    // green
                return as->green_highlight;
            case 4 :    // yellow
                return as->yellow_highlight;
            default:
                return as->BUG_color;
        }
    }
    else {
        switch (COLOR) {
            case 0 :    // empty slot
                return as->empty_slot_color;
            case 1 :    // blue
                return as->blue;
            case 2 :    // red
                return as->red;
            case 3 :    // green
                return as->green;
            case 4 :    // yellow
                return as->yellow;
            default:
                return as->BUG_color;
        }
    }
    
}
