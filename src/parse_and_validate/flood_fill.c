#include "../../include/cub3d.h"

int			is_valid_map(t_map *map);

static void	free_map_copy(char **copy)
{
	int	i;

	i = 0;
	if (!copy)
		return ;
	while (copy[i])
		free(copy[i++]);
	free(copy);
}

t_map	*create_map(const char **map_data)
{
	t_map	*map;
	int		rows;
	int		cols;
	int		i;
	int		len;

	map = malloc(sizeof(t_map));
	rows = 0;
	cols = 0;
	if (!map)
		return (NULL);
	// Count rows and find max cols
	for (i = 0; map_data[i]; i++)
	{
		rows++;
		len = strlen(map_data[i]);
		if (len > cols)
			cols = len;
	}
	// Allocate map_data
	map->map_data = malloc(sizeof(char *) * (rows + 1));
	if (!map->map_data)
	{
		free(map);
		return (NULL);
	}
	// Copy map data
	for (i = 0; i < rows; i++)
	{
		map->map_data[i] = ft_strdup(map_data[i]);
		if (!map->map_data[i])
		{
			// Clean up on failure
			while (--i >= 0)
				free(map->map_data[i]);
			free(map->map_data);
			free(map);
			return (NULL);
		}
	}
	map->map_data[rows] = NULL;
	map->rows = rows;
	map->cols = cols;
	return (map);
}

void	free_map(t_map *map)
{
	int	i;

	i = 0;
	if (!map)
		return ;
	if (map->map_data)
	{
		while (map->map_data[i])
			free(map->map_data[i++]);
		free(map->map_data);
	}
	free(map);
}

void	print_map(t_map *map)
{
	int	i;

	i = 0;
	printf("Map (%d rows, %d cols):\n", map->rows, map->cols);
	while (map->map_data[i])
	{
		printf("[%2d] %s\n", i, map->map_data[i]);
		i++;
	}
	printf("\n");
}

void	test_map(const char *name, const char **map_data)
{
	t_map	*map;

	if (!name || !map_data)
	{
		printf("Error: Invalid test data\n");
		return ;
	}
	printf("\n===== Testing Map: %s =====\n", name);
	map = create_map(map_data);
	if (!map)
	{
		printf("Failed to create map\n");
		return ;
	}
	print_map(map);
	printf("Validation result: ");
	if (is_valid_map(map))
		printf("VALID ✓\n");
	else
		printf("INVALID ✗\n");
	free_map(map);
}

static int	is_player_position(char c)
{
	return (c == 'N' || c == 'S' || c == 'E' || c == 'W');
}

static int	is_walkable(char c)
{
	return (c == '0' || is_player_position(c));
}

/**
 * Find player starting position
 */
static int	find_player_position(t_map *map, int *x, int *y)
{
	int	i;
	int	j;
	int	found;

	i = 0;
	found = 0;
	while (map->map_data[i])
	{
		j = 0;
		while (map->map_data[i][j])
		{
			if (is_player_position(map->map_data[i][j]))
			{
				*x = j;
				*y = i;
				found++;
			}
			j++;
		}
		i++;
	}
	if (found != 1)
	{
		printf("Error: Expected 1 player position, found %d\n", found);
		return (0);
	}
	return (1);
}

static int	check_valid_chars(t_map *map)
{
	int	i;
	int	j;

	i = 0;
	while (map->map_data[i])
	{
		j = 0;
		while (map->map_data[i][j])
		{
			if (!is_valid_map_char(map->map_data[i][j]))
			{
				printf("Error\nInvalid character in map: {%c}\n",
					map->map_data[i][j]);
				return (0);
			}
			j++;
		}
		i++;
	}
	return (1);
}

/**
 * Create a duplicate of the map for flood fill
 */
static char	**create_map_copy(t_map *map)
{
	char	**copy;
	int		i;

	i = 0;
	copy = (char **)malloc(sizeof(char *) * (map->rows + 1));
	if (!copy)
		return (NULL);
	while (map->map_data[i])
	{
		copy[i] = ft_strdup(map->map_data[i]);
		if (!copy[i])
		{
			// Free previously allocated rows on failure
			while (--i >= 0)
				free(copy[i]);
			free(copy);
			return (NULL);
		}
		i++;
	}
	copy[i] = NULL;
	return (copy);
}

/**
 * Fixed Flood fill algorithm with proper bounds checking
 */
static void	flood_fill(char **map_copy, int x, int y)
{
	// Check if current position is valid and walkable
	if (y < 0 || x < 0 || !map_copy[y] || map_copy[y][x] == '\0'
		|| !is_walkable(map_copy[y][x]) || map_copy[y][x] == 'V')
		return ;
	// Debug print
	printf("DEBUG: Filling cell at (%d,%d) = '%c'\n", x, y, map_copy[y][x]);
	// Mark current cell as visited
	map_copy[y][x] = 'V';
	// Recursively fill in all four directions
	flood_fill(map_copy, x + 1, y); // right
	flood_fill(map_copy, x - 1, y); // left
	flood_fill(map_copy, x, y + 1); // down
	flood_fill(map_copy, x, y - 1); // up
}

/**
 * Print map copy with visited cells for debugging
 */
static void	print_flood_map(char **map_copy)
{
	int	i;

	i = 0;
	printf("\nFlood fill result:\n");
	while (map_copy[i])
	{
		printf("[%2d] %s\n", i, map_copy[i]);
		i++;
	}
	printf("\n");
}

/**
 * Check if the flood fill reached any invalid positions
 */
static int	check_flood_boundaries(char **map_copy, t_map *map)
{
	int	i;
	int	j;

	i = 0;
	while (map_copy[i])
	{
		j = 0;
		while (map_copy[i][j])
		{
			// Only check visited cells
			if (map_copy[i][j] == 'V')
			{
				// Check if cell is on map edge
				if (i == 0 || i == map->rows - 1 || j == 0 || map_copy[i][j
					+ 1] == '\0')
				{
					printf("Error\nMap is not enclosed by walls (edge boundary at %d,%d)\n", j, i);
					return (0);
				}
				// Check for spaces adjacent to visited cells
				if ((j > 0 && map_copy[i][j - 1] == ' ') || (map_copy[i][j + 1]
						&& map_copy[i][j + 1] == ' ') || (i > 0 && map_copy[i
						- 1] && j < strlen(map_copy[i - 1]) && map_copy[i
						- 1][j] == ' ') || (map_copy[i + 1]
						&& j < strlen(map_copy[i + 1]) && map_copy[i
						+ 1][j] == ' '))
				{
					printf("Error\nWalkable area adjacent to space at %d,%d\n", j, i);
					return (0);
				}
			}
			j++;
		}
		i++;
	}
	return (1);
}

/**
 * Check if all walkable areas were reached by the flood fill
 */
static int	check_all_walkable_reached(char **map_copy)
{
	int	i;
	int	j;

	i = 0;
	while (map_copy[i])
	{
		j = 0;
		while (map_copy[i][j])
		{
			// If there's a walkable cell that wasn't visited
			if (is_walkable(map_copy[i][j]) && map_copy[i][j] != 'V')
			{
				printf("Error\nIsolated walkable area detected at %d,%d\n", j, i);
				return (0);
			}
			j++;
		}
		i++;
	}
	return (1);
}

/**
 * Main map validation function using flood fill
 */
int	is_valid_map(t_map *map)
{
	char	**map_copy;
	int		result;

	int player_x, player_y;
	result = 1;
	// Check basic map requirements
	if (!check_valid_chars(map) || !find_player_position(map, &player_x,
			&player_y))
		return (0);
	printf("DEBUG: Player position found at (%d,%d) = '%c'\n", player_x,
		player_y, map->map_data[player_y][player_x]);
	// Create a copy of the map for flood fill
	map_copy = create_map_copy(map);
	if (!map_copy)
	{
		printf("Error\nMemory allocation failed\n");
		return (0);
	}
	// Perform flood fill from player position
	flood_fill(map_copy, player_x, player_y);
	// Print the flood-filled map for debugging
	print_flood_map(map_copy);
	// Check if flood fill results are valid
	if (!check_flood_boundaries(map_copy, map))
		result = 0;
	else if (!check_all_walkable_reached(map_copy))
		result = 0;
	// Free the map copy
	free_map_copy(map_copy);
	return (result);
}
