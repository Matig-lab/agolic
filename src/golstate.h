#ifndef _GOLSTATE_H_
#define _GOLSTATE_H_

#include "node.h"
#include <math.h>

#include <stdbool.h>

#define GRID_WIDTH 2000
#define GRID_SIZE GRID_WIDTH *GRID_WIDTH

typedef struct {
    bool grid[GRID_SIZE];
    bool analyzed_grid_cells[GRID_SIZE];
    Node *alive_cells;
    Node *dying_cells;
    Node *becoming_alive_cells;
    Node *recycled_cells;
    int population, generation;
    bool is_generation_analyzed;
} GolState;

#define MIN_NEIGHBORS_TO_SURVIVE 2
#define MAX_NEIGHBORS_TO_STAY_ALIVE 3
#define NEIGHBORS_TO_REPRODUCE 3

GolState *golstate_alloc();
void golstate_destroy(GolState **gol_state);
void golstate_restart(GolState *gol_state);
void golstate_arbitrary_give_birth_cell(GolState *gol_state, int grid_index);
void golstate_arbitrary_kill_cell(GolState *gol_state, int grid_index);
void golstate_analyze_generation(GolState *gol_state);
void golstate_next_generation(GolState *gol_state);

#endif // _GOLSTATE_H_
