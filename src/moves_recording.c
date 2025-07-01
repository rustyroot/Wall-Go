#include "../include/moves_recording.h"
#include <SDL3/SDL_stdinc.h>

move_t* create_move (int type, int i, int j, int i_destination, int j_destination, bool is_vertical_wall) {
    move_t* move = SDL_malloc(sizeof(move_t));
    move->type = type;
    move->i = i;
    move->j = j;
    move->i_destination = i_destination;
    move->j_destination = j_destination;
    move->is_vertical_wall = is_vertical_wall;
    return move;
}

move_t* create_piece_placement_move (int i, int j) {
    return create_move(PIECE_PLACEMENT, i, j, -1, -1, false);
}

move_t* create_deplacement_move (int i_start, int j_start, int i_end, int j_end) {
    return create_move(DEPLACEMENT, i_start, j_start, i_end, j_end, false);
}

move_t* create_wall_placement_move (int cell_i, int cell_j, int wall_i, int wall_j, bool is_vertical_wall) {
    return create_move(WALL_PLACEMENT, cell_i, cell_j, wall_i, wall_j, is_vertical_wall);
}

move_t* create_skip_move (int i, int i_destination) {
    return create_move(SKIP, i, -1, i_destination, -1, false);
}

void record_move (record_t** record_ptr, move_t* move) {
    record_t* head = SDL_malloc(sizeof(record_t));
    head->last_move = move;
    head->previous = *record_ptr;
    *record_ptr = head; 
}

void removed_last_move (record_t** record_ptr) {
    if (*record_ptr != NULL) {
        record_t* to_free = *record_ptr;
        *record_ptr = (*record_ptr)->previous;
        SDL_free(to_free->last_move);
        SDL_free(to_free);
    }
}

void destroy_record (record_t* record) {
    if (record != NULL) {
        destroy_record(record->previous);
        SDL_free(record->last_move);
        SDL_free(record);
    }
}

char* cell_coords_to_format_string(int i, int j) {
    char* format_string;
    SDL_asprintf(&format_string, "%c%d", 'A'+i, j+1);
    return format_string;
}

char* move_to_format_string (move_t* move) {
    char* start;
    char* end;
    char* format_string;
    switch (move->type) {
        case PIECE_PLACEMENT:
            return cell_coords_to_format_string(move->i, move->j);
            
        case DEPLACEMENT:
            start = cell_coords_to_format_string(move->i, move->j);
            end = cell_coords_to_format_string(move->i_destination, move->j_destination);   
            SDL_asprintf(&format_string, "%s->%s", start, end);
            SDL_free(start);
            SDL_free(end);
            return format_string;
            
        case WALL_PLACEMENT:
            SDL_asprintf(&format_string, "%c %d %d", move->is_vertical_wall?'v':'h', move->i, move->j);
            return format_string;

        case SKIP:
            SDL_asprintf(&format_string, "SKIP");
            return format_string;

        default:
            SDL_Log("Unknown move type");
            return NULL;
    }
}

// void write_record_to_file (record_t* record, char* filename) {
//     // TODO
// }

// move_t* format_string_to_move (char* format_string) {
//     /* 
//     4 possibilties
//     | just cell coords -> it's a placement move
//     | two cell coords separate by an arrow -> it's a deplacement move 
//     | h or v followed by wall coords -> it's a wall placement wall
//     | other -> it's garbage, the record file is corrupted
//     */

//     // TODO
//     return NULL;
// }

// record_t* read_record_from_file (char* filename) {
//     // TODO
//     return NULL;
// }