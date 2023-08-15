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
    p.y = (float)roundf((float)grid_index / GRID_SIDE);
    p.x = (float)roundf((grid_index % GRID_SIDE));
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

GolState *golstate_alloc() {
    GolState *gol_state = malloc(sizeof(*gol_state));
    gol_state->grid = calloc(false, sizeof(bool) * GRID_AREA);
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

void golstate_arbitrary_give_birth_cell(int grid_index) {}

void golstate_arbitrary_kill_cell(int grid_index) {}

void golstate_analize_state() {}

void golstate_next_generation() {}

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
        // TODO: add new cell to grid
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

    Point p1 = grid_index_to_point(0);
    Point p2 = grid_index_to_point(9);
    Point p3 = grid_index_to_point(99);
    Point p4 = grid_index_to_point(90);

    gui_draw_cell(gui_ptr, p1);
    gui_draw_cell(gui_ptr, p2);
    gui_draw_cell(gui_ptr, p3);
    gui_draw_cell(gui_ptr, p4);

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
