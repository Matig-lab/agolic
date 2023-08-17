typedef struct {
    float x, y;
} Point;

int point2d_to_grid1d(Point point2d, int width_1d_grid, int size_1d_grid);
Point grid1d_to_point2d(int point1d, int width_1d_grid, int size_1d_grid);
