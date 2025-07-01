#ifndef MOVES_RECORDING_H
#define MOVES_RECORDING_H MOVES_RECORDING_H

#include <SDL3/SDL.h>

#define PIECE_PLACEMENT 0
#define DEPLACEMENT 1
#define WALL_PLACEMENT 2
#define SKIP 3

typedef struct move_s {
    int type;
    int i;
    int j;
    int i_destination;
    int j_destination;
    bool is_vertical_wall;
} move_t;

typedef struct record_s {
    move_t* last_move;
    struct record_s* previous;
} record_t;

/*
NB_PLAYERS
PIECE_PLACEMENT_1
...
PIECE_PLACEMENT_<NB_PLAYERS>
PIECE_PLACEMENT_1
...
PIECE_PLACEMENT_<NB_PLAYERS>
START->END WALL_DIRECTION

*/

/* example */
/*
3
A5
B7
C4
A7
G5
G4
A5->A6
h . .
B7->C6
v 
...
*/

move_t* create_move (int type, int i, int j, int i_destination, int j_destination, bool is_vertical_wall);
move_t* create_piece_placement_move (int i, int j);
move_t* create_deplacement_move (int i_start, int j_start, int i_end, int j_end);
move_t* create_wall_placement_move (int cell_i, int cell_j, int wall_i, int wall_j, bool is_vertical_wall);
move_t* create_skip_move (int i, int i_destination);
void record_move (record_t** record_ptr, move_t* move);
void removed_last_move (record_t** record_ptr);
void destroy_record (record_t* record);
char* cell_coords_to_format_string(int i, int j);
char wall_direction_to_char (int wall_direction);
char* move_to_format_string (move_t* move);
void write_record_to_file (record_t* record, char* filename);
move_t* format_string_to_move (char* format_string);
record_t* read_record_from_file (char* filename);

#endif