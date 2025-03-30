#ifndef CUB3D_H
# define CUB3D_H

/********** Includes: Standard Libraries **********/

# include <assert.h>
# include <errno.h>
# include <fcntl.h>
# include <limits.h>
# include <math.h>
# include <stdbool.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>

/********** Includes: External Libraries **********/
# include "../libft/libft.h"
# include "../mlx/mlx.h"
# include "garbage_collector.h"
# include "queue.h"

/********** No Magic Numbers **********/

# define ZERO 0
# define ONE 1

/********** Enums **********/

/**
 * Enum for texture directions
 */

typedef enum e_texture_type
{
	NORTH = 0,
	SOUTH = 1,
	WEST = 2,
	EAST = 3
}				t_texture_type;

/**
 * Enum for color types
 */
typedef enum e_color_type
{
	FLOOR = 0,
	CEILING = 1
}				t_color_type;

/**
 * Enum for map elements
 */
typedef enum e_map_element
{
	EMPTY = '0',
	WALL = '1',
	SPACE = ' ',
	PLAYER_NORTH = 'N',
	PLAYER_SOUTH = 'S',
	PLAYER_EAST = 'E',
	PLAYER_WEST = 'W',
	VISITED = 'X', // Used in flood fill
	FILL = 'F'     // Used to replace spaces for flood fill
}				t_map_element;

/********** Structures **********/

typedef struct s_textures
{
	char		*north_path;
	char		*south_path;
	char		*west_path;
	char		*east_path;
	bool		colors_complete;
	int			floor_color_count;
	int			ceiling_color_count;
	int			element_count;
}				t_textures;

typedef struct s_colors
{
	int			floor_r;
	int			floor_g;
	int			floor_b;
	int			ceiling_r;
	int			ceiling_g;
	int			ceiling_b;
}				t_colors;

typedef struct s_map
{
	int			cols;
	int			rows;
	char		**map_data;
}				t_map;

typedef struct s_player
{
	double		x;
	double		y;
	double		dx;
	double		dy;
	double		direction;
	double		fov;
}				t_player;

typedef struct s_game
{
	t_textures	textures;
	t_colors	colors;
	t_player	player;
	t_map		map;
	// Additional MLX-related fields will go here
}				t_game;

/********** Function Prototypes **********/
int				error_handler(char *message);
int				is_valid_map_char(char c);
void			set_player_direction(t_player *player);
int				starts_with(char *str, char *prefix);
int				is_empty_line(char *line);
int				is_cub_file(char *filename);
// void			flood_fill(char **map, int x, int y, int width, int height);
// int				flood_fill_validation(char **map, int width, int height);
int				validate_map(t_game *game);
int				validate_textures(t_game *game);
int				validate_game(t_game *game);
int				parse_map(char **map_lines, t_game *game);
int				parse_color(char *line, int color_type, t_game *game);
int				parse_texture(char *line, int texture_type, t_game *game);
int				parse_file(char *filename, t_game *game);
void			free_game(t_game *game);
int				file_exists(char *file_path);
int				is_map_line(char *line);
int				is_numeric(char *str);
int				is_player_pos(char c);
int				is_map_valid(t_game *game);
int				is_valid_map(t_map *map);
void			test_map(const char *name, const char **map_data);
int				ft_isspace(char c);
unsigned int	convert_rgb(int r, int g, int b);
void			process_rgb(unsigned int *color, char *rgb_color,
					char *original, char del);
void			initialize_textures_data(t_textures *textures);
void			parse_scene_element(t_textures *textures, char *identifier,
					char *line_buffer);
/********** Error Messages **********/

/********** Graphics **********/

# define WINDOW_WIDTH 1024
# define WINDOW_HEIGHT 1024
# define RESOLUTION 1024
# define TILE_SIZE 64
# define TEXTURE_SIZE 64
# define MOVE_SPEED 40
# define ROTATE_SPEED 0.2
# define HORIZONTAL 0
# define VERTICAL 1

typedef struct s_point
{
	int			x;
	int			y;
}				t_point;

typedef struct s_fpoint
{
	float		x;
	float		y;
}				t_fpoint;

typedef struct s_img
{
	void		*img;
	char		*addr;
	int			bits_per_pixel;
	int			bpp;
	int			line_length;
	int			endian;
	int			width;
	int			height;
}				t_img;

typedef struct s_ray
{
	double		x;
	double		y;
	double		direction;
	double		length;
	int			hit;
}				t_ray;

typedef struct s_wall
{
	double		wall_height;
	char		*wall_texture;
	t_img		*texture_img;
	int			texture_x;
	int			texture_y;
	int			wall_y;
}				t_wall;

typedef struct s_params
{
	void		*mlx;
	void		*win;
	t_img		window_img;

	t_map		map;
	// t_map_infos			map_infos; // To be implemented later

	t_player	player;
	t_ray		ray;

	t_img		north_texture;
	t_img		south_texture;
	t_img		west_texture;
	t_img		east_texture;

	int			floor_color;
	int			ceiling_color;
	double  dist_proj_plane; // Distance to projection plane for 3D rendering
	t_wall		wall;

}				t_params;

int				put_pixel(t_params *params, int x, int y, int color);
void			bresenham_algorithm(t_point p1, t_point p2, t_point *delta,
					t_point *sign);
void			draw_line(t_params *params, t_point p1, t_point p2, int color);
void			draw_line_img(t_params *params, t_point p1, t_point p2,
					int color);

t_fpoint		find_vertical_wall_intersection(t_params *params,
					double ray_angle);
t_fpoint		find_horizontal_wall_intersection(t_params *params,
					double ray_angle);
double			calculate_euclidean_distance(double x1, double y1, double x2,
					double y2);

#endif // CUB3D_H
