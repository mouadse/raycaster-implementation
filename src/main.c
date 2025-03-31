#include "../include/cub3d.h" // Main project header (assumed to include necessary types like t_params, t_point, t_fpoint, etc.)
#include <X11/X.h>            // For Event Masks
#include <X11/keysym.h>       // For XK_ Key Symbols
#include <limits.h>           // For INT_MAX (if not in cub3d.h)
#include <math.h>             // For M_PI, cos, sin, fmod, sqrt, pow, tan
#include <stdbool.h>          // For bool, true, false (if not in cub3d.h)
#include <stdio.h>            // For fprintf, perror
#include <stdlib.h>           // For exit, malloc, free
#include <string.h>           // For memset (or ft_memset)
#include <sys/time.h>         // For gettimeofday
#include <unistd.h>           // For usleep

// -------- Constants --------
#define MAP_SCALE 10
#define NUM_RAYS WINDOW_WIDTH
#define PLAYER_FOV (M_PI / 3.0) // 60 degrees
#define FRAME_RATE_CAP 60
#define MAX_VISIBLE_DISTANCE (15.0 * TILE_SIZE)
#define MINIMAP_RAY_STEP 8

// -------- Colors (Example) --------
#define C_BLACK 0x000000
#define C_WHITE 0xFFFFFF
#define C_RED 0xFF0000
#define C_GREEN 0x00FF00
#define C_BLUE 0x0000FF
#define C_YELLOW 0xFFFF00
#define C_GRAY 0x808080
#define C_DARK_GRAY 0x404040
#define C_CEILING 0x303060
#define C_FLOOR 0x604040

typedef struct s_ray_hit {
  double distance;
  t_fpoint hit_point;
  bool is_vertical;
  int map_x;
  int map_y;
  double ray_angle;
} t_ray_hit;

// --- Forward Declarations ---
void init_params(t_params *params);
int game_loop(t_params *params);
int key_press_hook(int keycode, t_params *params);
int close_window_hook(t_params *params);
void draw_map(t_params *params);
void draw_player(t_params *params);
void cast_rays(t_params *params, t_ray_hit *ray_hits);
void draw_rays_minimap(t_params *params, t_ray_hit *ray_hits);
double normalize_angle(double angle);
void clear_image_direct(t_params *params, int color);
int is_wall_at(t_params *params, double x, double y);
void cleanup(t_params *params);
void render_3d_view(t_params *params, t_ray_hit *ray_hits);
void draw_vertical_slice_direct(t_params *params, int x, int y_start, int y_end,
                                int color, double distance);
long get_time_ms(void);
void frame_rate_control(long *last_time, int target_fps);
int apply_shading(int color, double distance);

// Assumed external/libft functions (ensure these are available)
void *ft_memset(void *b, int c, size_t len);
char *ft_strdup(const char *s1);
size_t ft_strlen(const char *s);
void draw_line_img(t_params *params, t_point p1, t_point p2, int color);
t_fpoint find_horizontal_wall_intersection(t_params *params, double ray_angle);
t_fpoint find_vertical_wall_intersection(t_params *params, double ray_angle);
// Note: put_pixel is replaced by put_pixel_direct

// --- Optimized Drawing & Helpers ---

static inline void put_pixel_direct(t_img *img, int x, int y, int color) {
  char *dst;

  if (x >= 0 && x < img->width && y >= 0 && y < img->height) {
    dst = img->addr + (y * img->line_length + x * (img->bpp));
    *(unsigned int *)dst = color;
  }
}

void clear_image_direct(t_params *params, int color) {
  int y;
  char *line_start;
  int pixel_bytes = params->window_img.bpp;

  if (pixel_bytes == 4) {
    line_start = params->window_img.addr;
    for (y = 0; y < params->window_img.height; y++) {
      for (int x = 0; x < params->window_img.width; ++x) {
        *(unsigned int *)(line_start + x * pixel_bytes) = color;
      }
      line_start += params->window_img.line_length;
    }
  } else // Fallback for other BPP
  {
    for (y = 0; y < params->window_img.height; y++) {
      for (int x = 0; x < params->window_img.width; x++) {
        put_pixel_direct(&params->window_img, x, y, color);
      }
    }
  }
}

double normalize_angle(double angle) {
  angle = fmod(angle, 2.0 * M_PI);
  if (angle < 0)
    angle += (2.0 * M_PI);
  return angle;
}

int is_wall_at(t_params *params, double x, double y) {
  int map_x, map_y;

  if (x < 0 || y < 0 || x >= params->map.cols * TILE_SIZE ||
      y >= params->map.rows * TILE_SIZE)
    return 1;

  map_x = (int)(x / TILE_SIZE);
  map_y = (int)(y / TILE_SIZE);

  if (map_y < 0 || map_y >= params->map.rows || map_x < 0 ||
      map_x >= params->map.cols)
    return 1;

  return (params->map.map_data[map_y][map_x] == WALL);
}

long get_time_ms(void) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

void frame_rate_control(long *last_time, int target_fps) {
  long current_time, elapsed, delay;
  if (target_fps <= 0)
    return;
  current_time = get_time_ms();
  elapsed = current_time - *last_time;
  delay = (1000 / target_fps) - elapsed;
  if (delay > 0)
    usleep(delay * 1000);
  *last_time = get_time_ms();
}

int apply_shading(int color, double distance) {
  double brightness;
  int r, g, b;

  if (distance <= 0)
    return color;

  brightness = 1.0 - (distance / MAX_VISIBLE_DISTANCE);
  if (brightness < 0.0)
    brightness = 0.0;
  if (brightness > 1.0)
    brightness = 1.0;

  r = (color >> 16) & 0xFF;
  g = (color >> 8) & 0xFF;
  b = color & 0xFF;

  r = (int)(r * brightness);
  g = (int)(g * brightness);
  b = (int)(b * brightness);

  r = (r > 255) ? 255 : (r < 0 ? 0 : r);
  g = (g > 255) ? 255 : (g < 0 ? 0 : g);
  b = (b > 255) ? 255 : (b < 0 ? 0 : b);

  return (r << 16) | (g << 8) | b;
}

// --- Drawing Functions ---

void draw_map(t_params *params) {
  int x, y, tile_x, tile_y, color;
  int draw_x_base, draw_y_base;
  int map_pixel_width = params->map.cols * MAP_SCALE;
  int map_pixel_height = params->map.rows * MAP_SCALE;
  int max_draw_x = (map_pixel_width < params->window_img.width)
                       ? map_pixel_width
                       : params->window_img.width;
  int max_draw_y = (map_pixel_height < params->window_img.height)
                       ? map_pixel_height
                       : params->window_img.height;

  for (y = 0; y < params->map.rows; y++) {
    draw_y_base = y * MAP_SCALE;
    if (draw_y_base >= max_draw_y)
      break;

    for (x = 0; x < params->map.cols; x++) {
      draw_x_base = x * MAP_SCALE;
      if (draw_x_base >= max_draw_x)
        break;

      color = (params->map.map_data[y][x] == WALL) ? C_GRAY : C_DARK_GRAY;

      for (tile_y = 0; tile_y < MAP_SCALE - 1; tile_y++) {
        int py = draw_y_base + tile_y;
        if (py >= max_draw_y)
          break;

        for (tile_x = 0; tile_x < MAP_SCALE - 1; tile_x++) {
          int px = draw_x_base + tile_x;
          if (px >= max_draw_x)
            break;
          put_pixel_direct(&params->window_img, px, py, color);
        }
      }
    }
  }
}

void draw_player(t_params *params) {
  int player_marker_size = 4;
  int player_screen_x = (int)(params->player.x / TILE_SIZE * MAP_SCALE);
  int player_screen_y = (int)(params->player.y / TILE_SIZE * MAP_SCALE);
  int i, j, px, py;

  for (i = -player_marker_size / 2; i <= player_marker_size / 2; i++) {
    for (j = -player_marker_size / 2; j <= player_marker_size / 2; j++) {
      px = player_screen_x + i;
      py = player_screen_y + j;
      put_pixel_direct(&params->window_img, px, py, C_RED);
    }
  }

  t_point p1 = {player_screen_x, player_screen_y};
  t_point p2 = {
      player_screen_x + (int)(cos(params->player.direction) * MAP_SCALE * 1.5),
      player_screen_y + (int)(sin(params->player.direction) * MAP_SCALE * 1.5)};
  draw_line_img(params, p1, p2, C_RED);
}

void cast_rays(t_params *params, t_ray_hit *ray_hits) {
  double ray_angle, angle_step;
  int i;
  t_fpoint h_intersect, v_intersect;
  double h_dist_sq, v_dist_sq;

  angle_step = PLAYER_FOV / (double)NUM_RAYS;
  ray_angle = params->player.direction - (PLAYER_FOV / 2.0);

  for (i = 0; i < NUM_RAYS; i++) {
    ray_angle = normalize_angle(ray_angle);
    h_intersect = find_horizontal_wall_intersection(params, ray_angle);
    v_intersect = find_vertical_wall_intersection(params, ray_angle);

    h_dist_sq = (h_intersect.x != INT_MAX)
                    ? pow(h_intersect.x - params->player.x, 2) +
                          pow(h_intersect.y - params->player.y, 2)
                    : __DBL_MAX__;
    v_dist_sq = (v_intersect.x != INT_MAX)
                    ? pow(v_intersect.x - params->player.x, 2) +
                          pow(v_intersect.y - params->player.y, 2)
                    : __DBL_MAX__;

    if (h_dist_sq < v_dist_sq) {
      ray_hits[i].distance = sqrt(h_dist_sq);
      ray_hits[i].hit_point = h_intersect;
      ray_hits[i].is_vertical = false;
    } else {
      ray_hits[i].distance = sqrt(v_dist_sq);
      ray_hits[i].hit_point = v_intersect;
      ray_hits[i].is_vertical = true;
    }

    ray_hits[i].distance *=
        cos(ray_angle - params->player.direction); // Fisheye correction
    ray_hits[i].ray_angle = ray_angle;

    if (ray_hits[i].distance < __DBL_MAX__) {
      double check_x = ray_hits[i].hit_point.x;
      double check_y = ray_hits[i].hit_point.y;
      if (ray_hits[i].is_vertical) {
        check_x += (cos(ray_angle) > 0 ? -0.01 : 0.01);
      } else {
        check_y += (sin(ray_angle) > 0 ? -0.01 : 0.01);
      }
      ray_hits[i].map_x = (int)(check_x / TILE_SIZE);
      ray_hits[i].map_y = (int)(check_y / TILE_SIZE);
    } else {
      ray_hits[i].map_x = -1;
      ray_hits[i].map_y = -1;
    }
    ray_angle += angle_step;
  }
}

void draw_rays_minimap(t_params *params, t_ray_hit *ray_hits) {
  int i;
  t_point p1, p2;

  p1.x = (int)(params->player.x / TILE_SIZE * MAP_SCALE);
  p1.y = (int)(params->player.y / TILE_SIZE * MAP_SCALE);

  for (i = 0; i < NUM_RAYS; i += MINIMAP_RAY_STEP) {
    if (ray_hits[i].distance < MAX_VISIBLE_DISTANCE &&
        ray_hits[i].distance > 0.01) {
      p2.x = (int)(ray_hits[i].hit_point.x / TILE_SIZE * MAP_SCALE);
      p2.y = (int)(ray_hits[i].hit_point.y / TILE_SIZE * MAP_SCALE);
      draw_line_img(params, p1, p2, C_YELLOW);
    }
  }
}

void draw_vertical_slice_direct(t_params *params, int x, int y_start, int y_end,
                                int base_color, double distance) {
  int y, shaded_color;
  char *pixel_addr;
  int pixel_bytes = params->window_img.bpp;

  if (x < 0 || x >= params->window_img.width)
    return;
  int clamped_y_start = (y_start < 0) ? 0 : y_start;
  int clamped_y_end = (y_end >= params->window_img.height)
                          ? params->window_img.height - 1
                          : y_end;

  if (clamped_y_start > clamped_y_end)
    return; // Nothing to draw

  shaded_color = apply_shading(base_color, distance);
  pixel_addr = params->window_img.addr +
               (clamped_y_start * params->window_img.line_length) +
               (x * pixel_bytes);

  for (y = clamped_y_start; y <= clamped_y_end; y++) {
    *(unsigned int *)pixel_addr = shaded_color;
    pixel_addr += params->window_img.line_length;
  }
}

void render_3d_view(t_params *params, t_ray_hit *ray_hits) {
  int i, draw_start, draw_end, wall_color;
  double slice_height, perp_distance;

  for (i = 0; i < NUM_RAYS; i++) {
    perp_distance = ray_hits[i].distance;

    if (perp_distance < MAX_VISIBLE_DISTANCE && perp_distance > 0.01) {
      slice_height = (TILE_SIZE / perp_distance) * params->dist_proj_plane;
      draw_start = (params->window_img.height / 2) - ((int)slice_height / 2);
      draw_end = draw_start + (int)slice_height;

      wall_color =
          ray_hits[i].is_vertical ? C_GREEN : C_BLUE; // Example coloring

      draw_vertical_slice_direct(params, i, 0, draw_start - 1, C_CEILING,
                                 MAX_VISIBLE_DISTANCE);
      draw_vertical_slice_direct(params, i, draw_start, draw_end, wall_color,
                                 perp_distance);
      draw_vertical_slice_direct(params, i, draw_end + 1,
                                 params->window_img.height - 1, C_FLOOR,
                                 MAX_VISIBLE_DISTANCE);
    } else {
      draw_vertical_slice_direct(params, i, 0,
                                 params->window_img.height / 2 - 1, C_CEILING,
                                 MAX_VISIBLE_DISTANCE);
      draw_vertical_slice_direct(params, i, params->window_img.height / 2,
                                 params->window_img.height - 1, C_FLOOR,
                                 MAX_VISIBLE_DISTANCE);
    }
  }
}

// --- Game Logic and Hooks ---

int game_loop(t_params *params) {
  static t_ray_hit ray_hits[NUM_RAYS];
  static long last_frame_time = 0;

  clear_image_direct(params, C_BLACK);
  cast_rays(params, ray_hits);
  render_3d_view(params, ray_hits);

#ifdef DRAW_MINIMAP // Compile with -D DRAW_MINIMAP to enable
  draw_map(params);
  draw_rays_minimap(params, ray_hits);
  draw_player(params);
#endif

  mlx_put_image_to_window(params->mlx, params->win, params->window_img.img, 0,
                          0);
  frame_rate_control(&last_frame_time, FRAME_RATE_CAP);

  return 0;
}

int key_press_hook(int keycode, t_params *params) {
  double move_step = MOVE_SPEED; // Assumes MOVE_SPEED is defined appropriately
  double rot_step =
      ROTATE_SPEED; // Assumes ROTATE_SPEED is defined appropriately
  double new_x = params->player.x;
  double new_y = params->player.y;
  double dir_x = cos(params->player.direction);
  double dir_y = sin(params->player.direction);
  double side_x = -dir_y;
  double side_y = dir_x;
  double collision_dist = TILE_SIZE * 0.25;
  double move_x_comp = 0.0;
  double move_y_comp = 0.0;

  if (keycode == XK_Escape) {
    close_window_hook(params);
    return 0;
  }

  if (keycode == XK_Left) {
    params->player.direction -= rot_step;
  } else if (keycode == XK_Right) {
    params->player.direction += rot_step;
  }

  params->player.direction = normalize_angle(params->player.direction);
  dir_x = cos(params->player.direction); // Update after rotation
  dir_y = sin(params->player.direction);
  side_x = -dir_y;
  side_y = dir_x;

  if (keycode == XK_w || keycode == XK_Up) {
    move_x_comp += dir_x * move_step;
    move_y_comp += dir_y * move_step;
  } else if (keycode == XK_s || keycode == XK_Down) {
    move_x_comp -= dir_x * move_step;
    move_y_comp -= dir_y * move_step;
  } else if (keycode == XK_a) {
    move_x_comp += side_x * move_step;
    move_y_comp += side_y * move_step;
  } else if (keycode == XK_d) {
    move_x_comp -= side_x * move_step;
    move_y_comp -= side_y * move_step;
  }

  if (move_x_comp != 0.0 || move_y_comp != 0.0) {
    double check_x = params->player.x + move_x_comp;
    double collision_check_x =
        check_x + (move_x_comp > 0 ? collision_dist : -collision_dist);
    if (!is_wall_at(params, collision_check_x, params->player.y)) {
      new_x = check_x;
    }

    double check_y = params->player.y + move_y_comp;
    double collision_check_y =
        check_y + (move_y_comp > 0 ? collision_dist : -collision_dist);
    if (!is_wall_at(
            params, new_x,
            collision_check_y)) { // Check using potentially updated new_x
      new_y = check_y;
    }

    params->player.x = new_x;
    params->player.y = new_y;
  }
  return 0;
}

int close_window_hook(t_params *params) {
  cleanup(params);
  exit(EXIT_SUCCESS);
}

void cleanup(t_params *params) {
  int i = 0;

  if (params->map.map_data) {
    for (i = 0; i < params->map.rows; i++) {
      if (params->map.map_data[i]) {
        free(params->map.map_data[i]);
      }
    }
    free(params->map.map_data);
    params->map.map_data = NULL;
  }

  if (params->mlx) {
    if (params->window_img.img)
      mlx_destroy_image(params->mlx, params->window_img.img);
    if (params->win)
      mlx_destroy_window(params->mlx, params->win);
#ifndef __APPLE__
    mlx_destroy_display(params->mlx);
#endif
    free(params->mlx);
  }
}

void init_params(t_params *params) {
  const char *map_layout[] = {
      // Example map
      "1111111111111111111111111", "1000000001000000000000101",
      "1011010111011001011101101", "1001000000010001000100001",
      "10110111110110W0011101001", "1000000010000000000000001",
      "1001000010000111111000101", "1010001010000000001000101",
      "1111111111111111111111111"};
  int rows = sizeof(map_layout) / sizeof(map_layout[0]);
  int cols = 0;
  int x, y;
  bool player_found = false;

  ft_memset(params, 0, sizeof(t_params)); // Use ft_memset if available

  if (rows > 0)
    cols = ft_strlen(map_layout[0]); // Use ft_strlen if available

  params->map.rows = rows;
  params->map.cols = cols;
  if (rows < 3 || cols < 3) {
    fprintf(stderr, "Error: Map is too small.\n");
    exit(EXIT_FAILURE);
  }
  params->map.map_data = malloc(rows * sizeof(char *));
  if (!params->map.map_data) {
    perror("Error allocating map rows");
    exit(EXIT_FAILURE);
  }

  for (y = 0; y < rows; y++) {
    if (ft_strlen(map_layout[y]) != cols) { // Use ft_strlen
      fprintf(stderr, "Error: Map row %d has inconsistent length.\n", y);
      while (--y >= 0)
        free(params->map.map_data[y]);
      free(params->map.map_data);
      exit(EXIT_FAILURE);
    }
    params->map.map_data[y] = ft_strdup(map_layout[y]); // Use ft_strdup
    if (!params->map.map_data[y]) {
      perror("Error allocating map row");
      while (--y >= 0)
        free(params->map.map_data[y]);
      free(params->map.map_data);
      exit(EXIT_FAILURE);
    }

    for (x = 0; x < cols; x++) {
      char cell = params->map.map_data[y][x];
      if (strchr("01NSEW ", cell) == NULL) { // Allow space?
        fprintf(stderr, "Error: Invalid map character '%c' at (%d, %d).\n",
                cell, x, y);
        cleanup(params);
        exit(EXIT_FAILURE);
      }
      if (!player_found && strchr("NSEW", cell)) {
        params->player.x = (x + 0.5) * TILE_SIZE;
        params->player.y = (y + 0.5) * TILE_SIZE;
        if (cell == PLAYER_NORTH)
          params->player.direction = 3.0 * M_PI / 2.0;
        else if (cell == PLAYER_SOUTH)
          params->player.direction = M_PI / 2.0;
        else if (cell == PLAYER_EAST)
          params->player.direction = 0.0;
        else if (cell == PLAYER_WEST)
          params->player.direction = M_PI;
        params->map.map_data[y][x] = EMPTY;
        player_found = true;
      } else if (player_found && strchr("NSEW", cell)) {
        fprintf(stderr, "Error: Multiple player start positions.\n");
        cleanup(params);
        exit(EXIT_FAILURE);
      }
    }
  }

  if (!player_found) {
    fprintf(stderr, "Error: No player start position found.\n");
    cleanup(params);
    exit(EXIT_FAILURE);
  }

  for (x = 0; x < cols; x++)
    if (params->map.map_data[0][x] != WALL ||
        params->map.map_data[rows - 1][x] != WALL)
      goto map_error;
  for (y = 0; y < rows; y++)
    if (params->map.map_data[y][0] != WALL ||
        params->map.map_data[y][cols - 1] != WALL)
      goto map_error;
  goto map_ok;
map_error:
  fprintf(stderr, "Error: Map must be enclosed by walls ('1').\n");
  cleanup(params);
  exit(EXIT_FAILURE);
map_ok:;

  params->player.fov = PLAYER_FOV;
  params->dist_proj_plane = (WINDOW_WIDTH / 2.0) / tan(PLAYER_FOV / 2.0);

  params->mlx = mlx_init();
  if (!params->mlx) {
    perror("mlx_init failed");
    cleanup(params);
    exit(EXIT_FAILURE);
  }
  params->win = mlx_new_window(params->mlx, WINDOW_WIDTH, WINDOW_HEIGHT,
                               "Cub3D - Optimized");
  if (!params->win) {
    perror("mlx_new_window failed");
    cleanup(params);
    exit(EXIT_FAILURE);
  }

  params->window_img.width = WINDOW_WIDTH;
  params->window_img.height = WINDOW_HEIGHT;
  params->window_img.img = mlx_new_image(params->mlx, params->window_img.width,
                                         params->window_img.height);
  if (!params->window_img.img) {
    perror("mlx_new_image failed");
    cleanup(params);
    exit(EXIT_FAILURE);
  }
  params->window_img.addr = mlx_get_data_addr(
      params->window_img.img, &params->window_img.bits_per_pixel,
      &params->window_img.line_length, &params->window_img.endian);
  if (!params->window_img.addr) {
    perror("mlx_get_data_addr failed");
    cleanup(params);
    exit(EXIT_FAILURE);
  }
  params->window_img.bpp = params->window_img.bits_per_pixel / 8;
  if (params->window_img.bpp != 4) {
    fprintf(stderr, "Warning: Code optimized for 32bpp. Current bpp: %d\n",
            params->window_img.bpp * 8);
  }
}

int main(void) {
  t_params params;

  init_params(&params);

  mlx_loop_hook(params.mlx, game_loop, &params);
  mlx_hook(params.win, KeyPress, KeyPressMask, key_press_hook, &params);
  mlx_hook(params.win, DestroyNotify, StructureNotifyMask, close_window_hook,
           &params);

  mlx_loop(params.mlx);

  // Cleanup is handled by close_window_hook or exit in init_params on error
  return (0);
}
