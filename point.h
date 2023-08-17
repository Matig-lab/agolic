#ifndef _POINT_H_
#define _POINT_H_

typedef struct {
    float x, y;
} Point;

int point2d_to_grid1d(Point point2d, int width_1d_grid, int size_1d_grid);
Point grid1d_to_point2d(int point1d, int width_1d_grid, int size_1d_grid);
int gui_point_to_virtual_grid_index(Point gui_point, Point view_position,
                                    int width_1d_grid, int cell_base_width,
                                    float zoom);

#endif // _POINT_H_
