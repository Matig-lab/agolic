#ifndef _GUI_H_
#define _GUI_H_

#include "golstate.h"
#include "point.h"

#include <SDL2/SDL.h>

typedef struct {
    SDL_Window *window;
    int window_width, window_height;
    SDL_Renderer *renderer;
    bool running, there_is_something_to_draw, simulaton_running, restart,
        center_grid, shift_pressed, drag_grid, left_click_pressed,
        step_to_next_generation, right_click_pressed;
    Point initial_mouse_drag_position;
    float current_zoom;
    Point view_position;
    GolState *gol_state;
} Gui;

#define CELL_WIDTH_BASE 15
#define MAX_ZOOM 2.f
#define MIN_ZOOM .05f
#define FPS 60
#define ZOOM_STEP .01f
#define MOVEMENT_STEP 5

Gui *gui_alloc();
void gui_destroy(Gui *gui);
void gui_run(Gui *gui);

#endif // _GUI_H_
