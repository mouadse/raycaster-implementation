#include "../include/cub3d.h"

char	map[19][30] = {"11111111111111111111111111111",
		"10000000000000001000000000001", "10111111111111101011111111101",
		"101         10101000000000101", "101 1111111 10101111111110101",
		"101 1000001 10000000000010101", "101 1011111111111111111010101",
		"101 1010000000000000001010101", "101 1010111111111111101110101",
		"101 1010100000000000100000101", "101 1010101111111110111111101",
		"101 1010101      10100000001", "101 1010101111111110111111101",
		"101 1010100000000000000000101", "101 1010111111111111111110101",
		"101 1010000000000000000010101", "101 1111111111111111111110101",
		"10S0000000000000000000000001", "11111111111111111111111111111"};

int	main(void)
{
	t_game	*game;

	game = malloc(sizeof(t_game));
	if (!game)
	{
		perror("Error\nMalloc failed\n");
		return (1);
	}
	// Convert the stack allocated map to heap allocated map
	game->map.grid = malloc(sizeof(char *) * 19);
	if (!game->map.grid)
	{
		perror("Error\nMalloc failed\n");
		return (1);
	}
	for (int i = 0; i < 19; i++)
	{
		game->map.grid[i] = malloc(sizeof(char) * 30);
		if (!game->map.grid[i])
		{
			perror("Error\nMalloc failed\n");
			return (1);
		}
		ft_strlcpy(game->map.grid[i], map[i], 30);
	}
	game->map.width = 30;
	game->map.height = 19;
	// Print the map
	for (int i = 0; i < game->map.height; i++)
	{
		for (int j = 0; j < game->map.width; j++)
			write(1, &game->map.grid[i][j], 1);
		write(1, "\n", 1);
	}
	// Validate the map
	if (!is_map_valid(game))
	{
		ft_putstr_fd("Error\nMap is not valid\n", STDERR_FILENO);
	}else{
		ft_putstr_fd("Map is valid\n", STDOUT_FILENO);
	}
	// Free the map
	for (int i = 0; i < 19; i++)
		free(game->map.grid[i]);
	free(game->map.grid);
	free(game);
	return (0);
}
