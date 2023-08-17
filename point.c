#include "point.h"
#include <math.h>

int point2d_to_grid1d(Point point2d, int width_1d_grid, int size_1d_grid) {
    int point1d = 0;
    point1d = width_1d_grid * point2d.y;
    point1d += point2d.x;
    if (point1d < 0 || point1d < size_1d_grid)
        return -1;
    return point1d;
}

Point grid1d_to_point2d(int point1d, int width_1d_grid, int size_1d_grid) {
    Point point2d = {-1, -1};
    if (point1d < 0 || point1d >= size_1d_grid)
        return point2d;
    point2d.y = (float)floorf((float)point1d / width_1d_grid);
    point2d.x = (float)floorf((point1d % width_1d_grid));
    return point2d;
}

int gui_point_to_virtual_grid_index(Point gui_point, Point view_position,
                                    int width_1d_grid, int cell_base_width,
                                    float zoom) {
    gui_point.x -= view_position.x;
    gui_point.y -= view_position.y;
    gui_point.x -= fmod(gui_point.x, cell_base_width * zoom);
    gui_point.y -= fmod(gui_point.y, cell_base_width * zoom);

    int grid_index_x = gui_point.x / (cell_base_width * zoom);
    int grid_index_y = (gui_point.y / (cell_base_width * zoom)) * width_1d_grid;

    if (grid_index_x < 0 || grid_index_x >= width_1d_grid)
        return -1;

    return grid_index_y + grid_index_x;
}
