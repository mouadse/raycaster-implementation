#include "../include/cub3d.h"
void draw_square(t_params *params, int x, int y, int size, int color) {
  t_point p1, p2;

  /* Top line */
  p1.x = x;
  p1.y = y;
  p2.x = x + size;
  p2.y = y;
  draw_line(params, p1, p2, color);

  /* Right line */
  p1.x = x + size;
  p1.y = y;
  p2.x = x + size;
  p2.y = y + size;
  draw_line(params, p1, p2, color);

  /* Bottom line */
  p1.x = x + size;
  p1.y = y + size;
  p2.x = x;
  p2.y = y + size;
  draw_line(params, p1, p2, color);

  /* Left line */
  p1.x = x;
  p1.y = y + size;
  p2.x = x;
  p2.y = y;
  draw_line(params, p1, p2, color);
}

/* Function to draw a grid of squares */
void draw_grid(t_params *params, int grid_size, int square_size) {
  int row, col;
  int color;

  for (row = 0; row < grid_size; row++) {
    for (col = 0; col < grid_size; col++) {
      /* Alternate colors for a checkerboard pattern */
      color = ((row + col) % 2 == 0) ? 0x00FFFFFF : 0x00FF0000;
      draw_square(params, col * square_size, row * square_size, square_size,
                  color);
    }
  }
}

/* Handle key events */
int handle_key(int keycode, t_params *params) {
  /* Exit on ESC key (keycode 65307 on Linux) */
  if (keycode == 65307) {
    mlx_destroy_window(params->mlx, params->win);
    exit(0);
  }
  return (0);
}

/* Handle window close event */
int handle_close(t_params *params) {
  mlx_destroy_window(params->mlx, params->win);
  exit(0);
  return (0);
}

/* Main function */
int main(void) {
  t_params params;
  int window_width = 800;
  int window_height = 800;
  int grid_size = 8;
  int square_size = window_width / grid_size;

  /* Initialize mlx and create a window */
  params.mlx = mlx_init();
  if (!params.mlx) {
    printf("Error initializing MLX\n");
    return (1);
  }

  params.win = mlx_new_window(params.mlx, window_width, window_height,
                              "Grid of Squares");
  if (!params.win) {
    printf("Error creating window\n");
    return (1);
  }

  // params.width = window_width;
  // params.height = window_height;

  /* Draw the grid of squares */
  draw_grid(&params, grid_size, square_size);

  /* Set up event hooks */
  mlx_key_hook(params.win, handle_key, &params);
  mlx_hook(params.win, 17, 0, handle_close, &params);

  /* Main loop */
  mlx_loop(params.mlx);

  return (0);
}
