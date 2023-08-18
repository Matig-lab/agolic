#include "../src/golstate.h"
#include <criterion/assert.h>
#include <criterion/criterion.h>
#include <time.h>

static inline int random_betewen(int lower, int upper) {
    return (rand() % (upper - lower + 1)) + lower;
}

void init_seed() { srand(time(NULL)); }

TestSuite(golstate, .init = init_seed);

Test(golstate, golstate_alloc) {
    GolState *gol_state = golstate_alloc();
    cr_assert_not_null(gol_state, "golstate_alloc() returned NULL");
    golstate_destroy(&gol_state);
    cr_assert_null(gol_state, "golstate_destroy() returned not NULL");
}

Test(golstate, golstate_restart) {
    GolState *gol_state = golstate_alloc();
    for (int i = 0; i < GRID_SIZE; i += random_betewen(10, 20)) {
        golstate_arbitrary_give_birth_cell(gol_state, i);
    }
    golstate_analyze_generation(gol_state);

    golstate_restart(gol_state);
    cr_assert_null(gol_state->alive_cells,
                   "alive_cells should be NULL after restart");
    cr_assert_null(gol_state->becoming_alive_cells,
                   "becoming_alive_cells should be NULL after restart");
    cr_assert_null(gol_state->dying_cells,
                   "dying_cells should be NULL after restart");
    cr_assert_eq(gol_state->population, 0,
                 "population should be 0 after restart");
    cr_assert_eq(gol_state->generation, 0,
                 "generation shoul be 0 after restart");
    golstate_destroy(&gol_state);
}

Test(golstate, no_population) {
    GolState *gol_state = golstate_alloc();
    for (int i = 0; i < 5; i++) {
        golstate_analyze_generation(gol_state);
        golstate_next_generation(gol_state);
    }
    cr_assert_eq(gol_state->population, 0,
                 "Generation with 0 population must remain with no life");
    golstate_destroy(&gol_state);
}

Test(golstate, still_life) {
    GolState *gol_state = golstate_alloc();

    int expected_population = 0;

    // Add square
    golstate_arbitrary_give_birth_cell(gol_state, 0);
    golstate_arbitrary_give_birth_cell(gol_state, 1);
    golstate_arbitrary_give_birth_cell(gol_state, GRID_WIDTH);
    golstate_arbitrary_give_birth_cell(gol_state, GRID_WIDTH + 1);
    expected_population += 4;

    // Add hive
    golstate_arbitrary_give_birth_cell(gol_state, 5);
    golstate_arbitrary_give_birth_cell(gol_state, 6);
    golstate_arbitrary_give_birth_cell(gol_state, 5 + GRID_WIDTH * 2);
    golstate_arbitrary_give_birth_cell(gol_state, 6 + GRID_WIDTH * 2);
    golstate_arbitrary_give_birth_cell(gol_state, 4 + GRID_WIDTH);
    golstate_arbitrary_give_birth_cell(gol_state, 7 + GRID_WIDTH);
    expected_population += 6;

    for (int i = 0; i < 10; i++) {
        golstate_analyze_generation(gol_state);
        golstate_next_generation(gol_state);
    }
    cr_assert_eq(expected_population, gol_state->population);

    golstate_destroy(&gol_state);
}

Test(golstate, inner_coherence) {
    GolState *gol_state = golstate_alloc();

    for (int i = 0; i < GRID_SIZE; i += random_betewen(10, 20)) {
        golstate_arbitrary_give_birth_cell(gol_state, i);
    }

    for (int i = 0; i < 5; i++) {
        golstate_analyze_generation(gol_state);
        golstate_next_generation(gol_state);
        int len = node_len(gol_state->alive_cells);
        cr_assert_eq(
            len, gol_state->population,
            "Inner incoherence, alive_cells length %d and population %d", len,
            gol_state->population);
    }

    golstate_destroy(&gol_state);
}

Test(golstate, grid_limits) {
    GolState *gol_state = golstate_alloc();

    // x axis
    // northwest
    golstate_arbitrary_give_birth_cell(gol_state, 0);
    golstate_arbitrary_give_birth_cell(gol_state, 1);
    golstate_arbitrary_give_birth_cell(gol_state, 2);
    golstate_analyze_generation(gol_state);
    golstate_next_generation(gol_state);
    cr_assert_eq(gol_state->population, 2,
                 "X axis: northwest population should be 2 instead of %d",
                 gol_state->population);

    // northeast
    golstate_restart(gol_state);
    golstate_arbitrary_give_birth_cell(gol_state, GRID_WIDTH - 1);
    golstate_arbitrary_give_birth_cell(gol_state, GRID_WIDTH - 2);
    golstate_arbitrary_give_birth_cell(gol_state, GRID_WIDTH - 3);
    golstate_analyze_generation(gol_state);
    golstate_next_generation(gol_state);
    cr_assert_eq(gol_state->population, 2,
                 "X axis: northeast population should be 2 instead of %d",
                 gol_state->population);

    // southwest
    golstate_restart(gol_state);
    golstate_arbitrary_give_birth_cell(gol_state, GRID_SIZE - (GRID_WIDTH - 1));
    golstate_arbitrary_give_birth_cell(gol_state, GRID_SIZE - (GRID_WIDTH - 2));
    golstate_arbitrary_give_birth_cell(gol_state, GRID_SIZE - (GRID_WIDTH - 3));
    golstate_analyze_generation(gol_state);
    golstate_next_generation(gol_state);
    cr_assert_eq(gol_state->population, 2,
                 "X axis: southwest population should be 2 instead of %d",
                 gol_state->population);

    // southeast
    golstate_restart(gol_state);
    golstate_arbitrary_give_birth_cell(gol_state, GRID_SIZE - 1);
    golstate_arbitrary_give_birth_cell(gol_state, GRID_SIZE - 2);
    golstate_arbitrary_give_birth_cell(gol_state, GRID_SIZE - 3);
    golstate_analyze_generation(gol_state);
    golstate_next_generation(gol_state);
    cr_assert_eq(gol_state->population, 2,
                 "X axis: southeast population should be 2 instead of %d",
                 gol_state->population);

    // y axis
    // northwest
    golstate_restart(gol_state);
    golstate_arbitrary_give_birth_cell(gol_state, 0);
    golstate_arbitrary_give_birth_cell(gol_state, GRID_WIDTH);
    golstate_arbitrary_give_birth_cell(gol_state, GRID_WIDTH * 2);
    golstate_analyze_generation(gol_state);
    golstate_next_generation(gol_state);
    cr_assert_eq(gol_state->population, 2,
                 "Y axis: northwest population should be 2 instead of %d",
                 gol_state->population);

    // northeast
    golstate_restart(gol_state);
    golstate_arbitrary_give_birth_cell(gol_state, GRID_WIDTH - 1);
    golstate_arbitrary_give_birth_cell(gol_state,
                                       (GRID_WIDTH - 1) + GRID_WIDTH);
    golstate_arbitrary_give_birth_cell(gol_state,
                                       (GRID_WIDTH - 1) + (GRID_WIDTH * 2));
    golstate_analyze_generation(gol_state);
    golstate_next_generation(gol_state);
    cr_assert_eq(gol_state->population, 2,
                 "Y axis: northeast population should be 2 instead of %d",
                 gol_state->population);

    // southwest
    golstate_restart(gol_state);
    golstate_arbitrary_give_birth_cell(gol_state, GRID_WIDTH * GRID_WIDTH - 1);
    golstate_arbitrary_give_birth_cell(gol_state, GRID_WIDTH * GRID_WIDTH - 2);
    golstate_arbitrary_give_birth_cell(gol_state, GRID_WIDTH * GRID_WIDTH - 3);
    golstate_analyze_generation(gol_state);
    golstate_next_generation(gol_state);
    cr_assert_eq(gol_state->population, 2,
                 "Y axis: southwest population should be 2 instead of %d (%d)",
                 gol_state->population, GRID_WIDTH * GRID_WIDTH);

    // southeast
    golstate_restart(gol_state);
    golstate_arbitrary_give_birth_cell(gol_state, GRID_SIZE - 1);
    golstate_arbitrary_give_birth_cell(gol_state, (GRID_SIZE - 1) - GRID_WIDTH);
    golstate_arbitrary_give_birth_cell(gol_state,
                                       (GRID_SIZE - 1) - GRID_WIDTH * 2);
    golstate_analyze_generation(gol_state);
    golstate_next_generation(gol_state);
    cr_assert_eq(gol_state->population, 2,
                 "Y axis: southeast population should be 2 instead of %d",
                 gol_state->population);

    golstate_destroy(&gol_state);
}

Test(golstate, recycled_nodes) {
    GolState *gol_state = golstate_alloc();

    // Add square
    golstate_arbitrary_give_birth_cell(gol_state, 0);
    golstate_arbitrary_give_birth_cell(gol_state, 1);
    golstate_arbitrary_give_birth_cell(gol_state, GRID_WIDTH);
    golstate_arbitrary_give_birth_cell(gol_state, GRID_WIDTH + 1);

    // Add hive
    golstate_arbitrary_give_birth_cell(gol_state, 5);
    golstate_arbitrary_give_birth_cell(gol_state, 6);
    golstate_arbitrary_give_birth_cell(gol_state, 5 + GRID_WIDTH * 2);
    golstate_arbitrary_give_birth_cell(gol_state, 6 + GRID_WIDTH * 2);
    golstate_arbitrary_give_birth_cell(gol_state, 4 + GRID_WIDTH);
    golstate_arbitrary_give_birth_cell(gol_state, 7 + GRID_WIDTH);

    // Isolated cell
    golstate_arbitrary_give_birth_cell(gol_state, GRID_WIDTH - 1);

    golstate_analyze_generation(gol_state);
    cr_assert_not_null(gol_state->dying_cells);

    golstate_next_generation(gol_state);

    cr_assert_eq(gol_state->population, 10);
    cr_assert_not_null(gol_state->recycled_nodes);

    golstate_destroy(&gol_state);
}

Test(golstate, evolution) {
    GolState *gol_state = golstate_alloc();

    int position = GRID_SIZE / 2 + GRID_WIDTH / 2;

    golstate_arbitrary_give_birth_cell(gol_state, position);
    golstate_arbitrary_give_birth_cell(gol_state, position + 1);
    golstate_arbitrary_give_birth_cell(gol_state, position + 2);
    golstate_arbitrary_give_birth_cell(gol_state, position + 3);
    golstate_arbitrary_give_birth_cell(gol_state, position + 4);

    golstate_arbitrary_give_birth_cell(gol_state, position + GRID_WIDTH);
    golstate_arbitrary_give_birth_cell(gol_state, position + 4 + GRID_WIDTH);

    golstate_arbitrary_give_birth_cell(gol_state, position + (GRID_WIDTH * 2));
    golstate_arbitrary_give_birth_cell(gol_state,
                                       position + 1 + (GRID_WIDTH * 2));
    golstate_arbitrary_give_birth_cell(gol_state,
                                       position + 2 + (GRID_WIDTH * 2));
    golstate_arbitrary_give_birth_cell(gol_state,
                                       position + 3 + (GRID_WIDTH * 2));
    golstate_arbitrary_give_birth_cell(gol_state,
                                       position + 4 + (GRID_WIDTH * 2));

    for (int i = 0; i < 30; i++) {
        golstate_analyze_generation(gol_state);
        golstate_next_generation(gol_state);
    }
    cr_assert_eq(
        gol_state->population, 56,
        "End of evolution should have a population of 56 instead of %d",
        gol_state->population);

    golstate_destroy(&gol_state);
}
