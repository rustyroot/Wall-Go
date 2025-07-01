/*
    Adaption du jeu de plateau nommé "Go bloqueur" apparaissant dans la série "à l'épreuve du diable".

    Règles :
        De 2 à 4 joueurs.
        Plateau de 7x7.
        
        Début du jeu :
            On tire un ordre aléatoire sur les joueurs.
            Pour 3 ou 4 joueurs :
                Le premier joueur pose un pions sur l'une des cases vides du plateau.
                De même pour les autres joueurs dans l'ordre
                Ensuite, les joueurs posent leur second pion dans l'ordre inverse
            Pour 2 joueurs :
                2 pions sont d'ores et déjà placé pour chaque joueurs 
                en B2/F6 et B6/F2
        
        Déroulé d'un tour :
            Le joueur choisi un de ses pions qui peut encore bouger.
            Il le déplace d'une ou deux cases (aller-retour possible)
            Il ne peut pas se déplacer sur une case où se trouve un joueur (ni passer outre).
            Il constuit ensuite un mur adjacent à sa case d'arrivée.

            3 ou 4 joueurs :
                Chaque joueur possède un et unique atout.
                Cet atout permet de casser un mur en passant à travers.
                Cet atout est à usage unique. 

        Fin de partie :
            Lorsque tous les joueurs sont seuls dans leur zone la partie s’arrête.
            On compte l'aire de chaque zone, et détermine le gagnant (et le classement) par ordre décroissant.


*/

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_video.h>
#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

typedef struct appstate_s {

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

    // phases flags
    bool is_first_placement_phase;
    bool is_second_placement_phase;
    bool is_placement_phase;
    bool is_there_a_selected_piece;
    bool is_wall_placement_phase;
} appstate_t;


// window and renderer pointers
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

// Colors
SDL_Color background_color = {237, 239, 226, 255};
SDL_Color text_color = {44, 34, 40, 255};
SDL_Color board_color = {181, 154, 102, 255};
SDL_Color empty_slot_color = {136, 90, 68, 255};
SDL_Color red = {192, 41, 49, 255};
SDL_Color red_highlight = {230, 133, 86, 255};
SDL_Color blue = {32, 140, 178, 255};
SDL_Color blue_highlight = {130, 220, 215, 255};
SDL_Color green = {102, 166, 80, 255};
SDL_Color green_highlight = {185, 216, 80, 255};
SDL_Color yellow = {220, 152, 36, 255};
SDL_Color yellow_highlight = {239, 203, 132, 255};
SDL_Color transparent = {0, 0, 0, 0};
SDL_Color BUG_color = {255, 0, 255, 255}; // default color for undefined behaviour

// 
int min (int a, int b) {
    if (a < b) return a;
    else return b;
}

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
    as->cells_size = min(as->window_width/(as->board_nb_horizontal_cells+2*as->inboard_margin_cells_size_ratio), as->window_height/(as->board_nb_vertical_cells+2*as->inboard_margin_cells_size_ratio)) * as->zoom;
    as->inboard_margin = as->inboard_margin_cells_size_ratio * as->cells_size;
    as->board_width = (as->board_nb_horizontal_cells+2*as->inboard_margin_cells_size_ratio) * as->cells_size;
    as->board_height = (as->board_nb_vertical_cells+2*as->inboard_margin_cells_size_ratio) * as->cells_size;
    as->horizontal_margin = 0.5 * (as->window_width - as->board_width);
    as->vertical_margin = 0.5 * (as->window_height - as->board_height);
    as->incell_margin = as->incell_margin_cells_size_ratio * as->cells_size;
    as->wall_width = as->wall_width_incell_margin_ratio*as->incell_margin;
    as->wall_length = min(as->wall_length_cells_size_ratio*as->cells_size, as->cells_size - as->wall_width);
    as->piece_radius = (as->cells_size - 2*as->incell_margin)/2;
}

SDL_Color set_color (int id, bool is_highlighted) {
    if (is_highlighted) {
        switch (id) {
            case 0 :    // empty slot
                return empty_slot_color;
            case 1 :    // blue
                return blue_highlight;
            case 2 :    // red
                return red_highlight;
            case 3 :    // green
                return green_highlight;
            case 4 :    // yellow
                return yellow_highlight;
            default:
                return BUG_color;
        }
    }
    else {
        switch (id) {
            case 0 :    // empty slot
                return empty_slot_color;
            case 1 :    // blue
                return blue;
            case 2 :    // red
                return red;
            case 3 :    // green
                return green;
            case 4 :    // yellow
                return yellow;
            default:
                return BUG_color;
        }
    }
    
}

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

void cells_coords_to_window_coords(void **appstate, int i, int j, float* x, float* y) {
    // return (x, y) is the center of the cell
    appstate_t* as = (appstate_t*)appstate;
    *x = as->horizontal_margin + as->inboard_margin + (i + 0.5)*as->cells_size;
    *y = as->vertical_margin + as->inboard_margin + (as->board_nb_vertical_cells-0.5-j) * as->cells_size;
}

void window_coords_to_cells_coords(void **appstate, float x, float y, int* i, int* j) {
    // (x, y) is one of the points of the cell
    appstate_t* as = (appstate_t*)appstate;
    *i = (x - (as->horizontal_margin + as->inboard_margin))/as->cells_size;
    *j = as->board_nb_vertical_cells - (y - (as->vertical_margin + as->inboard_margin))/as->cells_size;
}

void horizontal_walls_coords_to_window_coords(void **appstate, int i, int j, float* x, float* y) {
    // return (x, y) is the up-left corner of the wall
    appstate_t* as = (appstate_t*)appstate;
    *x = as->horizontal_margin + as->inboard_margin + 0.5 * (as->cells_size - as->wall_length) + i * as->cells_size; 
    *y = as->vertical_margin + as->inboard_margin + (as->board_nb_vertical_cells-j-1)*as->cells_size - 0.5 * as->wall_width;
}

void vertical_walls_coords_to_window_coords(void **appstate, int i, int j, float* x, float* y) {
    // return (x, y) is the up-left corner of the wall
    appstate_t* as = (appstate_t*)appstate;
    *x = as->horizontal_margin + as->inboard_margin - 0.5 * as->wall_width + (i+1) * as->cells_size; 
    *y = as->vertical_margin + as->inboard_margin + (as->board_nb_vertical_cells-j-1)*as->cells_size + 0.5 * (as->cells_size - as->wall_length);
}

bool is_on_board (void **appstate, float x, float y) {
    appstate_t* as = (appstate_t*)appstate;
    return x >= as->horizontal_margin + as->inboard_margin
        && x <= as->horizontal_margin + as->inboard_margin + as->board_nb_horizontal_cells*as->cells_size 
        && y >= as->vertical_margin + as->inboard_margin
        && y <= as->vertical_margin + as->inboard_margin + as->board_nb_vertical_cells*as->cells_size;
}

void window_coords_to_walls_coords (void **appstate, float x, float y, int*** walls_matrix_ptr, int* i, int* j) {
    appstate_t* as = (appstate_t*)appstate;
    // if (x, y) is a point of a wall, this function set wall_matrix to as->horizontal_walls or as->vertical_walls
    // and (i, j) to the according wall index
    // if (x, y) is not a point of a wall, walls_matrix is set to NULL and (i,j) is not touch.

    if (is_on_board(appstate, x, y)) {
        int cell_i, cell_j;
        window_coords_to_cells_coords(appstate, x, y, &cell_i, &cell_j);
        float top_left_corner_x, top_left_corner_y;
        cells_coords_to_window_coords(appstate, cell_i, cell_j, &top_left_corner_x, &top_left_corner_y);
        top_left_corner_x -= 0.5 * as->cells_size;
        top_left_corner_y -= 0.5 * as->cells_size;
        x -= top_left_corner_x;
        y -= top_left_corner_y;

        // top
        if (y <= 0.5 * as->wall_width && x >= 0.5*(as->cells_size-as->wall_length) && x <= 0.5*(as->cells_size+as->wall_length)) {
            if (cell_j >= as->board_nb_vertical_cells-1) {
                *walls_matrix_ptr = NULL;
                return;
            }
            *walls_matrix_ptr = as->horizontal_walls;
            *i = cell_i;
            *j = cell_j;
        }
        // right
        else if (x >= as->cells_size-0.5*as->wall_width && y >= 0.5*(as->cells_size-as->wall_length) && y <= 0.5*(as->cells_size+as->wall_length)) {
            if (cell_i >= as->board_nb_horizontal_cells-1) {
                *walls_matrix_ptr = NULL;
                return;
            }
            *walls_matrix_ptr = as->vertical_walls;
            *i = cell_i;
            *j = cell_j;
        }
        // down
        else if (y >= as->cells_size-0.5*as->wall_width && x >= 0.5*(as->cells_size-as->wall_length) && x <= 0.5*(as->cells_size+as->wall_length)) {
            if (cell_j == 0) {
                *walls_matrix_ptr = NULL;
                return;
            }
            *walls_matrix_ptr = as->horizontal_walls;
            *i = cell_i;
            *j = cell_j-1;
        }
        // left 
        else if (x <= 0.5 * as->wall_width && y >= 0.5*(as->cells_size-as->wall_length) && y <= 0.5*(as->cells_size+as->wall_length)) {
            if (cell_i == 0) {
                *walls_matrix_ptr = NULL;
                return;
            }
            *walls_matrix_ptr = as->vertical_walls;
            *i = cell_i-1;
            *j = cell_j;
        }
        // None of the above
        else {
            *walls_matrix_ptr = NULL;
        }
    }
    else {
        *walls_matrix_ptr = NULL;
    }
}

bool is_a_surrounder_wall (int cell_i, int cell_j, int wall_i, int wall_j) {
    return (cell_i == wall_i   && cell_j == wall_j  )
        || (cell_i == wall_i+1 && cell_j == wall_j  )
        || (cell_i == wall_i   && cell_j == wall_j+1);
}

#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3

bool is_there_a_neighbour_in_this_direction (void **appstate, int i, int j, int DIRECTION) {
    appstate_t* as = (appstate_t*)appstate;
    switch (DIRECTION) {
            //     (Don't go off limit) && (Don't get throught a wall)
        case UP:
            return (j < as->board_nb_vertical_cells-1) && (as->horizontal_walls[i][j] == 0);            
        case RIGHT:
            return (i < as->board_nb_horizontal_cells-1) && (as->vertical_walls[i][j] == 0); 
        case DOWN:
            return (j > 0) && (as->horizontal_walls[i][j-1] == 0); 
        case LEFT:
            return (i > 0) && (as->vertical_walls[i-1][j] == 0); 
        default:
            SDL_Log("Wrong DIRECTION");
            return false;
    }
}

bool is_allowed_mouvement (void **appstate, int i, int j, int DIRECTION) {
    appstate_t* as = (appstate_t*)appstate;

    if (is_there_a_neighbour_in_this_direction(appstate, i, j, DIRECTION)) {
        switch (DIRECTION) {
            //     Don't go to an occupied cell
        case UP:
            return as->cells[i][j+1] == 0;            
        case RIGHT:
            return as->cells[i+1][j] == 0; 
        case DOWN:
            return as->cells[i][j-1] == 0; 
        case LEFT:
            return as->cells[i-1][j] == 0; 
        default:
            SDL_Log("Wrong DIRECTION");
            return false;
        }
    }
    else {
        return false;
    }
}

void deep_limited_search (int limit, void **appstate, int i, int j, int* nb_avaible_moves_ptr) {
    appstate_t* as = (appstate_t*)appstate;
    if (limit > 0 && as->avaible_moves[2 + i - as->selected_piece_i][2 + j - as->selected_piece_j] == false) {
        as->avaible_moves[2 + i - as->selected_piece_i][2 + j - as->selected_piece_j] = true;
        (*nb_avaible_moves_ptr)++;
        if (is_allowed_mouvement(appstate, i, j, UP)) deep_limited_search(limit-1, appstate, i, j+1, nb_avaible_moves_ptr);
        if (is_allowed_mouvement(appstate, i, j, RIGHT)) deep_limited_search(limit-1, appstate, i+1, j, nb_avaible_moves_ptr);
        if (is_allowed_mouvement(appstate, i, j, DOWN)) deep_limited_search(limit-1, appstate, i, j-1, nb_avaible_moves_ptr);
        if (is_allowed_mouvement(appstate, i, j, LEFT)) deep_limited_search(limit-1, appstate, i-1, j, nb_avaible_moves_ptr);
    }
}

int compute_avaible_moves (void **appstate) {
    appstate_t* as = (appstate_t*)appstate;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            as->avaible_moves[i][j] = false;
        }
    }
    int nb_avaible_moves = 0;
    if (as->is_there_a_selected_piece) {
        deep_limited_search(3, appstate, as->selected_piece_i, as->selected_piece_j, &nb_avaible_moves);
        // aller-retour autorisé mais pas le sur place.
        if (as->avaible_moves[2][2] && !(as->avaible_moves[2][3] ||  as->avaible_moves[2][1] || as->avaible_moves[1][2] || as->avaible_moves[3][2])) {
            as->avaible_moves[2][2] = false;
            nb_avaible_moves--;
        }
    }
    return nb_avaible_moves;
}

int count_total_avaible_moves (void **appstate) {
    appstate_t* as = (appstate_t*)appstate;
    if (as->is_wall_placement_phase) {
        SDL_Log("wall phase moves count isn't implemented");
        return 0; // NEED TO BE IMPLEMENTED
    }
    else {
        int total = 0;
        for (int i = 0; i < as->board_nb_horizontal_cells; i++) {
            for (int j = 0; j < as->board_nb_vertical_cells; j++) {
                if (as->cells[i][j] == as->player_turn) {
                    as->is_there_a_selected_piece = true;
                    as->selected_piece_i = i;
                    as->selected_piece_j = j;
                    total += compute_avaible_moves(appstate);
                    as->is_there_a_selected_piece = false;
                }
            }
        }
        return total;
    }
}

void deepfirstsearch (void **appstate, int** connected_components, int i, int j, int connected_component_index) {
    if (connected_components[i][j] < 0) {
        connected_components[i][j] = connected_component_index;
        if (is_there_a_neighbour_in_this_direction(appstate, i, j, UP)) deepfirstsearch(appstate, connected_components, i, j+1, connected_component_index);
        if (is_there_a_neighbour_in_this_direction(appstate, i, j, RIGHT)) deepfirstsearch(appstate, connected_components, i+1, j, connected_component_index);
        if (is_there_a_neighbour_in_this_direction(appstate, i, j, DOWN)) deepfirstsearch(appstate,connected_components, i, j-1, connected_component_index);
        if (is_there_a_neighbour_in_this_direction(appstate, i, j, LEFT)) deepfirstsearch(appstate, connected_components, i-1, j, connected_component_index);
    }
}

bool is_game_over (void** appstate) {
    // determine if the game is over or not
    // if so, scores array is allocated are fill in a correct way
    // else, scores array remain untouch
    appstate_t* as = (appstate_t*)appstate;
    int** connected_components = SDL_malloc(sizeof(int*)*as->board_nb_horizontal_cells); 
    for (int i = 0; i < as->board_nb_horizontal_cells; i++) {
        connected_components[i] = SDL_malloc(sizeof(int)*as->board_nb_vertical_cells);
        for (int j = 0; j < as->board_nb_vertical_cells; j++) {
            connected_components[i][j] = -1;
        }
    }

    int nb_connected_components = 0;

    for (int i = 0; i < as->board_nb_horizontal_cells; i++) {
        for (int j = 0; j < as->board_nb_vertical_cells; j++) {
            if (connected_components[i][j] < 0) {
                deepfirstsearch(appstate, connected_components, i, j, nb_connected_components);
                nb_connected_components++;
            }
        }
    }

    SDL_Log("There is %d connected components", nb_connected_components);
    int* connected_components_owners = SDL_malloc(sizeof(int) * nb_connected_components);
    for (int k = 0; k < nb_connected_components; k++) connected_components_owners[k] = 0;

    for (int i = 0; i < as->board_nb_horizontal_cells; i++) {
        for (int j = 0; j < as->board_nb_vertical_cells; j++) {
            if (as->cells[i][j] != 0) {
                int connected_component_index = connected_components[i][j];
                if (connected_components_owners[connected_component_index] != 0  && connected_components_owners[connected_component_index] != as->cells[i][j]) {
                    SDL_Log("%c%d is own by %d but occupied by %d, so the game isn't over yet", 'A'+i, j+1, connected_components_owners[connected_component_index], as->cells[i][j]);
                    for (int i = 0; i < as->board_nb_horizontal_cells; i++) {
                        SDL_free(connected_components[i]);
                    }
                    SDL_free(connected_components);
                    SDL_free(connected_components_owners);
                    return false;
                }
                else {
                    connected_components_owners[connected_component_index] = as->cells[i][j];
                }
            }
        }
    }

    as->scores = SDL_malloc(sizeof(int) * as->nb_players);
    for (int k = 0; k < as->nb_players; k++) {
        as->scores[k] = 0;
    }

    for (int i = 0; i < as->board_nb_horizontal_cells; i++) {
        for (int j = 0; j < as->board_nb_vertical_cells; j++) {
            int connected_component_index = connected_components[i][j];
            if (connected_components_owners[connected_component_index] != 0) {
                as->scores[connected_components_owners[connected_component_index]-1]++;
            }
        }
    }

    for (int i = 0; i < as->board_nb_horizontal_cells; i++) {
        SDL_free(connected_components[i]);
    }
    SDL_free(connected_components);
    SDL_free(connected_components_owners);

    return true;
}

void window_update_reaction (void** appstate, char* event) {
    appstate_t* as = (appstate_t*)appstate;
    SDL_GetWindowSize(window, &(as->window_width), &(as->window_height));
    SDL_Log("Window event : %s; w = %d; h = %d\n", event, as->window_width, as->window_height);
    as->has_to_be_render = true;
}

void increment_turn (void **appstate) {
    appstate_t* as = (appstate_t*)appstate;
    as->player_turn = (as->player_turn == as->nb_players) ? 1 : as->player_turn+1;
}

void draw_board(void **appstate) {
    appstate_t* as = (appstate_t*)appstate;
    SDL_FRect board_rect;
    board_rect.x = as->horizontal_margin;
    board_rect.y = as->vertical_margin;
    board_rect.w = as->board_width;
    board_rect.h = as->board_height;
    SDL_RenderFillRect(renderer, &board_rect);
}

void draw_horizontal_wall(void **appstate, int i, int j, SDL_Color color) {
    appstate_t* as = (appstate_t*)appstate;
    SDL_FRect horizontal_wall;
    horizontal_wall.w = as->wall_length;
    horizontal_wall.h = as->wall_width;
    SDL_SetRenderDrawBlendMode(renderer, (color.a == 255) ? SDL_BLENDMODE_NONE : SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    horizontal_walls_coords_to_window_coords(appstate, i, j, &horizontal_wall.x, &horizontal_wall.y);
    SDL_RenderFillRect(renderer, &horizontal_wall);
}

void draw_horizontal_walls(void **appstate) {
    appstate_t* as = (appstate_t*)appstate;
    for (int i = 0; i < as->board_nb_horizontal_cells; i++) {
        for (int j = 0; j < as->board_nb_vertical_cells-1; j++) {
            SDL_Color wall_color = set_color(as->horizontal_walls[i][j], false);
            draw_horizontal_wall(appstate, i, j, wall_color);
        }
    }
}

void draw_vertical_wall(void **appstate, int i, int j, SDL_Color color) {
    appstate_t* as = (appstate_t*)appstate;
    SDL_FRect vertical_wall;
    vertical_wall.w = as->wall_width;
    vertical_wall.h = as->wall_length;
    SDL_SetRenderDrawBlendMode(renderer, (color.a == 255) ? SDL_BLENDMODE_NONE : SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    vertical_walls_coords_to_window_coords(appstate, i, j, &vertical_wall.x, &vertical_wall.y);
    SDL_RenderFillRect(renderer, &vertical_wall);

}

void draw_vertical_walls(void **appstate) {
    appstate_t* as = (appstate_t*)appstate;
    for (int i = 0; i < as->board_nb_horizontal_cells-1; i++) {
        for (int j = 0; j < as->board_nb_vertical_cells; j++) {
            SDL_Color wall_color = set_color(as->vertical_walls[i][j], false);
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
                filledCircleRGBA(renderer, piece_x, piece_y, as->piece_radius, set_color(as->cells[i][j], false));   
            }       
        }
    }
}

void draw_selected_piece(void **appstate) {
    appstate_t* as = (appstate_t*)appstate;
    float x,y;
    cells_coords_to_window_coords(appstate, as->selected_piece_i, as->selected_piece_j, &x, &y);
    filledCircleRGBA(renderer, x, y, as->piece_radius, set_color(as->player_turn, true)); 
}
        
void draw_avaible_moves(void **appstate) {
    appstate_t* as = (appstate_t*)appstate;
    float x,y;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if (as->avaible_moves[i][j]) {
                cells_coords_to_window_coords(appstate, as->selected_piece_i + i-2, as->selected_piece_j + j-2, &x, &y);
                filledCircleRGBA(renderer, x, y, as->piece_radius/2, set_color(as->player_turn, false)); 
            }
        }
    }
}

void draw_phantom_walls(void **appstate) {
    appstate_t* as = (appstate_t*)appstate;
    SDL_Color wall_color = set_color(as->player_turn, true);

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

void switch_fullscreen (void **appstate) {
    appstate_t* as = (appstate_t*)appstate;
    SDL_SetWindowFullscreen(window, !as->is_fullscreen);
    as->is_fullscreen = !as->is_fullscreen;
    as->has_to_be_render = true;
}

void update_zoom(void **appstate, float amount) {
    appstate_t* as = (appstate_t*)appstate;
    float old_value = as->zoom;
    as->zoom *= SDL_pow(as->zoom_speed, amount);
    if (as->zoom < as->zoom_min) {
        as->zoom = as->zoom_min;
    }
    else if (as->zoom > as->zoom_max) {
        as->zoom = as->zoom_max;
    }
    SDL_Log("zoom updated : %f to %f\n", old_value, as->zoom);
    as->has_to_be_render = true;
}

void manage_wall_placement_click(void **appstate, float x_mouse, float y_mouse) {
    appstate_t* as = (appstate_t*)appstate;
    int** walls_matrix;
    int i, j;
    window_coords_to_walls_coords(appstate, x_mouse, y_mouse, &walls_matrix, &i, &j);
    if (walls_matrix != NULL) {
        SDL_Log("Wall clicked (%d, %d)", i, j);
        if (is_a_surrounder_wall(as->selected_piece_i, as->selected_piece_j, i, j) && walls_matrix[i][j] == 0) {
            SDL_Log("Player %d place a wall", as->player_turn);
            walls_matrix[i][j] = as->player_turn;
            as->is_wall_placement_phase = false;

            increment_turn(appstate);
            int nb_avaible_moves = count_total_avaible_moves(appstate);
            SDL_Log("Player %d has %d moves avaible", as->player_turn, nb_avaible_moves);
            while (nb_avaible_moves == 0) {
                SDL_Log("Turn of the player %d has been skiped", as->player_turn);
                increment_turn(appstate);
                nb_avaible_moves = count_total_avaible_moves(appstate);
                SDL_Log("Player %d has %d moves avaible", as->player_turn, nb_avaible_moves);
            }

            if (is_game_over(appstate)) {
                as->game_over = true;
                for (int k = 0; k < as->nb_players; k++) {
                    SDL_Log("Player %d : %d points", k+1, as->scores[k]);
                }
                SDL_Log("GAME OVER");
            }

            as->has_to_be_render = true;
        }
    }
}

void manage_placement_piece_click (void **appstate, int i, int j) {
    appstate_t* as = (appstate_t*)appstate;
    if (as->is_first_placement_phase && as->cells[i][j] == 0) {
        SDL_Log("%c%d : player %d place a piece", 'A'+i, j+1, as->player_turn);
        as->cells[i][j] = as->player_turn;
        if (as->player_turn == as->nb_players) {
            SDL_Log("First placement phase ended");
            as->is_first_placement_phase = false;
            as->is_second_placement_phase = true;
        }
        else {
            as->player_turn += 1;
        }
        as->has_to_be_render = true;
    }
    else if (as->is_second_placement_phase && as->cells[i][j] == 0) {
        as->cells[i][j] = as->player_turn;
        SDL_Log("%c%d : player %d place a piece", 'A'+i, j+1, as->player_turn);
        if (as->player_turn == 1) {
            SDL_Log("Second placement phase ended");
            as->is_second_placement_phase = false;
            as->is_placement_phase = false;
            SDL_Log("Player %d has %d moves avaible", as->player_turn, count_total_avaible_moves(appstate));
        }
        else {
            as->player_turn -= 1;
        }
        as->has_to_be_render = true;
    }
    else {
        SDL_Log("%c%d : Nothing happened", 'A'+i, j+1);
    }
}

void select_piece(void **appstate, int i , int j) {
    appstate_t* as = (appstate_t*)appstate;
    SDL_Log("%c%d : player %d selected one of their piece", 'A'+i, j+1, as->player_turn);
    as->selected_piece_i = i;
    as->selected_piece_j = j;
    as->is_there_a_selected_piece = true;
    as->has_to_be_render = true;
    compute_avaible_moves(appstate);
}

void move_selected_piece(void **appstate, int i, int j) {
    appstate_t* as = (appstate_t*)appstate;
    SDL_Log("%c%d : player %d moved their piece from %c%d to %c%d", 'A'+i, j+1, as->player_turn, 'A'+as->selected_piece_i, as->selected_piece_j+1, 'A'+i, j+1);
    as->cells[as->selected_piece_i][as->selected_piece_j] = 0;
    as->cells[i][j] = as->player_turn;
    as->is_there_a_selected_piece = false;
    as->selected_piece_i = i;
    as->selected_piece_j = j;
    as->is_wall_placement_phase = true;
    as->has_to_be_render = true;
}

void manage_on_board_click(void **appstate, float x_mouse, float y_mouse) {
    appstate_t* as = (appstate_t*)appstate;
    if (as->is_wall_placement_phase) {
        manage_wall_placement_click(appstate, x_mouse, y_mouse);
    }
    else {
        int i, j;
        window_coords_to_cells_coords(appstate, x_mouse, y_mouse, &i, &j);
        if (as->is_placement_phase) {
            manage_placement_piece_click(appstate, i, j);
        }
        else if (as->is_there_a_selected_piece && as->avaible_moves[2 + i - as->selected_piece_i][2 + j - as->selected_piece_j]) {
            move_selected_piece(appstate, i, j);
        }
        else if (as->cells[i][j] == as->player_turn) {
            select_piece(appstate, i , j);
        }
        else {
            SDL_Log("%c%d : Nothing happened", 'A'+i, j+1);
        }
    }    
}

// char* format_cell_coords(int i, int j) {
    
// }

// void init_appstate (void **appstate) {
//     appstate_t* as = (appstate_t*)appstate;

//     // rendering purpose
//     as->has_to_be_render = true;

//     // const game parameters
//     as->nb_players = 4;
//     as->board_nb_horizontal_cells = 7;
//     as->board_nb_vertical_cells = 7;

//     // const display parameters
//     as->zoom_speed = 1.1; // has to be close to 1, zoom_speed < 1 will resulat in inverted zoom behaviour
//     as->zoom_min = 0;
//     as->zoom_max = 1;

//     // display parameters
//     as->window_width = 640;     // default width
//     as->window_height = 640;    // default height
//     as->inboard_margin_cells_size_ratio = 0.1;
//     as->wall_width_incell_margin_ratio = 0.72;
//     as->wall_length_cells_size_ratio = 0.78;
//     as->incell_margin_cells_size_ratio = 0.2;
//     as->zoom = 0.9; // default zoom
//     as->is_fullscreen = false;
//     update_display_parameters(appstate);

//     // game logic
//     as->player_turn = 1;
//     as->scores = NULL;

//     // cells 
//     as->cells = SDL_malloc(sizeof(int*) * as->board_nb_horizontal_cells);
//     for (int i = 0; i < as->board_nb_horizontal_cells; i++) {
//         as->cells[i] = SDL_malloc(sizeof(int) * as->board_nb_vertical_cells);
//         for (int j = 0; j < as->board_nb_vertical_cells; j++) {
//             as->cells[i][j] = 0;
//         }
//     }

//     if (as->nb_players <= 2) {
//         as->cells[1][1] = 1;
//         as->cells[5][5] = 1;
//         as->cells[1][5] = 2;
//         as->cells[5][1] = 2;
//     }   

//     // Horizontal walls
//     as->horizontal_walls = SDL_malloc(sizeof(int*) * (as->board_nb_horizontal_cells));
//     for (int i = 0; i < as->board_nb_horizontal_cells; i++) {
//         as->horizontal_walls[i] = SDL_malloc(sizeof(int) * (as->board_nb_vertical_cells-1));
//         for (int j = 0; j < as->board_nb_vertical_cells-1; j++) {
//             as->horizontal_walls[i][j] = 0;
//         }
//     }

//     // Vertical_walls
//     as->vertical_walls = SDL_malloc(sizeof(int*) * (as->board_nb_horizontal_cells-1));
//     for (int i = 0; i < as->board_nb_horizontal_cells-1; i++) {
//         as->vertical_walls[i] = SDL_malloc(sizeof(int) * (as->board_nb_vertical_cells));
//         for (int j = 0; j < as->board_nb_vertical_cells; j++) {
//             as->vertical_walls[i][j] = 0;
//         }
//     }

//     // phases flags
//     as->is_placement_phase = true;
//     as->is_first_placement_phase = true;
//     as->is_second_placement_phase = false;
//     as->is_there_a_selected_piece = false;
//     as-> is_wall_placement_phase = false;
//     as->game_over = false;
// }

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    // define software metadata
    SDL_SetAppMetadata("Walls", "0.1", "WALLS");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Log parameters
    for (int i = 0; i < argc; i++) SDL_Log("%s\n", argv[i]);

    // Allocate appstate
    appstate_t *as = (appstate_t *)SDL_malloc(sizeof(appstate_t));
    if (!as) return SDL_APP_FAILURE;

    // Allocate and init appstate values
    // rendering purpose
    as->has_to_be_render = true;

    // const game parameters
    as->nb_players = 4;
    as->board_nb_horizontal_cells = 7;
    as->board_nb_vertical_cells = 7;

    // const display parameters
    as->zoom_speed = 1.1; // has to be close to 1, zoom_speed < 1 will resulat in inverted zoom behaviour
    as->zoom_min = 0;
    as->zoom_max = 1;

    // display parameters
    as->window_width = 640;     // default width
    as->window_height = 640;    // default height
    as->inboard_margin_cells_size_ratio = 0.1;
    as->wall_width_incell_margin_ratio = 0.72;
    as->wall_length_cells_size_ratio = 0.78;
    as->incell_margin_cells_size_ratio = 0.2;
    as->zoom = 0.9; // default zoom
    as->is_fullscreen = false;
    update_display_parameters(appstate);

    // game logic
    as->player_turn = 1;
    as->scores = NULL;

    // cells 
    as->cells = SDL_malloc(sizeof(int*) * as->board_nb_horizontal_cells);
    for (int i = 0; i < as->board_nb_horizontal_cells; i++) {
        as->cells[i] = SDL_malloc(sizeof(int) * as->board_nb_vertical_cells);
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
    for (int i = 0; i < as->board_nb_horizontal_cells; i++) {
        as->horizontal_walls[i] = SDL_malloc(sizeof(int) * (as->board_nb_vertical_cells-1));
        for (int j = 0; j < as->board_nb_vertical_cells-1; j++) {
            as->horizontal_walls[i][j] = 0;
        }
    }

    // Vertical_walls
    as->vertical_walls = SDL_malloc(sizeof(int*) * (as->board_nb_horizontal_cells-1));
    for (int i = 0; i < as->board_nb_horizontal_cells-1; i++) {
        as->vertical_walls[i] = SDL_malloc(sizeof(int) * (as->board_nb_vertical_cells));
        for (int j = 0; j < as->board_nb_vertical_cells; j++) {
            as->vertical_walls[i][j] = 0;
        }
    }

    // phases flags
    as->is_placement_phase = true;
    as->is_first_placement_phase = true;
    as->is_second_placement_phase = false;
    as->is_there_a_selected_piece = false;
    as-> is_wall_placement_phase = false;
    as->game_over = false;

    // Create window
    if (!(window = SDL_CreateWindow("Walls", as->window_width, as->window_height, SDL_WINDOW_RESIZABLE))) {
        SDL_Log("Couldn't create window : %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Create renderer
    if (!(renderer = SDL_CreateRenderer( window, "gpu" ))) {
       SDL_Log("Couldn't create renderer : %s", SDL_GetError());
       return SDL_APP_FAILURE;
    }    

    *appstate = as;

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    appstate_t* as = (appstate_t*)appstate;
    float x_mouse, y_mouse;

    switch (event->type) {
        case SDL_EVENT_QUIT:
            return SDL_APP_SUCCESS;
            break;

        case SDL_EVENT_KEY_DOWN:
            switch (event->key.key) {
                case SDLK_ESCAPE :
                    return SDL_APP_SUCCESS; 
                
                case SDLK_F:
                    switch_fullscreen(appstate);
                    break;

                default:
                    SDL_Log("the \'%s\' key isn't bind.", SDL_GetKeyName(event->key.key));
                    break;
            }
            break;

        case SDL_EVENT_MOUSE_WHEEL:
            update_zoom(appstate, event->wheel.y);
            break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            SDL_GetMouseState(&x_mouse, &y_mouse);
            if (is_on_board(appstate, x_mouse, y_mouse) && !as->game_over) {
                manage_on_board_click(appstate, x_mouse, y_mouse);
            }
            break;

        case SDL_EVENT_WINDOW_RESIZED:
            window_update_reaction(appstate, "resized");
            break;

        case SDL_EVENT_WINDOW_ENTER_FULLSCREEN:
            window_update_reaction(appstate, "enter_fullscreen");
            break;

        case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN:
            window_update_reaction(appstate, "leave_fullscreen");
            break;

        case SDL_EVENT_WINDOW_MAXIMIZED:
            window_update_reaction(appstate, "maximized_fullscreen");
            break;

        case SDL_EVENT_WINDOW_MINIMIZED:
            window_update_reaction(appstate, "minimized_fullscreen");
            break;

    }

    return SDL_APP_CONTINUE; 
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    appstate_t* as = (appstate_t*)appstate;
    if (!as->has_to_be_render) {
        return SDL_APP_CONTINUE;
    }

    update_display_parameters(appstate);

    // Draw the background
    SDL_SetRenderDrawColor(renderer, background_color.r, background_color.g, background_color.b, background_color.a);
    SDL_RenderClear(renderer);

    // Draw the board
    SDL_SetRenderDrawColor(renderer, board_color.r, board_color.g, board_color.b, board_color.a);
    draw_board(appstate);

    // Draw pieces
    draw_pieces(appstate);
    
    if (as->is_there_a_selected_piece) {
        draw_selected_piece(appstate);
        draw_avaible_moves(appstate);
    }

    // Draw walls
    draw_walls(appstate);
    if (as->is_wall_placement_phase) {
        draw_phantom_walls(appstate);
    }
    SDL_RenderPresent(renderer);

    as->has_to_be_render = false;
    return SDL_APP_CONTINUE; 
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    if (result == SDL_APP_FAILURE) SDL_Log("Something goes wrong...\n");
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

    SDL_free(as->scores);

    SDL_free(as);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
}