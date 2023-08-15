#include <SDL2/SDL.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Node Node;
struct Node {
    int data;
    Node *next;
};

Node *node_alloc(int data) {
    Node *new_node = malloc(sizeof(*new_node));
    new_node->next = NULL;
    new_node->data = data;
    return new_node;
}

void node_append(Node **head, int data) {
    Node *new_node = node_alloc(data);
    if (!*head) {
        *head = new_node;
        return;
    }

    Node *current = *head;
    while (current->next) {
        current = current->next;
    }
    current->next = new_node;
}

void node_insert_head(Node **head, int data) {
    Node *new_node = node_alloc(data);
    if (!*head) {
        *head = new_node;
        return;
    }

    new_node->next = (*head);
    *head = new_node;
}

void node_delete_by_index(Node **head, int index) {
    if (!*head || index < 0)
        return;

    int i = 0;
    Node *current = *head;
    Node *last = current;

    if (i == 0) {
        *head = current->next;
        free(current);
        return;
    }

    while (current->next) {
        if (i == index) {
            last->next = current->next;
            free(current);
            break;
        }
        last = current;
        current = current->next;
    }
}

void node_delete_by_data(Node **head, int data) {
    if (!*head)
        return;

    if ((*head)->data == data) {
        Node *temp = *head;
        *head = (*head)->next;
        free(temp);
        return;
    }

    Node *current = *head;
    Node *last = current;
    while (current && current->data != data) {
        last = current;
        current = current->next;
    }
    last->next = current->next;
    free(current);
}

void node_delete_all(Node **head) {
    if (!*head)
        return;

    Node *current = *head;
    while (current) {
        Node *next = current->next;
        free(current);
        current = next;
    }
    *head = NULL;
}

#define GRID_SIDE 20
#define GRID_AREA pow(GRID_SIDE, 2)
typedef struct {
    float x, y;
} Point;

int point_to_grid_index(Point p) {
    int index = 0;
    index = GRID_SIDE * p.y;
    index += p.x;
    if (index < 0 || index < GRID_AREA)
        return -1;
    return index;
}

Point grid_index_to_point(int grid_index) {
    Point p = {-1, -1};
    if (grid_index < 0 || grid_index >= GRID_AREA)
        return p;
    p.y = (float)floorf((float)grid_index / GRID_SIDE);
    p.x = (float)floorf((grid_index % GRID_SIDE));
    return p;
}

typedef struct {
    bool *grid;
    Node *alive_cells;
    Node *dying_cells;
    Node *becoming_alive_cells;
    int population, generation;
    bool generation_analized;
} GolState;

#define MIN_CELLS_TO_NOT_BE_ISOLATED 2
#define MAX_CELLS_TO_NOT_BE_OVERPOPULATED 3
#define MIN_CELLS_TO_PRODUCE_LIFE 3
GolState *golstate_alloc() {
    GolState *gol_state = malloc(sizeof(*gol_state));
    gol_state->grid = malloc(sizeof(bool) * GRID_AREA);
    for (int i = 0; i < GRID_AREA; i++) {
        gol_state->grid[i] = false;
    }
    gol_state->alive_cells = NULL;
    gol_state->dying_cells = NULL;
    gol_state->becoming_alive_cells = NULL;
    gol_state->population = 0;
    gol_state->generation = 0;
    gol_state->generation_analized = false;
    return gol_state;
}

void golstate_destroy(GolState *gol_state) {
    free(gol_state->grid);
    node_delete_all(&gol_state->alive_cells);
    node_delete_all(&gol_state->dying_cells);
    node_delete_all(&gol_state->becoming_alive_cells);
    free(gol_state);
}

void golstate_arbitrary_give_birth_cell(GolState *gol_state, int grid_index) {
    if (grid_index < 0 || grid_index >= GRID_AREA) {
        printf("info: cell rejected: index out of bounds (%d)\n", grid_index);
        return;
    }
    if (gol_state->grid[grid_index]) {
        printf("info: cell rejected: there is a cell there (%d)\n", grid_index);
        return;
    }
    printf("info: new cell at %d\n", grid_index);
    node_append(&gol_state->alive_cells, grid_index);
    gol_state->grid[grid_index] = true;
    gol_state->population++;
}

void golstate_arbitrary_kill_cell(GolState *gol_state, int grid_index) {
    if (grid_index < 0 || grid_index >= GRID_AREA)
        return;
    if (!gol_state->grid[grid_index])
        return;
    node_append(&gol_state->dying_cells, grid_index);
}

Node *golstate_neighboring_cells_index_list(int neighborhood_center) {
    if (neighborhood_center < 0 || neighborhood_center >= GRID_AREA)
        return NULL;

    Node *head = NULL;
    int neighborhood_start = neighborhood_center - (GRID_SIDE - 1);
    int neighborhood_end = neighborhood_center + (GRID_SIDE + 1);
    int neighborhood_end_of_first_line = neighborhood_center - (GRID_AREA + 1);
    int neighborhood_end_of_second_line = neighborhood_center + 1;
    int jump_to_start_of_nextline = GRID_SIDE - 2;

    for (int i = neighborhood_start; i <= neighborhood_end; i++) {
        if (neighborhood_center < 0 || i == neighborhood_center)
            continue;
        if (i > GRID_AREA)
            break;
        if (i == neighborhood_end_of_first_line ||
            i == neighborhood_end_of_second_line)
            i += jump_to_start_of_nextline;
        node_append(&head, i);
    }
    return head;
}

int golstate_sum_of_neighborhood_lives(GolState *gol_state,
                                       Node *cell_neighborhood) {
    int sum = 0;
    Node *current_cell = cell_neighborhood;
    while (current_cell) {
        if (gol_state->grid[current_cell->data])
            sum++;
        current_cell = current_cell->next;
    }
    return sum;
}

void golstate_analize_state(GolState *gol_state) {
    // Analize current cell
    Node *current_cell = gol_state->alive_cells;
    while (current_cell) {
        Node *current_cell_neighborhood =
            golstate_neighboring_cells_index_list(current_cell->data);
        int life_in_neighborhood = golstate_sum_of_neighborhood_lives(
            gol_state, current_cell_neighborhood);
        if (life_in_neighborhood < MIN_CELLS_TO_NOT_BE_ISOLATED ||
            life_in_neighborhood > MAX_CELLS_TO_NOT_BE_OVERPOPULATED) {
            node_append(&gol_state->dying_cells, current_cell->data);
        }
        Node *current_neighboring_cell = current_cell_neighborhood;

        // Analize each dead cells in neighborhood
        while (current_neighboring_cell) {
            if (gol_state->grid[current_neighboring_cell->data])
                continue;
            Node *current_neighboring_cell_neighborhood =
                golstate_neighboring_cells_index_list(current_neighboring_cell->data);
            int life_in_neghboring_neighborhood =
                golstate_sum_of_neighborhood_lives(
                    gol_state, current_neighboring_cell_neighborhood);
            if (life_in_neghboring_neighborhood >= MIN_CELLS_TO_PRODUCE_LIFE) {
                node_append(&gol_state->becoming_alive_cells,
                            current_neighboring_cell->data);
            }
            current_neighboring_cell = current_neighboring_cell->next;
        }
        current_cell = current_cell->next;
    }
    gol_state->generation_analized = true;
}

void golstate_next_generation(GolState *gol_state) {
    if (!gol_state->generation_analized)
        return;
    Node *current = gol_state->dying_cells;
    while (current) {
        node_delete_by_data(&gol_state->alive_cells, current->data);
        gol_state->population--;
        current = current->next;
    }
    node_delete_all(&gol_state->dying_cells);

    current = gol_state->becoming_alive_cells;
    while (current) {
        node_insert_head(&gol_state->alive_cells, current->data);
        gol_state->population--;
        current = current->next;
    }
    node_delete_all(&gol_state->becoming_alive_cells);
}

typedef struct {
    SDL_Window *window;
    int window_width, window_height;
    SDL_Renderer *renderer;
    bool running, there_is_something_to_draw;
    float current_zoom;
    Point view_position;
    GolState *gol_state;
} Gui;

typedef struct {
    Point position;
    float side;
} Cell;

#define CELL_WIDTH_BASE 15
#define ZOOM_MAX 2.f
#define ZOOM_MIN .5f
#define FPS 60
#define ZOOM_FACTOR .01f
#define MOVEMENT_FACTOR 5

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
    new_gui->current_zoom = 1.f;
    new_gui->view_position.x = 0;
    new_gui->view_position.y = 0;

    new_gui->gol_state = golstate_alloc();
    new_gui->there_is_something_to_draw = true;

    SDL_RenderPresent(new_gui->renderer);
    return new_gui;
}

void gui_destroy(Gui *gui_ptr) {
    golstate_destroy(gui_ptr->gol_state);
    SDL_DestroyWindow(gui_ptr->window);
    SDL_DestroyRenderer(gui_ptr->renderer);
    SDL_Quit();
    free(gui_ptr);
}

int gui_point_to_virtual_grid_index(Gui *gui_ptr, Point gui_point) {
    gui_point.x -= gui_ptr->view_position.x;
    gui_point.x -= fmod(gui_point.x, CELL_WIDTH_BASE * gui_ptr->current_zoom);
    gui_point.y -= gui_ptr->view_position.y;
    gui_point.y -= fmod(gui_point.y, CELL_WIDTH_BASE * gui_ptr->current_zoom);

    int grid_index = (gui_point.y / CELL_WIDTH_BASE) * GRID_SIDE;
    grid_index += gui_point.x / (CELL_WIDTH_BASE * gui_ptr->current_zoom);
    return grid_index;
}

void gui_process_keyboard_events(Gui *gui_ptr, SDL_Event *e) {
    switch (e->key.keysym.sym) {
    case SDLK_ESCAPE:
    case SDLK_q:
        gui_ptr->running = false;
        break;
    case SDLK_PLUS:
        if (gui_ptr->current_zoom + ZOOM_FACTOR <= ZOOM_MAX)
            gui_ptr->current_zoom += ZOOM_FACTOR;
        break;
    case SDLK_MINUS:
        if (gui_ptr->current_zoom - ZOOM_FACTOR >= ZOOM_MIN)
            gui_ptr->current_zoom -= ZOOM_FACTOR;
        break;
    case SDLK_UP:
        gui_ptr->view_position.y += MOVEMENT_FACTOR;
        break;
    case SDLK_DOWN:
        gui_ptr->view_position.y -= MOVEMENT_FACTOR;
        break;
    case SDLK_RIGHT:
        gui_ptr->view_position.x -= MOVEMENT_FACTOR;
        break;
    case SDLK_LEFT:
        gui_ptr->view_position.x += MOVEMENT_FACTOR;
        break;
    default:
        break;
    }
}

void gui_process_mouse_event(Gui *gui_ptr, SDL_Event *e) {
    Point mouse_position = {0};
    switch (e->button.button) {
    case SDL_BUTTON_LEFT:
        mouse_position.x = e->button.x;
        mouse_position.y = e->button.y;
        int mouse_in_virtual_grid =
            gui_point_to_virtual_grid_index(gui_ptr, mouse_position);
        printf("info: mouse click (%f, %f) transformed to %d\n",
               mouse_position.x, mouse_position.y, mouse_in_virtual_grid);
        golstate_arbitrary_give_birth_cell(gui_ptr->gol_state,
                                           mouse_in_virtual_grid);
        gui_ptr->there_is_something_to_draw = true;
        break;
    default:
        break;
    }
}

void gui_process_events(Gui *gui_ptr) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
        case SDL_QUIT:
            gui_ptr->running = false;
            break;
        case SDL_KEYDOWN:
            gui_ptr->there_is_something_to_draw = true;
            gui_process_keyboard_events(gui_ptr, &e);
            break;
        case SDL_MOUSEBUTTONDOWN:
            gui_process_mouse_event(gui_ptr, &e);
            break;
        default:
            break;
        }
    }
}

void gui_draw_grid(Gui *gui_ptr) {
    float final_cell_width = CELL_WIDTH_BASE * gui_ptr->current_zoom;
    SDL_SetRenderDrawColor(gui_ptr->renderer, 20, 20, 20, 255);

    float start_x = fmax(0, gui_ptr->view_position.x);
    float start_y = fmax(0, gui_ptr->view_position.y);

    float end_x = fmin(gui_ptr->window_width,
                       GRID_SIDE * (CELL_WIDTH_BASE * gui_ptr->current_zoom) +
                           gui_ptr->view_position.x);
    float end_y = fmin(gui_ptr->window_height,
                       GRID_SIDE * (CELL_WIDTH_BASE * gui_ptr->current_zoom) +
                           gui_ptr->view_position.y);

    for (float x = start_x; x <= end_x; x += final_cell_width) {
        SDL_RenderDrawLineF(gui_ptr->renderer, x, start_y, x, end_y);
    }

    for (float y = start_y; y <= end_y; y += final_cell_width) {
        SDL_RenderDrawLineF(gui_ptr->renderer, start_x, y, end_x, y);
    }
}

void gui_draw_cell(Gui *gui_ptr, Point position) {
    Cell cell;
    cell.side = (float)CELL_WIDTH_BASE * gui_ptr->current_zoom;
    cell.position.x = (position.x * cell.side) + gui_ptr->view_position.x;
    cell.position.y = (position.y * cell.side) + gui_ptr->view_position.y;

    if (cell.position.x > gui_ptr->view_position.x + gui_ptr->window_width)
        return;
    if (cell.position.y > gui_ptr->view_position.y + gui_ptr->window_height)
        return;

    SDL_SetRenderDrawColor(gui_ptr->renderer, 255, 255, 255, 255);
    for (float i = cell.position.x; i < cell.position.x + cell.side;
         i += .01f) {
        SDL_RenderDrawLineF(gui_ptr->renderer, i, cell.position.y, i,
                            cell.position.y + cell.side);
    }
}

void gui_render(Gui *gui_ptr) {
    SDL_SetRenderDrawColor(gui_ptr->renderer, 0, 0, 0, 255);
    SDL_RenderClear(gui_ptr->renderer);
    gui_draw_grid(gui_ptr);

    Node *current = gui_ptr->gol_state->alive_cells;
    while (current) {
        Point gui_point = grid_index_to_point(current->data);
        gui_draw_cell(gui_ptr, gui_point);
        current = current->next;
    }

    SDL_RenderPresent(gui_ptr->renderer);
    gui_ptr->there_is_something_to_draw = false;
}

void gui_run(Gui *gui_ptr) {

    Point p = {150, 150};
    gui_ptr->gol_state->grid[point_to_grid_index(p)] = true;

    uint32_t current_time = SDL_GetTicks();
    uint32_t last_frame_time = current_time;
    while (gui_ptr->running) {

        current_time = SDL_GetTicks();
        uint32_t elapsed_time = current_time - last_frame_time;
        if (elapsed_time <= FPS)
            continue;

        gui_process_events(gui_ptr);
        // Gui_update(gui_ptr);

        if (gui_ptr->there_is_something_to_draw)
            gui_render(gui_ptr);

        // TODO: find better optimization method
        SDL_Delay(10);
    }
}

int main(void) {

    Gui *gui_ptr = gui_alloc();
    gui_run(gui_ptr);
    gui_destroy(gui_ptr);

    return 0;
}
