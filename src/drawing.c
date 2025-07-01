#include "../include/drawing.h"
#include <SDL3/SDL_oldnames.h>

int filledCircleRGBA(SDL_Renderer * renderer, int x, int y, int rad, SDL_Color color) {
    int result;
    int cx = 0;
    int cy = rad;
    int ocx = (int) 0xffff;
    int ocy = (int) 0xffff;
    int df = 1 - rad;
    int d_e = 3;
    int d_se = -2 * rad + 5;
    int xpcx, xmcx, xpcy, xmcy;
    int ypcy, ymcy, ypcx, ymcx;

    /*
    * Sanity check radius 
    */
    if (rad < 0) {
            return (-1);
    }

    /*
    * Special case for rad=0 - draw a point 
    */
    if (rad == 0) {
            result = 0;
            result |= SDL_SetRenderDrawBlendMode(renderer, (color.a == 255) ? SDL_BLENDMODE_NONE : SDL_BLENDMODE_BLEND);
            result |= SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
            result |= SDL_RenderPoint(renderer, x, y);
            return result;
    }

    /*
    * Set color
    */
    result = 0;
    result |= SDL_SetRenderDrawBlendMode(renderer, (color.a == 255) ? SDL_BLENDMODE_NONE : SDL_BLENDMODE_BLEND);
    result |= SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    /*
    * Draw 
    */
    do {
            xpcx = x + cx;
            xmcx = x - cx;
            xpcy = x + cy;
            xmcy = x - cy;
            if (ocy != cy) {
                    if (cy > 0) {
                            ypcy = y + cy;
                            ymcy = y - cy;
                            result |= SDL_RenderLine(renderer, xmcx, ypcy, xpcx, ypcy);
                            result |= SDL_RenderLine(renderer, xmcx, ymcy, xpcx, ymcy);
                    } else {
                            result |= SDL_RenderLine(renderer, xmcx, y, xpcx, y);
                    }
                    ocy = cy;
            }
            if (ocx != cx) {
                    if (cx != cy) {
                            if (cx > 0) {
                                    ypcx = y + cx;
                                    ymcx = y - cx;
                                    result |= SDL_RenderLine(renderer, xmcy, ymcx, xpcy, ymcx);
                                    result |= SDL_RenderLine(renderer, xmcy, ypcx, xpcy, ypcx);
                            } else {
                                    result |= SDL_RenderLine(renderer, xmcy, y, xpcy, y);
                            }
                    }
                    ocx = cx;
            }

            /*
            * Update 
            */
            if (df < 0) {
                    df += d_e;
                    d_e += 2;
                    d_se += 2;
            } else {
                    df += d_se;
                    d_e += 2;
                    d_se += 4;
                    cy--;
            }
            cx++;
    } while (cx <= cy);

    return (result);
}

void draw_background(void **appstate) {
    appstate_t* as = (appstate_t*)appstate;
    SDL_Color frame_color = set_color(appstate, as->player_turn, false);
    SDL_SetRenderDrawColor(as->renderer, frame_color.r, frame_color.g, frame_color.b, frame_color.a);
    SDL_RenderClear(as->renderer);
    float window_max = SDL_max(as->window_width, as->window_height);
    float frame_width =  as->frame_width_window_max_ratio * window_max;
    SDL_FRect background_rect;
    background_rect.x = frame_width;
    background_rect.y = frame_width;
    background_rect.w = as->window_width - 2 * frame_width;
    background_rect.h = as->window_height - 2 * frame_width;
    SDL_SetRenderDrawColor(as->renderer, as->background_color.r, as->background_color.g, as->background_color.b, as->background_color.a);
    SDL_RenderFillRect(as->renderer, &background_rect);
}

void draw_board(void **appstate) {
    appstate_t* as = (appstate_t*)appstate;
    SDL_FRect board_rect;
    board_rect.x = as->horizontal_margin;
    board_rect.y = as->vertical_margin;
    board_rect.w = as->board_width;
    board_rect.h = as->board_height;
    SDL_SetRenderDrawColor(as->renderer, as->board_color.r, as->board_color.g, as->board_color.b, as->board_color.a);
    SDL_RenderFillRect(as->renderer, &board_rect);
}

void draw_horizontal_wall(void **appstate, int i, int j, SDL_Color color) {
    appstate_t* as = (appstate_t*)appstate;
    SDL_FRect horizontal_wall;
    horizontal_wall.w = as->wall_length;
    horizontal_wall.h = as->wall_width;
    SDL_SetRenderDrawBlendMode(as->renderer, (color.a == 255) ? SDL_BLENDMODE_NONE : SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(as->renderer, color.r, color.g, color.b, color.a);
    horizontal_walls_coords_to_window_coords(appstate, i, j, &horizontal_wall.x, &horizontal_wall.y);
    SDL_RenderFillRect(as->renderer, &horizontal_wall);
}

void draw_horizontal_walls(void **appstate) {
    appstate_t* as = (appstate_t*)appstate;
    for (int i = 0; i < as->board_nb_horizontal_cells; i++) {
        for (int j = 0; j < as->board_nb_vertical_cells-1; j++) {
            SDL_Color wall_color = set_color(appstate, as->horizontal_walls[i][j], false);
            draw_horizontal_wall(appstate, i, j, wall_color);
        }
    }
}

void draw_vertical_wall(void **appstate, int i, int j, SDL_Color color) {
    appstate_t* as = (appstate_t*)appstate;
    SDL_FRect vertical_wall;
    vertical_wall.w = as->wall_width;
    vertical_wall.h = as->wall_length;
    SDL_SetRenderDrawBlendMode(as->renderer, (color.a == 255) ? SDL_BLENDMODE_NONE : SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(as->renderer, color.r, color.g, color.b, color.a);
    vertical_walls_coords_to_window_coords(appstate, i, j, &vertical_wall.x, &vertical_wall.y);
    SDL_RenderFillRect(as->renderer, &vertical_wall);

}

void draw_vertical_walls(void **appstate) {
    appstate_t* as = (appstate_t*)appstate;
    for (int i = 0; i < as->board_nb_horizontal_cells-1; i++) {
        for (int j = 0; j < as->board_nb_vertical_cells; j++) {
            SDL_Color wall_color = set_color(appstate, as->vertical_walls[i][j], false);
            draw_vertical_wall(appstate, i, j, wall_color);
        }
    }
}

void draw_walls(void **appstate) {
    draw_horizontal_walls(appstate);
    draw_vertical_walls(appstate);
}

void draw_pieces (void **appstate) {
    appstate_t* as = (appstate_t*)appstate;
    float piece_x, piece_y;
    for (int i = 0; i < as->board_nb_horizontal_cells; i++) {
        for (int j = 0; j < as->board_nb_vertical_cells; j++) {
            if (as->cells[i][j] > 0) {
                cells_coords_to_window_coords(appstate, i, j, &piece_x, &piece_y);
                SDL_Color piece_color = set_color(appstate, as->cells[i][j], false);
                filledCircleRGBA(as->renderer, piece_x, piece_y, as->piece_radius, piece_color);   
            }       
        }
    }
}

void draw_selected_piece(void **appstate) {
    appstate_t* as = (appstate_t*)appstate;
    float x,y;
    cells_coords_to_window_coords(appstate, as->selected_piece_i, as->selected_piece_j, &x, &y);
    filledCircleRGBA(as->renderer, x, y, as->piece_radius, set_color(appstate, as->player_turn, true)); 
}
        
void draw_avaible_moves(void **appstate) {
    appstate_t* as = (appstate_t*)appstate;
    float x,y;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if (as->avaible_moves[i][j]) {
                cells_coords_to_window_coords(appstate, as->selected_piece_i + i-2, as->selected_piece_j + j-2, &x, &y);
                filledCircleRGBA(as->renderer, x, y, as->piece_radius/2, set_color(appstate, as->player_turn, false)); 
            }
        }
    }
}

void draw_phantom_walls(void **appstate) {
    appstate_t* as = (appstate_t*)appstate;
    SDL_Color wall_color = set_color(appstate, as->player_turn, true);

    // up
    if (as->selected_piece_j < as->board_nb_vertical_cells-1 && as->horizontal_walls[as->selected_piece_i][as->selected_piece_j] == 0) 
        draw_horizontal_wall(appstate, as->selected_piece_i, as->selected_piece_j, wall_color); 

    // right
    if (as->selected_piece_i < as->board_nb_vertical_cells-1 && as->vertical_walls[as->selected_piece_i][as->selected_piece_j] == 0) 
        draw_vertical_wall(appstate, as->selected_piece_i, as->selected_piece_j, wall_color);

    // down
    if (as->selected_piece_j > 0 && as->horizontal_walls[as->selected_piece_i][as->selected_piece_j-1] == 0) 
        draw_horizontal_wall(appstate, as->selected_piece_i, as->selected_piece_j-1, wall_color);

    // left
    if (as->selected_piece_i > 0 && as->vertical_walls[as->selected_piece_i-1][as->selected_piece_j] == 0) 
        draw_vertical_wall(appstate, as->selected_piece_i-1, as->selected_piece_j, wall_color);
}