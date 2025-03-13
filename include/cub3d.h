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

typedef struct s_player
{
	double		pos_x;
	double		pos_y;
	double		dir_x;
	double		dir_y;
	char orientation; // 'N', 'S', 'E', or 'W'
}				t_player;

typedef struct s_map
{
	char		**grid;
	int			width;
	int			height;
}				t_map;

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
void			flood_fill(char **map, int x, int y, int width, int height);
int				flood_fill_validation(char **map, int width, int height);
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
int				ft_isspace(char c);
unsigned int	convert_rgb(int r, int g, int b);
void			process_rgb(unsigned int *color, char *rgb_color,
					char *original, char del);

/********** Error Messages **********/

#endif // CUB3D_H
