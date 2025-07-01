/*            INCLUDES              */
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

#include "../include/drawing.h"
#include "../include/game_logic.h"
#include "../include/input_managing.h"


/*            INITIALISATION OF THE APPLICATION              */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    // define software metadata
    SDL_SetAppMetadata("Walls", "0.1", "WALLS");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Log parameters
    for (int i = 0; i < argc; i++) SDL_Log("%s\n", argv[i]);

    // initialisation of appstate
    if (!init_appstate (appstate)) {
        SDL_Log("Appstate initialisation failed\n");
        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE;
}


/*            MANAGE INPUT              */
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

                case SDLK_R:
                    cancel_last_move(appstate);
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
            window_update_reaction(appstate, "maximized");
            break;

        case SDL_EVENT_WINDOW_MINIMIZED:
            window_update_reaction(appstate, "minimized");
            break;

    }

    return SDL_APP_CONTINUE; 
}

/*            ITERATION LOGIC             */
SDL_AppResult SDL_AppIterate(void *appstate) {
    appstate_t* as = (appstate_t*)appstate;
    if (!as->has_to_be_render) {
        return SDL_APP_CONTINUE;
    }

    update_display_parameters(appstate);

    // Draw the background
    draw_background(appstate);

    // Draw the board
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

    SDL_RenderPresent(as->renderer);

    as->has_to_be_render = false;
    return SDL_APP_CONTINUE; 
}


/*          QUIT THE APPLICATION            */
void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    if (result == SDL_APP_FAILURE) SDL_Log("Something goes wrong...\n");

    free_appstate(appstate);

    SDL_Quit();
}