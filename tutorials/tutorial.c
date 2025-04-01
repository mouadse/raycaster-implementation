#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <mlx.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#define PI 3.14159265
#define TWO_PI 6.28318530

#define TILE_SIZE 64

#define MINIMAP_SCALE_FACTOR 0.25
#define MINIMAP_MARGIN 10
#define MINIMAP_OPACITY 0.8 // 0.0 to 1.0
#define MINIMAP_BORDER_COLOR 0xFFFFFF
#define MINIMAP_BACKGROUND_COLOR 0x222222
#define MINIMAP_WALL_COLOR 0x4A90E2   // Modern blue
#define MINIMAP_FLOOR_COLOR 0x333333  // Dark gray
#define MINIMAP_PLAYER_COLOR 0xFF5252 // Vibrant red
#define MINIMAP_RAY_COLOR 0xFFD700    // Gold

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 800

#define FOV_ANGLE (60 * (PI / 180))

#define NUM_RAYS WINDOW_WIDTH

#define DIST_PROJ_PLANE ((WINDOW_WIDTH / 2) / tan(FOV_ANGLE / 2))

#define FPS 30
#define FRAME_TIME_LENGTH (1000 / FPS)

#define MAP_NUM_COLS 25
#define MAP_NUM_ROWS 9

// Linux key codes
#define KEY_ESC 65307
#define KEY_W 119
#define KEY_A 97
#define KEY_S 115
#define KEY_D 100
#define KEY_UP 65362
#define KEY_DOWN 65364
#define KEY_LEFT 65361
#define KEY_RIGHT 65363

typedef struct s_point {
  int x;
  int y;
} t_point;

typedef struct s_fpoint {
  float x;
  float y;
} t_fpoint;

typedef struct s_img {
  void *img;
  char *addr;
  int bits_per_pixel;
  int bpp;
  int line_length;
  int endian;
  int width;
  int height;
} t_img;

typedef struct s_ray {
  float rayAngle;
  float wallHitX;
  float wallHitY;
  float distance;
  bool wasHitVertical;
} t_ray;

typedef struct s_wall {
  double wall_height;
  char *wall_texture;
  t_img *texture_img;
  int texture_x;
  int texture_y;
  int wall_y;
} t_wall;

typedef struct s_map {
  int cols;
  int rows;
  char **map_data;
} t_map;

typedef struct s_player {
  float x;
  float y;
  float width;
  float height;
  int turnDirection; // -1 for left, +1 for right
  int walkDirection; // -1 for back, +1 for front
  float rotationAngle;
  float walkSpeed;
  float turnSpeed;
  float direction; // Current direction (angle)
  float dx;        // Direction vector x
  float dy;        // Direction vector y
  float fov;       // Field of view
} t_player;

typedef struct s_params {
  void *mlx;
  void *win;
  t_img window_img;

  t_map map;

  t_player player;
  t_ray rays[NUM_RAYS];

  t_img north_texture;
  t_img south_texture;
  t_img west_texture;
  t_img east_texture;

  int floor_color;
  int ceiling_color;
  double dist_proj_plane; // Distance to projection plane for 3D rendering
  t_wall wall;
} t_params;

// Function prototypes
bool isRayFacingUp(float angle);
bool isRayFacingDown(float angle);
bool isRayFacingLeft(float angle);
bool isRayFacingRight(float angle);
void castAllRays(t_params *params);
void castRay(float rayAngle, int stripId, t_params *params);
void renderMapRays(t_params *params);
void normalizeAngle(float *angle);
float distanceBetweenPoints(float x1, float y1, float x2, float y2);
int put_pixel(t_params *params, int x, int y, int color);
void drawRect(t_params *params, int x, int y, int width, int height, int color);

const char *map_layout[] = {
    // Example map
    "1111111111111111111111111", "1000000001000000000000101",
    "1011010111011001011101101", "1001000000010001000100001",
    "10110111110110P0011101001", "1000000010000000000000001",
    "1001000010000111111000101", "1010001010000000001000101",
    "1111111111111111111111111"};

// Calculate minimap dimensions
int calculateMinimapWidth(t_params *params) {
  return params->map.cols * TILE_SIZE * MINIMAP_SCALE_FACTOR;
}

int calculateMinimapHeight(t_params *params) {
  return params->map.rows * TILE_SIZE * MINIMAP_SCALE_FACTOR;
}

// Draw a filled rectangle with opacity
void drawRectWithOpacity(t_params *params, int x, int y, int width, int height,
                         int color, float opacity) {
  int alpha, red, green, blue;
  int bg_red, bg_green, bg_blue;
  int final_red, final_green, final_blue;
  unsigned int final_color;

  // Extract RGBA components from color
  red = (color >> 16) & 0xFF;
  green = (color >> 8) & 0xFF;
  blue = color & 0xFF;

  for (int j = y; j < y + height; j++) {
    for (int i = x; i < x + width; i++) {
      if (i >= 0 && i < WINDOW_WIDTH && j >= 0 && j < WINDOW_HEIGHT) {
        // For true opacity, we would blend with existing pixel
        // But for simplicity, we'll just adjust the color intensity
        final_red = red * opacity;
        final_green = green * opacity;
        final_blue = blue * opacity;

        final_color = (final_red << 16) | (final_green << 8) | final_blue;
        put_pixel(params, i, j, final_color);
      }
    }
  }
}

// Draw a border around a rectangle
void drawRectBorder(t_params *params, int x, int y, int width, int height,
                    int color, int thickness) {
  // Top border
  drawRect(params, x, y, width, thickness, color);
  // Bottom border
  drawRect(params, x, y + height - thickness, width, thickness, color);
  // Left border
  drawRect(params, x, y, thickness, height, color);
  // Right border
  drawRect(params, x + width - thickness, y, thickness, height, color);
}

int put_pixel(t_params *params, int x, int y, int color) {
  int pixel_index;

  pixel_index = y * params->window_img.line_length + x * params->window_img.bpp;
  if (pixel_index >= 0 &&
      pixel_index < params->window_img.line_length * params->window_img.height)
    *(unsigned int *)(params->window_img.addr + pixel_index) = color;
  return (0);
}

void bresenham_algorithm(t_point p1, t_point p2, t_point *delta,
                         t_point *sign) {
  delta->x = abs(p2.x - p1.x);
  delta->y = -abs(p2.y - p1.y);
  if (p1.x < p2.x)
    sign->x = 1;
  else
    sign->x = -1;
  if (p1.y < p2.y)
    sign->y = 1;
  else
    sign->y = -1;
}

void draw_line(t_params *params, t_point p1, t_point p2, int color) {
  t_point d;
  t_point s;
  int err;
  int e2;

  bresenham_algorithm(p1, p2, &d, &s);
  err = d.x + d.y;
  while (1) {
    put_pixel(params, p1.x, p1.y, color);
    if (p1.x == p2.x && p1.y == p2.y)
      break;
    e2 = 2 * err;
    if (e2 >= d.y) {
      err += d.y;
      p1.x += s.x;
    }
    if (e2 <= d.x) {
      err += d.x;
      p1.y += s.y;
    }
  }
}

bool mapHasWallAt(float x, float y, t_params *params) {
  int mapGridIndexX;
  int mapGridIndexY;

  if (x < 0 || x >= MAP_NUM_COLS * TILE_SIZE || y < 0 ||
      y >= MAP_NUM_ROWS * TILE_SIZE) {
    return (true);
  }
  mapGridIndexX = floor(x / TILE_SIZE);
  mapGridIndexY = floor(y / TILE_SIZE);
  return (params->map.map_data[mapGridIndexY][mapGridIndexX] != '0');
}

bool isInsideMap(float x, float y) {
  return (x >= 0 && x <= MAP_NUM_COLS * TILE_SIZE && y >= 0 &&
          y <= MAP_NUM_ROWS * TILE_SIZE);
}

int getMapAt(int i, int j, t_params *params) {
  if (i >= 0 && i < MAP_NUM_ROWS && j >= 0 && j < MAP_NUM_COLS)
    return (params->map.map_data[i][j]);
  return ('1'); // Return wall for out of bounds
}

void drawRect(t_params *params, int x, int y, int width, int height,
              int color) {
  int i;
  int j;

  for (i = y; i < y + height; i++) {
    for (j = x; j < x + width; j++) {
      put_pixel(params, j, i, color);
    }
  }
}

void renderMapGrid(t_params *params) {
  int minimapWidth = calculateMinimapWidth(params);
  int minimapHeight = calculateMinimapHeight(params);
  int minimapX = MINIMAP_MARGIN;
  int minimapY = MINIMAP_MARGIN;

  // Draw background with opacity
  drawRectWithOpacity(params, minimapX, minimapY, minimapWidth, minimapHeight,
                      MINIMAP_BACKGROUND_COLOR, MINIMAP_OPACITY);

  // Draw map grid
  int tileColor;
  int tileX, tileY;
  int tileSize = TILE_SIZE * MINIMAP_SCALE_FACTOR;

  for (int i = 0; i < MAP_NUM_ROWS; i++) {
    for (int j = 0; j < MAP_NUM_COLS; j++) {
      tileX = minimapX + j * tileSize;
      tileY = minimapY + i * tileSize;

      // Choose tile color based on map content
      tileColor = getMapAt(i, j, params) != '0' ? MINIMAP_WALL_COLOR
                                                : MINIMAP_FLOOR_COLOR;

      // Draw tile with slight padding for grid effect
      drawRect(params, tileX + 1, tileY + 1, tileSize - 2, tileSize - 2,
               tileColor);
    }
  }

  // Draw border
  drawRectBorder(params, minimapX, minimapY, minimapWidth, minimapHeight,
                 MINIMAP_BORDER_COLOR, 2);
}

// Initialize MLX and window
int init_mlx(t_params *params) {
  params->mlx = mlx_init();
  if (!params->mlx)
    return (0);
  params->win =
      mlx_new_window(params->mlx, WINDOW_WIDTH, WINDOW_HEIGHT, "Raycaster");
  if (!params->win)
    return (0);
  return (1);
}

// Initialize window image
int init_image(t_params *params) {
  params->window_img.img =
      mlx_new_image(params->mlx, WINDOW_WIDTH, WINDOW_HEIGHT);
  if (!params->window_img.img)
    return (0);
  params->window_img.addr = mlx_get_data_addr(
      params->window_img.img, &params->window_img.bits_per_pixel,
      &params->window_img.line_length, &params->window_img.endian);
  params->window_img.bpp = params->window_img.bits_per_pixel / 8;
  params->window_img.width = WINDOW_WIDTH;
  params->window_img.height = WINDOW_HEIGHT;
  return (1);
}

// Initialize player
void init_player(t_params *params) {
  params->player.width = 1;
  params->player.height = 1;
  params->player.turnDirection = 0;
  params->player.walkDirection = 0;
  params->player.rotationAngle = PI / 2;
  params->player.walkSpeed = 100;
  params->player.turnSpeed =
      30.0 * (PI / 180); // Much faster rotation (8 degrees per frame)
  params->player.dx = cos(params->player.rotationAngle);
  params->player.dy = sin(params->player.rotationAngle);
  params->player.fov = FOV_ANGLE;
}

// Initialize map data
void init_map(t_params *params) {
  int i;

  params->map.rows = MAP_NUM_ROWS;
  params->map.cols = MAP_NUM_COLS;
  // Allocate memory for map data
  params->map.map_data = malloc(params->map.rows * sizeof(char *));
  for (i = 0; i < params->map.rows; i++) {
    params->map.map_data[i] = strdup(map_layout[i]);
  }
}

// Handle key press
int key_press(int keycode, t_params *params) {
  // Check for ESC key
  if (keycode == KEY_ESC) {
    mlx_destroy_window(params->mlx, params->win);
    exit(0);
  }
  // WASD and Arrow keys for movement
  if (keycode == KEY_W || keycode == KEY_UP)
    params->player.walkDirection = 1;
  if (keycode == KEY_S || keycode == KEY_DOWN)
    params->player.walkDirection = -1;
  if (keycode == KEY_A || keycode == KEY_LEFT)
    params->player.turnDirection = -1;
  if (keycode == KEY_D || keycode == KEY_RIGHT)
    params->player.turnDirection = 1;

  return (0);
}

// Handle key release
int key_release(int keycode, t_params *params) {
  // WASD and Arrow keys for movement
  if ((keycode == KEY_W || keycode == KEY_UP) &&
      params->player.walkDirection == 1)
    params->player.walkDirection = 0;
  if ((keycode == KEY_S || keycode == KEY_DOWN) &&
      params->player.walkDirection == -1)
    params->player.walkDirection = 0;
  if ((keycode == KEY_A || keycode == KEY_LEFT) &&
      params->player.turnDirection == -1)
    params->player.turnDirection = 0;
  if ((keycode == KEY_D || keycode == KEY_RIGHT) &&
      params->player.turnDirection == 1)
    params->player.turnDirection = 0;

  return (0);
}

// Clear the screen (fill with black)
void clear_screen(t_params *params) {
  int x, y;
  for (y = 0; y < WINDOW_HEIGHT; y++) {
    for (x = 0; x < WINDOW_WIDTH; x++) {
      put_pixel(params, x, y, 0x000000); // Black color
    }
  }
}

void movePlayer(t_params *params, float deltaTime) {
  // Update rotation based on turn direction
  params->player.rotationAngle +=
      params->player.turnDirection * params->player.turnSpeed * deltaTime;

  // Normalize angle
  normalizeAngle(&params->player.rotationAngle);

  // Update direction vectors
  params->player.dx = cos(params->player.rotationAngle);
  params->player.dy = sin(params->player.rotationAngle);

  // Calculate movement distance
  float moveStep =
      params->player.walkDirection * params->player.walkSpeed * deltaTime;

  // Calculate new position
  float newX = params->player.x + params->player.dx * moveStep;
  float newY = params->player.y + params->player.dy * moveStep;

  // Check for collisions before updating position
  if (!mapHasWallAt(newX, params->player.y, params))
    params->player.x = newX;
  if (!mapHasWallAt(params->player.x, newY, params))
    params->player.y = newY;
}

// Update game state
int update(t_params *params) {
  static struct timeval lastFrameTime;
  struct timeval currentTime;
  float deltaTime;

  // Get current time
  gettimeofday(&currentTime, NULL);

  // Calculate delta time in seconds
  if (lastFrameTime.tv_sec == 0 && lastFrameTime.tv_usec == 0) {
    // First frame, no delta time yet
    deltaTime = 0.016f; // Assume ~60fps for first frame
  } else {
    deltaTime = (currentTime.tv_sec - lastFrameTime.tv_sec) +
                (currentTime.tv_usec - lastFrameTime.tv_usec) / 1000000.0f;
  }

  // Store current time for next frame
  lastFrameTime = currentTime;

  // Limit delta time to avoid large jumps
  if (deltaTime > 0.05f)
    deltaTime = 0.05f;

  // Update player position
  movePlayer(params, deltaTime);

  return (0);
}

// Clean up resources
// void	cleanup(t_params *params)
// {
// 	int	i;

// 	// Free map data
// 	for (i = 0; i < params->map.rows; i++)
// 	{
// 		free(params->map.map_data[i]);
// 	}
// 	free(params->map.map_data);
// 	// Destroy MLX resources
// 	mlx_destroy_image(params->mlx, params->window_img.img);
// 	mlx_destroy_window(params->mlx, params->win);
// }

void findPlayer(t_params *params) {
  for (int i = 0; i < MAP_NUM_ROWS; i++) {
    for (int j = 0; j < MAP_NUM_COLS; j++) {
      if (params->map.map_data[i][j] == 'P') {
        // Initialize player position at the center of the tile
        params->player.x = j * TILE_SIZE + (TILE_SIZE / 2);
        params->player.y = i * TILE_SIZE + (TILE_SIZE / 2);
        params->player.rotationAngle = PI / 2; // Default facing north

        // Set default movement vectors
        params->player.dx = cos(params->player.rotationAngle);
        params->player.dy = sin(params->player.rotationAngle);

        // Replace 'P' with '0' (empty space)
        params->map.map_data[i][j] = '0';
        return;
      }
    }
  }
}

// Function to load textures from XPM files
int load_textures(t_params *params) {
  // Load the North (NO) texture
  params->north_texture.img =
      mlx_xpm_file_to_image(params->mlx, "no.xpm", &params->north_texture.width,
                            &params->north_texture.height);
  if (!params->north_texture.img)
    return (0);
  params->north_texture.addr = mlx_get_data_addr(
      params->north_texture.img, &params->north_texture.bits_per_pixel,
      &params->north_texture.line_length, &params->north_texture.endian);
  params->north_texture.bpp = params->north_texture.bits_per_pixel / 8;

  // Load the South (SO) texture
  params->south_texture.img =
      mlx_xpm_file_to_image(params->mlx, "so.xpm", &params->south_texture.width,
                            &params->south_texture.height);
  if (!params->south_texture.img)
    return (0);
  params->south_texture.addr = mlx_get_data_addr(
      params->south_texture.img, &params->south_texture.bits_per_pixel,
      &params->south_texture.line_length, &params->south_texture.endian);
  params->south_texture.bpp = params->south_texture.bits_per_pixel / 8;

  // Load the East (EA) texture
  params->east_texture.img =
      mlx_xpm_file_to_image(params->mlx, "ea.xpm", &params->east_texture.width,
                            &params->east_texture.height);
  if (!params->east_texture.img)
    return (0);
  params->east_texture.addr = mlx_get_data_addr(
      params->east_texture.img, &params->east_texture.bits_per_pixel,
      &params->east_texture.line_length, &params->east_texture.endian);
  params->east_texture.bpp = params->east_texture.bits_per_pixel / 8;

  // Load the West (WE) texture
  params->west_texture.img =
      mlx_xpm_file_to_image(params->mlx, "we.xpm", &params->west_texture.width,
                            &params->west_texture.height);
  if (!params->west_texture.img)
    return (0);
  params->west_texture.addr = mlx_get_data_addr(
      params->west_texture.img, &params->west_texture.bits_per_pixel,
      &params->west_texture.line_length, &params->west_texture.endian);
  params->west_texture.bpp = params->west_texture.bits_per_pixel / 8;

  return (1);
}

// Function to get color from texture at specified coordinates
unsigned int get_texture_color(t_img *texture, int x, int y) {
  char *dst;

  if (x < 0 || y < 0 || x >= texture->width || y >= texture->height)
    return (0);
  dst = texture->addr + (y * texture->line_length + x * texture->bpp);
  return (*(unsigned int *)dst);
}

// Modified wall rendering function to use textures
// void renderWallProjection(t_params *params) {
//   // Define colors for ceiling and floor
//   int ceilingColor = 0x444444; // Gray for ceiling
//   int floorColor = 0x888888;   // Darker gray for floor

//   for (int x = 0; x < NUM_RAYS; x++) {
//     // Calculate perpendicular distance to avoid fisheye effect
//     float perpDistance =
//         params->rays[x].distance *
//         cos(params->rays[x].rayAngle - params->player.rotationAngle);

//     // Calculate projected wall height
//     float projectedWallHeight = (TILE_SIZE / perpDistance) * DIST_PROJ_PLANE;
//     int wallStripHeight = (int)projectedWallHeight;

//     // Calculate wall top and bottom positions
//     int wallTopPixel = (WINDOW_HEIGHT / 2) - (wallStripHeight / 2);
//     wallTopPixel = wallTopPixel < 0 ? 0 : wallTopPixel;

//     int wallBottomPixel = (WINDOW_HEIGHT / 2) + (wallStripHeight / 2);
//     wallBottomPixel =
//         wallBottomPixel > WINDOW_HEIGHT ? WINDOW_HEIGHT : wallBottomPixel;

//     // Determine which texture to use based on wall orientation
//     t_img *texture;
//     if (params->rays[x].wasHitVertical) {
//       // Vertical wall hit
//       if (isRayFacingLeft(params->rays[x].rayAngle)) {
//         texture =
//             &params->east_texture; // Ray facing left hits east-facing wall
//       } else {
//         texture =
//             &params->west_texture; // Ray facing right hits west-facing wall
//       }
//     } else {
//       // Horizontal wall hit
//       if (isRayFacingUp(params->rays[x].rayAngle)) {
//         texture =
//             &params->south_texture; // Ray facing up hits south-facing wall
//       } else {
//         texture =
//             &params->north_texture; // Ray facing down hits north-facing wall
//       }
//     }

//     // Calculate texture x-coordinate
//     float wallHitX = params->rays[x].wallHitX;
//     float wallHitY = params->rays[x].wallHitY;
//     int textureOffsetX;

//     if (params->rays[x].wasHitVertical) {
//       // If vertical wall hit, use y-coordinate for texture mapping
//       textureOffsetX = (int)wallHitY % TILE_SIZE;
//     } else {
//       // If horizontal wall hit, use x-coordinate for texture mapping
//       textureOffsetX = (int)wallHitX % TILE_SIZE;
//     }

//     // Fix texture direction if needed
//     if ((params->rays[x].wasHitVertical &&
//          isRayFacingLeft(params->rays[x].rayAngle)) ||
//         (!params->rays[x].wasHitVertical &&
//          isRayFacingUp(params->rays[x].rayAngle))) {
//       textureOffsetX = TILE_SIZE - textureOffsetX - 1;
//     }

//     // Draw ceiling
//     for (int y = 0; y < wallTopPixel; y++) {
//       put_pixel(params, x, y, ceilingColor);
//     }

//     // Draw wall with texture
//     for (int y = wallTopPixel; y < wallBottomPixel; y++) {
//       // Calculate texture y-coordinate
//       int distFromTop = y - wallTopPixel;
//       int textureOffsetY =
//           distFromTop * ((float)texture->height / wallStripHeight);

//       // Get color from texture
//       unsigned int texelColor = get_texture_color(
//           texture, (textureOffsetX * texture->width) / TILE_SIZE,
//           textureOffsetY);

//       put_pixel(params, x, y, texelColor);
//     }

//     // Draw floor
//     for (int y = wallBottomPixel; y < WINDOW_HEIGHT; y++) {
//       put_pixel(params, x, y, floorColor);
//     }
//   }
// }

void renderWallProjection(t_params *params) {
	// Define colors for ceiling and floor
	int ceilingColor = 0x444444; // Gray for ceiling
	int floorColor = 0x888888;   // Darker gray for floor

	for (int x = 0; x < NUM_RAYS; x++) {
	  // Calculate perpendicular distance to avoid fisheye effect
	  float rayAngle = params->rays[x].rayAngle;
	  float playerAngle = params->player.rotationAngle;
	  float angleDifference = rayAngle - playerAngle;

	  // perpDistance = distance * cos(angle_difference)
	  float perpDistance = params->rays[x].distance * cos(angleDifference);

	  // Calculate projected wall height using the perpendicular distance
	  // Avoid division by zero or very small numbers if perpDistance is tiny
	  float projectedWallHeight = 0;
	  if (perpDistance > 0.0001) { // Add a small epsilon check
		  projectedWallHeight = (TILE_SIZE / perpDistance) * DIST_PROJ_PLANE;
	  } else {
		  projectedWallHeight = WINDOW_HEIGHT; // Render as infinitely tall if too close
	  }

	  int wallStripHeight = (int)projectedWallHeight;

	  // Calculate wall top and bottom positions
	  int wallTopPixel = (WINDOW_HEIGHT / 2) - (wallStripHeight / 2);
	  wallTopPixel = wallTopPixel < 0 ? 0 : wallTopPixel;

	  int wallBottomPixel = (WINDOW_HEIGHT / 2) + (wallStripHeight / 2);
	  wallBottomPixel =
		  wallBottomPixel > WINDOW_HEIGHT ? WINDOW_HEIGHT : wallBottomPixel;

	  // Determine which texture to use based on wall orientation
	  t_img *texture;
	  if (params->rays[x].wasHitVertical) {
		// Vertical wall hit
		if (isRayFacingLeft(params->rays[x].rayAngle)) {
		  texture =
			  &params->east_texture; // Ray facing left hits east-facing wall
		} else {
		  texture =
			  &params->west_texture; // Ray facing right hits west-facing wall
		}
	  } else {
		// Horizontal wall hit
		if (isRayFacingUp(params->rays[x].rayAngle)) {
		  texture =
			  &params->south_texture; // Ray facing up hits south-facing wall
		} else {
		  texture =
			  &params->north_texture; // Ray facing down hits north-facing wall
		}
	  }

	  // Calculate texture x-coordinate
	  float wallHitX = params->rays[x].wallHitX;
	  float wallHitY = params->rays[x].wallHitY;
	  int textureOffsetX;

	  if (params->rays[x].wasHitVertical) {
		// If vertical wall hit, use y-coordinate for texture mapping
		textureOffsetX = (int)wallHitY % TILE_SIZE;
	  } else {
		// If horizontal wall hit, use x-coordinate for texture mapping
		textureOffsetX = (int)wallHitX % TILE_SIZE;
	  }

	  // Fix texture mirroring if needed (adjust based on ray direction)
	  if ((params->rays[x].wasHitVertical && isRayFacingLeft(params->rays[x].rayAngle)) ||
		  (!params->rays[x].wasHitVertical && isRayFacingUp(params->rays[x].rayAngle))) {
		  // Invert texture X if looking left at a vertical wall or up at a horizontal wall
		  textureOffsetX = (TILE_SIZE - 1) - textureOffsetX;
	  }

	  // Draw ceiling
	  for (int y = 0; y < wallTopPixel; y++) {
		put_pixel(params, x, y, ceilingColor);
	  }

	  // Draw wall with texture
	  for (int y = wallTopPixel; y < wallBottomPixel; y++) {
		  // Calculate texture y-coordinate ensuring it stays within bounds
		  int distFromTop = y + (wallStripHeight / 2) - (WINDOW_HEIGHT / 2);
		  int textureOffsetY = distFromTop * ((float)texture->height / wallStripHeight);

		  // Clamp texture coordinates to valid range
		  textureOffsetY = textureOffsetY < 0 ? 0 : textureOffsetY;
		  textureOffsetY = textureOffsetY >= texture->height ? texture->height - 1 : textureOffsetY;

		  // Ensure textureOffsetX is also valid (though modulo should handle it)
		  textureOffsetX = textureOffsetX < 0 ? 0 : textureOffsetX;
		  textureOffsetX = textureOffsetX >= TILE_SIZE ? TILE_SIZE - 1 : textureOffsetX;

		  // Map textureOffsetX (0-63) to the actual texture width
		  int finalTextureX = (textureOffsetX * texture->width) / TILE_SIZE;
		  finalTextureX = finalTextureX < 0 ? 0 : finalTextureX;
		  finalTextureX = finalTextureX >= texture->width ? texture->width - 1 : finalTextureX;

		  // Get color from texture
		  unsigned int texelColor = get_texture_color(texture, finalTextureX, textureOffsetY);

		  put_pixel(params, x, y, texelColor);
	  }

	  // Draw floor
	  for (int y = wallBottomPixel; y < WINDOW_HEIGHT; y++) {
		put_pixel(params, x, y, floorColor);
	  }
	}
  }

// Add to cleanup function to free texture resources
void cleanup(t_params *params) {
  int i;

  // Free map data
  for (i = 0; i < params->map.rows; i++) {
    free(params->map.map_data[i]);
  }
  free(params->map.map_data);

  // Destroy texture images
  if (params->north_texture.img)
    mlx_destroy_image(params->mlx, params->north_texture.img);
  if (params->south_texture.img)
    mlx_destroy_image(params->mlx, params->south_texture.img);
  if (params->east_texture.img)
    mlx_destroy_image(params->mlx, params->east_texture.img);
  if (params->west_texture.img)
    mlx_destroy_image(params->mlx, params->west_texture.img);

  // Destroy window image
  mlx_destroy_image(params->mlx, params->window_img.img);
  mlx_destroy_window(params->mlx, params->win);
}

void renderMapPlayer(t_params *params) {
  int minimapX = MINIMAP_MARGIN;
  int minimapY = MINIMAP_MARGIN;

  // Calculate player position on minimap
  int playerX = minimapX + params->player.x * MINIMAP_SCALE_FACTOR;
  int playerY = minimapY + params->player.y * MINIMAP_SCALE_FACTOR;
  int playerSize = 6; // Size of player indicator

  // Draw player as a filled circle with a border
  for (int y = -playerSize; y <= playerSize; y++) {
    for (int x = -playerSize; x <= playerSize; x++) {
      float distance = sqrt(x * x + y * y);
      if (distance <= playerSize) {
        // Draw outer border
        if (distance >= playerSize - 2) {
          put_pixel(params, playerX + x, playerY + y, 0xFFFFFF); // White border
        } else {
          // Draw inner circle
          put_pixel(params, playerX + x, playerY + y, MINIMAP_PLAYER_COLOR);
        }
      }
    }
  }

  // Draw direction indicator (more pronounced)
  float dirLength = playerSize * 2.5;
  int dirEndX = playerX + params->player.dx * dirLength;
  int dirEndY = playerY + params->player.dy * dirLength;

  // Draw direction line with a bit of thickness
  t_point start = {playerX, playerY};
  t_point end = {dirEndX, dirEndY};

  draw_line(params, start, end, 0xFFFFFF); // White line

  // Draw a slightly offset line to make it thicker
  start.x += 1;
  end.x += 1;
  draw_line(params, start, end, 0xFFFFFF);
}

void normalizeAngle(float *angle) {
  *angle = remainder(*angle, TWO_PI);
  if (*angle < 0) {
    *angle = TWO_PI + *angle;
  }
}

float distanceBetweenPoints(float x1, float y1, float x2, float y2) {
  return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

bool isRayFacingDown(float angle) { return angle > 0 && angle < PI; }

bool isRayFacingUp(float angle) { return !isRayFacingDown(angle); }

bool isRayFacingRight(float angle) {
  return angle < 0.5 * PI || angle > 1.5 * PI;
}

bool isRayFacingLeft(float angle) { return !isRayFacingRight(angle); }

void castRay(float rayAngle, int stripId, t_params *params) {
  normalizeAngle(&rayAngle);

  float xintercept, yintercept;
  float xstep, ystep;

  ///////////////////////////////////////////
  // HORIZONTAL RAY-GRID INTERSECTION CODE
  ///////////////////////////////////////////
  bool foundHorzWallHit = false;
  float horzWallHitX = 0;
  float horzWallHitY = 0;
  int horzWallTexture = 0;

  // Find the y-coordinate of the closest horizontal grid intersection
  yintercept = floor(params->player.y / TILE_SIZE) * TILE_SIZE;
  yintercept += isRayFacingDown(rayAngle) ? TILE_SIZE : 0;

  // Find the x-coordinate of the closest horizontal grid intersection
  xintercept =
      params->player.x + (yintercept - params->player.y) / tan(rayAngle);

  // Calculate the increment xstep and ystep
  ystep = TILE_SIZE;
  ystep *= isRayFacingUp(rayAngle) ? -1 : 1;

  xstep = TILE_SIZE / tan(rayAngle);
  xstep *= (isRayFacingLeft(rayAngle) && xstep > 0) ? -1 : 1;
  xstep *= (isRayFacingRight(rayAngle) && xstep < 0) ? -1 : 1;

  float nextHorzTouchX = xintercept;
  float nextHorzTouchY = yintercept;

  // Increment xstep and ystep until we find a wall
  while (isInsideMap(nextHorzTouchX, nextHorzTouchY)) {
    float xToCheck = nextHorzTouchX;
    float yToCheck = nextHorzTouchY + (isRayFacingUp(rayAngle) ? -1 : 0);

    if (mapHasWallAt(xToCheck, yToCheck, params)) {
      // found a wall hit
      horzWallHitX = nextHorzTouchX;
      horzWallHitY = nextHorzTouchY;
      horzWallTexture = getMapAt((int)floor(yToCheck / TILE_SIZE),
                                 (int)floor(xToCheck / TILE_SIZE), params);
      foundHorzWallHit = true;
      break;
    } else {
      nextHorzTouchX += xstep;
      nextHorzTouchY += ystep;
    }
  }

  ///////////////////////////////////////////
  // VERTICAL RAY-GRID INTERSECTION CODE
  ///////////////////////////////////////////
  bool foundVertWallHit = false;
  float vertWallHitX = 0;
  float vertWallHitY = 0;
  int vertWallTexture = 0;

  // Find the x-coordinate of the closest vertical grid intersection
  xintercept = floor(params->player.x / TILE_SIZE) * TILE_SIZE;
  xintercept += isRayFacingRight(rayAngle) ? TILE_SIZE : 0;

  // Find the y-coordinate of the closest vertical grid intersection
  yintercept =
      params->player.y + (xintercept - params->player.x) * tan(rayAngle);

  // Calculate the increment xstep and ystep
  xstep = TILE_SIZE;
  xstep *= isRayFacingLeft(rayAngle) ? -1 : 1;

  ystep = TILE_SIZE * tan(rayAngle);
  ystep *= (isRayFacingUp(rayAngle) && ystep > 0) ? -1 : 1;
  ystep *= (isRayFacingDown(rayAngle) && ystep < 0) ? -1 : 1;

  float nextVertTouchX = xintercept;
  float nextVertTouchY = yintercept;

  // Increment xstep and ystep until we find a wall
  while (isInsideMap(nextVertTouchX, nextVertTouchY)) {
    float xToCheck = nextVertTouchX + (isRayFacingLeft(rayAngle) ? -1 : 0);
    float yToCheck = nextVertTouchY;

    if (mapHasWallAt(xToCheck, yToCheck, params)) {
      // found a wall hit
      vertWallHitX = nextVertTouchX;
      vertWallHitY = nextVertTouchY;
      vertWallTexture = getMapAt((int)floor(yToCheck / TILE_SIZE),
                                 (int)floor(xToCheck / TILE_SIZE), params);
      foundVertWallHit = true;
      break;
    } else {
      nextVertTouchX += xstep;
      nextVertTouchY += ystep;
    }
  }

  // Calculate both horizontal and vertical hit distances and choose the
  // smallest one
  float horzHitDistance =
      foundHorzWallHit
          ? distanceBetweenPoints(params->player.x, params->player.y,
                                  horzWallHitX, horzWallHitY)
          : FLT_MAX;
  float vertHitDistance =
      foundVertWallHit
          ? distanceBetweenPoints(params->player.x, params->player.y,
                                  vertWallHitX, vertWallHitY)
          : FLT_MAX;

  if (vertHitDistance < horzHitDistance) {
    params->rays[stripId].distance = vertHitDistance;
    params->rays[stripId].wallHitX = vertWallHitX;
    params->rays[stripId].wallHitY = vertWallHitY;
    params->rays[stripId].wasHitVertical = true;
    params->rays[stripId].rayAngle = rayAngle;
  } else {
    params->rays[stripId].distance = horzHitDistance;
    params->rays[stripId].wallHitX = horzWallHitX;
    params->rays[stripId].wallHitY = horzWallHitY;
    params->rays[stripId].wasHitVertical = false;
    params->rays[stripId].rayAngle = rayAngle;
  }
}

void castAllRays(t_params *params) {
  float rayAngle = params->player.rotationAngle - (FOV_ANGLE / 2);

  for (int stripId = 0; stripId < NUM_RAYS; stripId++) {
    castRay(rayAngle, stripId, params);
    rayAngle += FOV_ANGLE / NUM_RAYS;
  }
}

void renderMapRays(t_params *params) {
  int minimapX = MINIMAP_MARGIN;
  int minimapY = MINIMAP_MARGIN;

  // Calculate player position on minimap
  int playerX = minimapX + params->player.x * MINIMAP_SCALE_FACTOR;
  int playerY = minimapY + params->player.y * MINIMAP_SCALE_FACTOR;

  // Draw rays on minimap - draw fewer rays for better visual clarity
  for (int i = 0; i < NUM_RAYS; i += 60) {
    float rayAlpha = 0.6; // Starting opacity for rays

    t_point start = {playerX, playerY};
    t_point end = {minimapX + params->rays[i].wallHitX * MINIMAP_SCALE_FACTOR,
                   minimapY + params->rays[i].wallHitY * MINIMAP_SCALE_FACTOR};

    // Adjust alpha based on ray distance for a nice fading effect
    float distFactor =
        1.0 - (params->rays[i].distance / (MAP_NUM_COLS * TILE_SIZE));
    rayAlpha *=
        distFactor > 0.2 ? distFactor : 0.2; // Don't let it fade completely

    // Get RGB components of the ray color
    int red = (MINIMAP_RAY_COLOR >> 16) & 0xFF;
    int green = (MINIMAP_RAY_COLOR >> 8) & 0xFF;
    int blue = MINIMAP_RAY_COLOR & 0xFF;

    // Apply alpha to create the final color
    int final_red = red * rayAlpha;
    int final_green = green * rayAlpha;
    int final_blue = blue * rayAlpha;
    int rayColor = (final_red << 16) | (final_green << 8) | final_blue;

    draw_line(params, start, end, rayColor);
  }
}

// Main render function
int render(t_params *params) {
  // Update game state
  update(params);

  // Clear the screen
  clear_screen(params);

  // Cast all rays
  castAllRays(params);

  // Render 3D walls
  renderWallProjection(params);

  // Render the minimap in the top-left corner
  renderMapGrid(params);
  renderMapRays(params);
  renderMapPlayer(params);

  // Put the image to the window
  mlx_put_image_to_window(params->mlx, params->win, params->window_img.img, 0,
                          0);

  return (0);
}

// Updated main function
int main(void) {
  t_params params;

  // Initialize all values to 0
  memset(&params, 0, sizeof(t_params));

  // Initialize MLX window
  if (!init_mlx(&params)) {
    printf("Error: Failed to initialize MLX\n");
    return (EXIT_FAILURE);
  }

  // Initialize window image
  if (!init_image(&params)) {
    printf("Error: Failed to initialize window image\n");
    mlx_destroy_window(params.mlx, params.win);
    return (EXIT_FAILURE);
  }

  // Initialize player
  init_player(&params);

  // Initialize map
  init_map(&params);

  // Find player on map
  findPlayer(&params);

  // Load textures
  if (!load_textures(&params)) {
    printf("Error: Failed to load textures\n");
    cleanup(&params);
    return (EXIT_FAILURE);
  }

  // Set up key hooks
  mlx_hook(params.win, 2, 1L << 0, key_press, &params);   // Key press
  mlx_hook(params.win, 3, 1L << 1, key_release, &params); // Key release

  // Set up render loop
  mlx_loop_hook(params.mlx, render, &params);

  // Start the MLX loop
  mlx_loop(params.mlx);

  // Clean up resources (this won't be reached unless mlx_loop returns)
  cleanup(&params);

  return (EXIT_SUCCESS);
}
