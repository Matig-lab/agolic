#include "golstate.h"
#include "node.h"

#include <stdlib.h>

GolState *golstate_alloc() {
    GolState *gol_state = malloc(sizeof(*gol_state));
    for (int i = 0; i < GRID_SIZE; i++) {
        gol_state->grid[i] = false;
        gol_state->analyzed_grid_cells[i] = false;
    }
    gol_state->alive_cells = NULL;
    gol_state->dying_cells = NULL;
    gol_state->becoming_alive_cells = NULL;
    gol_state->recycled_cells = NULL;
    gol_state->population = 0;
    gol_state->generation = 0;
    gol_state->is_generation_analyzed = false;
    return gol_state;
}

void golstate_destroy(GolState **gol_state) {
    node_destroy_all(&(*gol_state)->alive_cells);
    node_destroy_all(&(*gol_state)->dying_cells);
    node_destroy_all(&(*gol_state)->becoming_alive_cells);
    node_destroy_all(&(*gol_state)->recycled_cells);
    free(*gol_state);
    *gol_state = NULL;
}

void golstate_restart(GolState *gol_state) {
    node_destroy_all(&gol_state->alive_cells);
    node_destroy_all(&gol_state->becoming_alive_cells);
    node_destroy_all(&gol_state->dying_cells);
    node_destroy_all(&gol_state->recycled_cells);
    gol_state->population = 0;
    gol_state->generation = 0;
    gol_state->is_generation_analyzed = false;
    for (int i = 0; i < GRID_SIZE; i++) {
        gol_state->grid[i] = false;
        gol_state->analyzed_grid_cells[i] = false;
    }
}

static void golstate_cleanup_analyzed_cells(GolState *gol_state) {
    for (int i = 0; i < GRID_SIZE; i++) {
        gol_state->analyzed_grid_cells[i] = false;
    }
}

static void golstate_cleanup_alive_cells(GolState *gol_state) {
    Node *new_alive_cells = NULL;
    Node *current = node_pop(&gol_state->alive_cells);
    while (current) {
        if (!gol_state->grid[current->data]) {
            node_insert_head_node(&gol_state->recycled_cells, current);
        } else {
            node_insert_head_node(&new_alive_cells, current);
        }
        current = node_pop(&gol_state->alive_cells);
    }
    gol_state->alive_cells = new_alive_cells;
}

static void golstate_cleanup(GolState *gol_state) {
    golstate_cleanup_analyzed_cells(gol_state);
    golstate_cleanup_alive_cells(gol_state);
}

void golstate_arbitrary_give_birth_cell(GolState *gol_state, int grid_index) {
    if (grid_index < 0 || grid_index >= GRID_SIZE ||
        gol_state->grid[grid_index]) {
        return;
    }

    if (gol_state->recycled_cells) {
        Node *new_cell = node_pop(&gol_state->recycled_cells);
        new_cell->data = grid_index;
        node_append_node(&gol_state->alive_cells, new_cell);
    } else {
        node_append(&gol_state->alive_cells, grid_index);
    }
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
static int golstate_get_neighborhood_start(int neighborhood_center,
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

#define END_IS_IN_CORRECT_INDEX(e, l)                                          \
    (e < GRID_SIZE && (e == 0 ? 0 : e / GRID_WIDTH) == l)
static int golstate_get_neighborhood_end(int neighborhood_center,
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

static void golstate_neighborhood_analysis(GolState *gol_state,
                                           int neighborhood_center,
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
    int neighborhood_start = golstate_get_neighborhood_start(
        neighborhood_center, neighborhood_center_line);
    int neighborhood_end = golstate_get_neighborhood_end(
        neighborhood_center, neighborhood_center_line);

    int neighborhood_end_of_first_line = neighborhood_center - (GRID_WIDTH - 1);
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
        if (gather_indexes && !gol_state->grid[i])
            node_append(list_of_indexes_dst, i);

        if (gol_state->grid[i])
            (*life_in_neighborhood)++;
    }
}

static bool golstate_cell_stays_alive(int life_in_neighborhood) {
    return life_in_neighborhood >= MIN_NEIGHBORS_TO_SURVIVE &&
           life_in_neighborhood <= MAX_NEIGHBORS_TO_STAY_ALIVE;
}

static bool golstate_dead_cell_becomes_alive(int life_in_neighborhood) {
    return life_in_neighborhood == NEIGHBORS_TO_REPRODUCE;
}

void golstate_analyze_generation(GolState *gol_state) {
    // Analyze current cell
    Node *current_cell = gol_state->alive_cells;
    while (current_cell) {

        int life_in_neighborhood = 0;
        Node *neighborhood = NULL;
        golstate_neighborhood_analysis(gol_state, current_cell->data,
                                       &neighborhood, &life_in_neighborhood,
                                       true);
        if (!golstate_cell_stays_alive(life_in_neighborhood)) {
            if (gol_state->recycled_cells) {
                Node *cell_node = node_pop(&gol_state->recycled_cells);
                cell_node->data = current_cell->data;
                node_append_node(&gol_state->dying_cells, cell_node);
            } else {
                node_append_uniq(&gol_state->dying_cells, current_cell->data);
            }
        }

        // Analyze each dead cell in neighborhood
        Node *current_neighborhood_cell = neighborhood;
        while (current_neighborhood_cell) {
            if (gol_state->grid[current_neighborhood_cell->data] ||
                gol_state
                    ->analyzed_grid_cells[current_neighborhood_cell->data]) {
                current_neighborhood_cell = current_neighborhood_cell->next;
                continue;
            }

            golstate_neighborhood_analysis(gol_state,
                                           current_neighborhood_cell->data,
                                           NULL, &life_in_neighborhood, false);

            if (golstate_dead_cell_becomes_alive(life_in_neighborhood)) {
                if (gol_state->recycled_cells) {
                    Node *cell_node = node_pop(&gol_state->recycled_cells);
                    cell_node->data = current_neighborhood_cell->data;
                    node_append_node(&gol_state->becoming_alive_cells,
                                     cell_node);
                } else {
                    node_append_uniq(&gol_state->becoming_alive_cells,
                                     current_neighborhood_cell->data);
                }
                gol_state
                    ->analyzed_grid_cells[current_neighborhood_cell->data] =
                    true;
            }

            current_neighborhood_cell = current_neighborhood_cell->next;
        }
        node_destroy_all(&neighborhood);
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
    node_concat(&gol_state->recycled_cells, &gol_state->dying_cells);

    current = gol_state->becoming_alive_cells;
    while (current) {
        gol_state->grid[current->data] = true;
        gol_state->population++;
        current = current->next;
    }
    node_concat(&gol_state->alive_cells, &gol_state->becoming_alive_cells);
    golstate_cleanup(gol_state);
    gol_state->generation++;
    gol_state->is_generation_analyzed = false;
}
