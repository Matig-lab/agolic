#include "node.h"
#include "point.h"
#include <SDL2/SDL.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define GRID_WIDTH 100
#define GRID_SIZE pow(GRID_WIDTH, 2)

typedef struct {
    bool *grid;
    bool *analyzed_grid_cells;
    Node *alive_cells;
    Node *dying_cells;
    Node *becoming_alive_cells;
    int population, generation;
    bool is_generation_analyzed;
} GolState;

#define MIN_NEIGHBORS_TO_SURVIVE 2
#define MAX_NEIGHBORS_TO_STAY_ALIVE 3
#define NEIGHBORS_TO_REPRODUCE 3

GolState *golstate_alloc() {
    GolState *gol_state = malloc(sizeof(*gol_state));
    gol_state->grid = malloc(sizeof(bool) * GRID_SIZE);
    gol_state->analyzed_grid_cells = malloc(sizeof(bool) * GRID_SIZE);
    for (int i = 0; i < GRID_SIZE; i++) {
        gol_state->grid[i] = false;
        gol_state->analyzed_grid_cells[i] = false;
    }
    gol_state->alive_cells = NULL;
    gol_state->dying_cells = NULL;
    gol_state->becoming_alive_cells = NULL;
    gol_state->population = 0;
    gol_state->generation = 0;
    gol_state->is_generation_analyzed = false;
    return gol_state;
}

void golstate_destroy(GolState *gol_state) {
    free(gol_state->grid);
    node_destroy_all(&gol_state->alive_cells);
    node_destroy_all(&gol_state->dying_cells);
    node_destroy_all(&gol_state->becoming_alive_cells);
    free(gol_state);
}

void golstate_restart(GolState *gol_state) {
    node_destroy_all(&gol_state->alive_cells);
    node_destroy_all(&gol_state->becoming_alive_cells);
    node_destroy_all(&gol_state->dying_cells);
    gol_state->population = 0;
    gol_state->generation = 0;
    for (int i = 0; i < GRID_SIZE; i++) {
        gol_state->grid[i] = false;
        gol_state->analyzed_grid_cells[i] = false;
    }
}

void golstate_arbitrary_give_birth_cell(GolState *gol_state, int grid_index) {
    if (grid_index < 0 || grid_index >= GRID_SIZE) {
        return;
    }
    if (gol_state->grid[grid_index]) {
        return;
    }
    node_append(&gol_state->alive_cells, grid_index);
    gol_state->grid[grid_index] = true;
    gol_state->population++;
}

void golstate_arbitrary_kill_cell(GolState *gol_state, int grid_index) {
    if (grid_index < 0 || grid_index >= GRID_SIZE)
        return;
    if (!gol_state->grid[grid_index])
        return;
    node_delete_by_data(&gol_state->alive_cells, grid_index);
    gol_state->grid[grid_index] = false;
    gol_state->population--;
}

#define START_IS_IN_CORRECT_INDEX(s, l)                                        \
    (s >= 0 && (s == 0 ? 0 : s / GRID_WIDTH) == l)
int golstate_compute_neighborhood_start(int neighborhood_center,
                                        int line_of_neighborhood_center) {
    int start;
    for (int offset = 1; offset >= -1; offset--) {
        start = neighborhood_center - (GRID_WIDTH + offset);
        if (START_IS_IN_CORRECT_INDEX(start, line_of_neighborhood_center - 1)) {
            return start;
        }
    }
    start = neighborhood_center - 1;
    if (START_IS_IN_CORRECT_INDEX(start, line_of_neighborhood_center)) {
        return start;
    }
    return neighborhood_center;
}

#define END_IS_IN_CORRECT_INDEX(s, l)                                          \
    (s < GRID_SIZE && (s == 0 ? 0 : s / GRID_WIDTH) == l)
int golstate_compute_neighborhood_end(int neighborhood_center,
                                      int line_of_neighborhood_center) {
    int end;
    for (int offset = 1; offset >= -1; offset--) {
        end = neighborhood_center + (GRID_WIDTH + offset);
        if (END_IS_IN_CORRECT_INDEX(end, line_of_neighborhood_center + 1)) {
            return end;
        }
    }
    end = neighborhood_center + 1;
    if (END_IS_IN_CORRECT_INDEX(end, line_of_neighborhood_center)) {
        return end;
    }
    return neighborhood_center;
}

void golstate_neighbor_analysis(GolState *gol_state, int neighborhood_center,
                                Node **list_of_indexes_dst,
                                int *life_in_neighborhood,
                                bool gather_indexes) {
    if (neighborhood_center < 0 || neighborhood_center >= GRID_SIZE)
        return;

    *life_in_neighborhood = 0;
    if (gather_indexes && *list_of_indexes_dst) {
        node_destroy_all(list_of_indexes_dst);
    }

    int neighborhood_center_line =
        neighborhood_center == 0 ? 0 : neighborhood_center / GRID_WIDTH;
    int neighborhood_start = golstate_compute_neighborhood_start(
        neighborhood_center, neighborhood_center_line);
    int neighborhood_end = golstate_compute_neighborhood_end(
        neighborhood_center, neighborhood_center_line);

    int neighborhood_end_of_first_line =
        neighborhood_center - (GRID_WIDTH - 1);
    int neighborhood_end_of_second_line = neighborhood_center + 1;
    int jump_to_start_of_nextline = GRID_WIDTH - 4;
    int current_line =
        neighborhood_center_line - 1 < 0 ? 0 : neighborhood_center_line - 1;
    for (int i = neighborhood_start; i <= neighborhood_end; i++) {
        if (i < 0 || i == neighborhood_center)
            continue;
        if (i >= GRID_SIZE)
            break;
        if (i == neighborhood_end_of_first_line + 1 ||
            i == neighborhood_end_of_second_line + 1) {
            i += jump_to_start_of_nextline;
            current_line++;
            continue;
        }

        if (i / GRID_WIDTH != current_line) {
            continue;
        }
        if (gather_indexes)
            node_append(list_of_indexes_dst, i);
        if (gol_state->grid[i])
            (*life_in_neighborhood)++;
    }
    return;
}

bool golstate_cell_stays_alive(int life_in_neighborhood) {
    return life_in_neighborhood >= MIN_NEIGHBORS_TO_SURVIVE &&
           life_in_neighborhood <= MAX_NEIGHBORS_TO_STAY_ALIVE;
}

bool golstate_dead_cell_becomes_alive(int life_in_neighborhood) {
    return life_in_neighborhood == NEIGHBORS_TO_REPRODUCE;
}

void golstate_cleanup(GolState *gol_state) {
    Node *current = gol_state->alive_cells;
    Node *prev = NULL;

    for (int i = 0; i < GRID_SIZE; i++) {
        gol_state->analyzed_grid_cells[i] = false;
    }

    while (current) {
        if (!gol_state->grid[current->data]) {
            Node *tmp = current;
            if (prev) {
                prev->next = current->next;
            } else {
                gol_state->alive_cells = current->next;
            }
            current = current->next;
            free(tmp);
        } else {
            prev = current;
            current = current->next;
        }
    }
}

void golstate_analyze_generation(GolState *gol_state) {
    // Analize current cell
    Node *current_cell = gol_state->alive_cells;
    while (current_cell) {

        int life_in_neighborhood = 0;
        Node *neighborhood = NULL;
        golstate_neighbor_analysis(gol_state, current_cell->data, &neighborhood,
                                   &life_in_neighborhood, true);
        if (!golstate_cell_stays_alive(life_in_neighborhood)) {
            node_append(&gol_state->dying_cells, current_cell->data);
        }

        // Analize each dead cell in neighborhood
        Node *current_neighborhood_cell = neighborhood;
        while (current_neighborhood_cell) {
            if (gol_state->grid[current_neighborhood_cell->data] ||
                gol_state
                    ->analyzed_grid_cells[current_neighborhood_cell->data]) {
                current_neighborhood_cell = current_neighborhood_cell->next;
                continue;
            }

            golstate_neighbor_analysis(gol_state,
                                       current_neighborhood_cell->data, NULL,
                                       &life_in_neighborhood, false);

            if (golstate_dead_cell_becomes_alive(life_in_neighborhood)) {
                node_append(&gol_state->becoming_alive_cells,
                            current_neighborhood_cell->data);
                gol_state
                    ->analyzed_grid_cells[current_neighborhood_cell->data] =
                    true;
            }

            current_neighborhood_cell = current_neighborhood_cell->next;
        }
        current_cell = current_cell->next;
    }
    gol_state->is_generation_analyzed = true;
}

void golstate_next_generation(GolState *gol_state) {
    if (!gol_state->is_generation_analyzed)
        return;
    Node *current = gol_state->dying_cells;
    while (current) {
        gol_state->grid[current->data] = false;
        gol_state->population--;
        current = current->next;
    }
    node_destroy_all(&gol_state->dying_cells);

    current = gol_state->becoming_alive_cells;
    while (current) {
        gol_state->grid[current->data] = true;
        gol_state->population++;
        current = current->next;
    }
    node_concat(gol_state->alive_cells, &gol_state->becoming_alive_cells);
    node_destroy_all(&gol_state->becoming_alive_cells);
    golstate_cleanup(gol_state);
    gol_state->generation++;
    gol_state->is_generation_analyzed = false;
}

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

typedef struct {
    Point position;
    float side;
} Cell;

#define CELL_WIDTH_BASE 15
#define MAX_ZOOM 2.f
#define MIN_ZOOM .05f
#define FPS 60
#define ZOOM_STEP .01f
#define MOVEMENT_STEP 5

void check_sdl_ptr(void *sdl_ptr) {
    if (!sdl_ptr) {
        fprintf(stderr, "error: sdl null pointer\n");
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
    golstate_destroy(gui->gol_state);
    SDL_DestroyWindow(gui->window);
    SDL_DestroyRenderer(gui->renderer);
    SDL_Quit();
    free(gui);
}

double gui_get_performance(void (*run)(Gui *), Gui *gui) {
    clock_t start, end;
    double time_elapsed = 0;
    start = clock();
    (*run)(gui);
    end = clock();
    time_elapsed = (double)end - start;
    return time_elapsed;
}

void gui_center_grid(Gui *gui) {
    float grid_width = CELL_WIDTH_BASE * gui->current_zoom * GRID_WIDTH;
    int x_offset = (gui->window_width - grid_width) / 2;
    int y_offset = (gui->window_height - grid_width) / 2;
    gui->view_position.x = x_offset;
    gui->view_position.y = y_offset;
}
int gui_point_to_virtual_grid_index(Gui *gui, Point gui_point) {
    gui_point.x -= gui->view_position.x;
    gui_point.y -= gui->view_position.y;
    gui_point.x -= fmod(gui_point.x, CELL_WIDTH_BASE * gui->current_zoom);
    gui_point.y -= fmod(gui_point.y, CELL_WIDTH_BASE * gui->current_zoom);

    int grid_index_x = gui_point.x / (CELL_WIDTH_BASE * gui->current_zoom);
    int grid_index_y =
        (gui_point.y / (CELL_WIDTH_BASE * gui->current_zoom)) * GRID_WIDTH;

    if (grid_index_x < 0 || grid_index_x >= GRID_WIDTH)
        return -1;

    return grid_index_y + grid_index_x;
}

enum e_zoom { ZOOM_INCREASE, ZOOM_DECREASE };
void gui_handle_zoom(Gui *gui, enum e_zoom e_zoom_flag) {
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

void gui_process_key_press_events(Gui *gui, SDL_Event *e) {
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

void gui_process_mouse_click_event(Gui *gui, SDL_Event *e) {
    Point mouse_position = {0};
    switch (e->button.button) {
    case SDL_BUTTON_LEFT:
        gui->left_click_pressed = true;
        if (!gui->shift_pressed) {
            mouse_position.x = e->button.x;
            mouse_position.y = e->button.y;
            int mouse_in_virtual_grid =
                gui_point_to_virtual_grid_index(gui, mouse_position);
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
        int mouse_in_virtual_grid =
            gui_point_to_virtual_grid_index(gui, mouse_position);
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

void gui_process_events(Gui *gui) {
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
                int mouse_in_virtual_grid =
                    gui_point_to_virtual_grid_index(gui, mouse_position);
                golstate_arbitrary_give_birth_cell(gui->gol_state,
                                                   mouse_in_virtual_grid);
            }
            if (gui->right_click_pressed && !gui->shift_pressed) {
                Point mouse_position;
                mouse_position.x = e.button.x;
                mouse_position.y = e.button.y;
                int mouse_in_virtual_grid =
                    gui_point_to_virtual_grid_index(gui, mouse_position);
                golstate_arbitrary_kill_cell(gui->gol_state,
                                             mouse_in_virtual_grid);
            }
            break;
        default:
            break;
        }
    }
}

void gui_update(Gui *gui) {
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

void gui_draw_grid(Gui *gui) {
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

void gui_draw_cell(Gui *gui, Point position) {
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

void gui_render(Gui *gui) {
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

    Point p = {150, 150};
    gui->gol_state->grid[point2d_to_grid1d(p, GRID_WIDTH, GRID_SIZE)] = true;

    uint32_t current_time = SDL_GetTicks();
    uint32_t last_frame_time = current_time;
    while (gui->running) {

        current_time = SDL_GetTicks();
        uint32_t elapsed_time = current_time - last_frame_time;
        if (elapsed_time <= FPS)
            continue;

        gui_process_events(gui);
        gui_update(gui);
        // double update_perf = gui_get_performance(gui_update, gui);

        if (gui->there_is_something_to_draw) {
            gui_render(gui);
            // double render_perf = gui_get_performance(gui_render, gui);
            // printf("Info: GUI updated in %f seconds\n", update_perf / 1000);
            // printf("Info: GUI rendered in %f seconds\n", render_perf / 1000);
        }
        SDL_Delay(10);
    }
}

int main(void) {

    Gui *gui = gui_alloc();
    gui_run(gui);
    gui_destroy(gui);

    return 0;
}
