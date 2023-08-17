#include "gui.h"

int main(void) {

    Gui *gui = gui_alloc();
    gui_run(gui);
    gui_destroy(gui);

    return 0;
}
