#include "../src/golstate.h"
#include <criterion/criterion.h>
#include <criterion/logging.h>
#include <time.h>

static double golstate_get_performance(void (*run)(GolState *),
                                       GolState *gol_state) {
    double start, end;
    double time_elapsed = 0;
    start = (double)clock() / CLOCKS_PER_SEC;
    (*run)(gol_state);
    end = (double)clock() / CLOCKS_PER_SEC;
    time_elapsed = end - start;
    return time_elapsed;
}

Test(golstate, high_load) {
    GolState *gol_state = golstate_alloc();

    for (int i = 0; i < GRID_SIZE; i += 2) {
        golstate_arbitrary_give_birth_cell(gol_state, i);
    }

    double perf1, perf2;
    for (int i = 0; i < 5; i++) {
        perf1 =
            golstate_get_performance(golstate_analyze_generation, gol_state);
        perf2 = golstate_get_performance(golstate_next_generation, gol_state);
        cr_log_info("Time elapsed on iteration #%d: %fs (Generation "
                    "analysis: %fs; Proceed to next gen: %fs; Population: %d)",
                    i + 1, perf1 + perf2, perf1, perf2, gol_state->population);
    }

    golstate_destroy(&gol_state);
}
