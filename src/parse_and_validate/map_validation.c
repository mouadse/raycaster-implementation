#include "../../include/cub3d.h"
#include <stdio.h>

int	validate_textures(t_game *game)
{
	if (!game->textures.north_path || !game->textures.south_path
		|| !game->textures.west_path || !game->textures.east_path)
		return (ZERO);
	if (!file_exists(game->textures.north_path)
		|| !file_exists(game->textures.south_path)
		|| !file_exists(game->textures.west_path)
		|| !file_exists(game->textures.east_path))
		return (ZERO);
	return (ONE);
}
static int	validate_map_helper(t_game *game, int i, int j, int *count)
{
	char	c;

	c = game->map.grid[i][j];
	if (!is_valid_map_char(c))
	{
		ft_putstr_fd("Error\nInvalid character in map\n", STDERR_FILENO);
		// print coordinates of invalid character
		printf("Coordinates: (%d, %d)\n", i, j);
		return (ZERO);
	}
	if (is_player_pos(c))
		(*count)++; // To check if there is only one player in the map
	if (i == 0 || i == game->map.height - 1 || j == 0 || j == game->map.width
		- 1)
	{
		if (c != '1' && c != ' ')
		{
			ft_putstr_fd("Error\nMap is not surrounded by walls\n",
				STDERR_FILENO);
			printf("Coordinates: (%d, %d)\n", i, j);
			return (ZERO);
		}
	}
	else if (c == '0' || is_player_pos(c))
	{
		if (game->map.grid[i][j + 1] == ' ' || game->map.grid[i][j - 1] == ' '
			|| game->map.grid[i + 1][j] == ' ' || game->map.grid[i
			- 1][j] == ' ')
		{
			ft_putstr_fd("Error\nMap is not surrounded by walls\n",
				STDERR_FILENO);
			printf("Coordinates: (%d, %d)\n", i, j);
			return (ZERO);
		}
	}
	return (ONE);
}

int	is_map_valid(t_game *game)
{
	int	i;
	int	j;
	int	count;

	count = i = 0;
	while (game->map.grid[i])
	{
		j = 0;
		while (game->map.grid[i][j])
		{
			if (!validate_map_helper(game, i, j, &count))
				return (ZERO);
			j++;
		}
		i++;
	}
	if (count != 1)
	{
		ft_putstr_fd("Error\nMap must contain exactly one player\n",
			STDERR_FILENO);
		return (ZERO);
	}
	return (ONE);
}
