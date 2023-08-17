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
