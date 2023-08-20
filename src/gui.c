#include "gui.h"
#include <time.h>

static void check_sdl_ptr(void *sdl_ptr) {
    if (!sdl_ptr) {
        fprintf(stderr, "Error: SDL2 null pointer (SDL error: \"%s\")\n",
                SDL_GetError());
        exit(1);
    }
}

Gui *gui_alloc() {
    Gui *new_gui = malloc(sizeof(*new_gui));
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Init(SDL_INIT_EVENTS);

    new_gui->window = SDL_CreateWindow("Game of life", 0, 0, 800, 600, 0);
    new_gui->window_width = 800;
    new_gui->window_height = 600;
    check_sdl_ptr(new_gui->window);

    new_gui->renderer = SDL_CreateRenderer(new_gui->window, 0, 0);
    check_sdl_ptr(new_gui->renderer);
    SDL_RenderClear(new_gui->renderer);

    new_gui->running = true;
    new_gui->center_grid = true;
    new_gui->simulaton_running = false;
    new_gui->restart = false;
    new_gui->drag_grid = false;
    new_gui->shift_pressed = false;
    new_gui->left_click_pressed = false;
    new_gui->right_click_pressed = false;
    new_gui->current_zoom = 1.f;
    new_gui->view_position.x = 0;
    new_gui->view_position.y = 0;

    new_gui->gol_state = golstate_alloc();
    new_gui->there_is_something_to_draw = true;

    SDL_RenderPresent(new_gui->renderer);
    return new_gui;
}

void gui_destroy(Gui *gui) {
    golstate_destroy(&gui->gol_state);
    SDL_DestroyWindow(gui->window);
    SDL_DestroyRenderer(gui->renderer);
    SDL_Quit();
    free(gui);
}

static void gui_center_grid(Gui *gui) {
    float grid_width = CELL_WIDTH_BASE * gui->current_zoom * GRID_WIDTH;
    int x_offset = (gui->window_width - grid_width) / 2;
    int y_offset = (gui->window_height - grid_width) / 2;
    gui->view_position.x = x_offset;
    gui->view_position.y = y_offset;
}

enum e_zoom { ZOOM_INCREASE, ZOOM_DECREASE };
static void gui_handle_zoom(Gui *gui, enum e_zoom e_zoom_flag) {
    switch (e_zoom_flag) {
    case ZOOM_INCREASE:
        if (gui->current_zoom + ZOOM_STEP < MAX_ZOOM)
            gui->current_zoom += ZOOM_STEP;
        break;
    case ZOOM_DECREASE:
        if (gui->current_zoom - ZOOM_STEP > MIN_ZOOM)
            gui->current_zoom -= ZOOM_STEP;
        break;
    default:
        break;
    }
}

static void gui_process_key_press_events(Gui *gui, SDL_Event *e) {
    switch (e->key.keysym.sym) {
    case SDLK_ESCAPE:
    case SDLK_q:
        gui->running = false;
        break;
    case SDLK_PLUS:
        gui_handle_zoom(gui, ZOOM_INCREASE);
        break;
    case SDLK_MINUS:
        gui_handle_zoom(gui, ZOOM_DECREASE);
        break;
    case SDLK_UP:
        gui->view_position.y += MOVEMENT_STEP;
        break;
    case SDLK_DOWN:
        gui->view_position.y -= MOVEMENT_STEP;
        break;
    case SDLK_RIGHT:
        gui->view_position.x -= MOVEMENT_STEP;
        break;
    case SDLK_LEFT:
        gui->view_position.x += MOVEMENT_STEP;
        break;
    case SDLK_LSHIFT:
        gui->shift_pressed = true;
        break;
    case SDLK_SPACE:
        if (gui->shift_pressed) {
            gui->step_to_next_generation = true;
            puts("Info: Step to the next generation...");
        } else {
            gui->simulaton_running = !gui->simulaton_running;
            printf("Info: %s simulation...\n",
                   gui->simulaton_running ? "Starting" : "Stoping");
        }
        break;
    case SDLK_r:
        gui->restart = true;
        gui->simulaton_running = false;
        puts("Info: Restarting...");
        break;
    case SDLK_c:
        gui->center_grid = true;
        puts("Info: Centering grid...");
        break;
    default:
        break;
    }
}

static void gui_process_mouse_click_event(Gui *gui, SDL_Event *e) {
    Point mouse_position = {0};
    switch (e->button.button) {
    case SDL_BUTTON_LEFT:
        gui->left_click_pressed = true;
        if (!gui->shift_pressed) {
            mouse_position.x = e->button.x;
            mouse_position.y = e->button.y;
            int mouse_in_virtual_grid = gui_point2d_to_grid1d(
                mouse_position, gui->view_position, GRID_WIDTH, CELL_WIDTH_BASE,
                gui->current_zoom);
            golstate_arbitrary_give_birth_cell(gui->gol_state,
                                               mouse_in_virtual_grid);
        } else {
            gui->drag_grid = true;
            gui->initial_mouse_drag_position.x = e->button.x;
            gui->initial_mouse_drag_position.y = e->button.y;
        }
        break;
    case SDL_BUTTON_RIGHT:
        gui->right_click_pressed = true;
        mouse_position.x = e->button.x;
        mouse_position.y = e->button.y;
        int mouse_in_virtual_grid = gui_point2d_to_grid1d(
            mouse_position, gui->view_position, GRID_WIDTH, CELL_WIDTH_BASE,
            gui->current_zoom);
        golstate_arbitrary_kill_cell(gui->gol_state, mouse_in_virtual_grid);
        break;
    case SDL_BUTTON_MIDDLE:
        gui->drag_grid = true;
        gui->initial_mouse_drag_position.x = e->button.x;
        gui->initial_mouse_drag_position.y = e->button.y;
        break;
    default:
        break;
    }
}

static void gui_process_events(Gui *gui) {
    SDL_Event e;
    gui->there_is_something_to_draw = true;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
        case SDL_QUIT:
            gui->running = false;
            break;
        case SDL_KEYDOWN:
            gui_process_key_press_events(gui, &e);
            break;
        case SDL_KEYUP:
            switch (e.key.keysym.sym) {
            case SDLK_LSHIFT:
                gui->shift_pressed = false;
                break;
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
            gui_process_mouse_click_event(gui, &e);
            break;
        case SDL_MOUSEBUTTONUP:
            switch (e.button.button) {
            case SDL_BUTTON_LEFT:
            case SDL_BUTTON_RIGHT:
                gui->left_click_pressed = false;
                gui->right_click_pressed = false;
                // fall through
            case SDL_BUTTON_MIDDLE:
                gui->drag_grid = false;
                break;
            }
            break;
        case SDL_MOUSEWHEEL:
            if (e.wheel.y > 0) {
                gui_handle_zoom(gui, ZOOM_INCREASE);
            } else {
                gui_handle_zoom(gui, ZOOM_DECREASE);
            }
            break;
        case SDL_MOUSEMOTION:
            if (gui->drag_grid) {
                gui->view_position.x += e.motion.xrel;
                gui->view_position.y += e.motion.yrel;
            }
            if (gui->left_click_pressed && !gui->shift_pressed) {
                Point mouse_position;
                mouse_position.x = e.button.x;
                mouse_position.y = e.button.y;
                int mouse_in_virtual_grid = gui_point2d_to_grid1d(
                    mouse_position, gui->view_position, GRID_WIDTH,
                    CELL_WIDTH_BASE, gui->current_zoom);
                golstate_arbitrary_give_birth_cell(gui->gol_state,
                                                   mouse_in_virtual_grid);
            }
            if (gui->right_click_pressed && !gui->shift_pressed) {
                Point mouse_position;
                mouse_position.x = e.button.x;
                mouse_position.y = e.button.y;
                int mouse_in_virtual_grid = gui_point2d_to_grid1d(
                    mouse_position, gui->view_position, GRID_WIDTH,
                    CELL_WIDTH_BASE, gui->current_zoom);
                golstate_arbitrary_kill_cell(gui->gol_state,
                                             mouse_in_virtual_grid);
            }
            break;
        default:
            break;
        }
    }
}

static void gui_update(Gui *gui) {
    if (gui->restart) {
        golstate_restart(gui->gol_state);
        gui->restart = false;
    }
    if (gui->center_grid) {
        gui_center_grid(gui);
        gui->center_grid = false;
    }
    if (gui->simulaton_running) {
        golstate_analyze_generation(gui->gol_state);
        golstate_next_generation(gui->gol_state);
        if (gui->gol_state->population == 0) {
            gui->simulaton_running = false;
            printf("Info: No population, stoping simulation...\n");
        }
    }
    if (gui->step_to_next_generation) {
        golstate_analyze_generation(gui->gol_state);
        golstate_next_generation(gui->gol_state);
        gui->step_to_next_generation = false;
    }
}

static void gui_draw_grid(Gui *gui) {
    float final_cell_width = CELL_WIDTH_BASE * gui->current_zoom;
    SDL_SetRenderDrawColor(gui->renderer, 20, 20, 20, 255);

    float start_x = gui->view_position.x;
    float start_y = gui->view_position.y;

    float end_x = fmin(gui->window_width,
                       GRID_WIDTH * (CELL_WIDTH_BASE * gui->current_zoom) +
                           gui->view_position.x);
    float end_y = fmin(gui->window_height,
                       GRID_WIDTH * (CELL_WIDTH_BASE * gui->current_zoom) +
                           gui->view_position.y);

    for (float x = start_x; x <= end_x + 1; x += final_cell_width) {
        if (x <= gui->window_width)
            SDL_RenderDrawLineF(gui->renderer, x, start_y, x, end_y);
    }

    for (float y = start_y; y <= end_y + 1; y += final_cell_width) {
        if (y <= gui->window_height)
            SDL_RenderDrawLineF(gui->renderer, start_x, y, end_x, y);
    }
}

static void gui_draw_cell(Gui *gui, Point position) {
    SDL_Rect rect;
    float cell_side_f = (float)CELL_WIDTH_BASE * gui->current_zoom;
    rect.w = rect.h = (float)CELL_WIDTH_BASE * gui->current_zoom;
    rect.x = (position.x * cell_side_f) + gui->view_position.x;
    rect.y = (position.y * cell_side_f) + gui->view_position.y;

    if (rect.x > gui->window_width ||
        rect.x + (CELL_WIDTH_BASE * gui->current_zoom) < 0)
        return;
    if (rect.y > gui->window_height ||
        rect.y + (CELL_WIDTH_BASE * gui->current_zoom) < 0)
        return;

    SDL_SetRenderDrawColor(gui->renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(gui->renderer, &rect);
}

static void gui_render(Gui *gui) {
    SDL_SetRenderDrawColor(gui->renderer, 0, 0, 0, 255);
    SDL_RenderClear(gui->renderer);
    gui_draw_grid(gui);

    Node *current = gui->gol_state->alive_cells;
    while (current) {
        Point gui_point =
            grid1d_to_point2d(current->data, GRID_WIDTH, GRID_SIZE);
        gui_draw_cell(gui, gui_point);
        current = current->next;
    }

    SDL_RenderPresent(gui->renderer);
    gui->there_is_something_to_draw = false;
}

void gui_run(Gui *gui) {
    uint32_t current_time = SDL_GetTicks();
    uint32_t last_frame_time = current_time;
    while (gui->running) {

        current_time = SDL_GetTicks();
        uint32_t elapsed_time = current_time - last_frame_time;
        if (elapsed_time <= FPS)
            continue;

        gui_process_events(gui);
        gui_update(gui);

        if (gui->there_is_something_to_draw) {
            gui_render(gui);
        }
        SDL_Delay(10);
    }
}
