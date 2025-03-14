#include "../include/cub3d.h"
#include <stdio.h>
#include "../include/cub3d.h"
#include <stdio.h>

/*
 * Fixed main function with proper error handling and memory management
 */

// Function to free allocated texture resources
static void	free_textures(t_textures *textures)
{
	if (!textures)
		return ;
	if (textures->north_path)
		free(textures->north_path);
	if (textures->south_path)
		free(textures->south_path);
	if (textures->west_path)
		free(textures->west_path);
	if (textures->east_path)
		free(textures->east_path);
}

// Function to clean up resources before exit
static void	cleanup_and_exit(t_game *game, int fd, char *line, int exit_code)
{
	if (fd >= 0)
		close(fd);
	if (line)
		free(line);
	if (game)
	{
		free_textures(&(game->textures));
		free(game);
	}
	exit(exit_code);
}

// Function to check if all required textures and colors are defined
static bool	is_scene_data_complete(t_textures *textures)
{
	return (textures->north_path && textures->south_path && textures->west_path
		&& textures->east_path && textures->colors_complete);
}

// Function to display scene data (for debugging/verification)
static void	display_scene_data(t_textures *textures)
{
	printf("North texture: %s\n",
		textures->north_path ? textures->north_path : "Not set");
	printf("South texture: %s\n",
		textures->south_path ? textures->south_path : "Not set");
	printf("West texture: %s\n",
		textures->west_path ? textures->west_path : "Not set");
	printf("East texture: %s\n",
		textures->east_path ? textures->east_path : "Not set");
	printf("Colors complete: %s\n", textures->colors_complete ? "Yes" : "No");
}

int	main(int argc, char **argv)
{
	t_game	*game;
	char	*line;
	int		fd;
	char	*map_file;
	char	*input_line;
	char	*trimmed;

	game = NULL;
	line = NULL;
	fd = -1;
	if (argc > 1)
		map_file = argv[1];
	else
		map_file = "map.cub";
	fd = open(map_file, O_RDONLY);
	if (fd < 0)
	{
		ft_putstr_fd("Error\n", 2);
		ft_putstr_fd("Failed to open map file: ", 2);
		ft_putstr_fd(map_file, 2);
		ft_putstr_fd("\n", 2);
		return (EXIT_FAILURE);
	}
	game = malloc(sizeof(t_game));
	if (!game)
	{
		ft_putstr_fd("Error\n", 2);
		ft_putstr_fd("Memory allocation failed\n", 2);
		cleanup_and_exit(NULL, fd, NULL, EXIT_FAILURE);
	}
	initialize_textures_data(&(game->textures));
	while ((input_line = get_next_line(fd)) != NULL)
	{
		line = input_line;
		trimmed = line;
		while (trimmed && ft_isspace(*trimmed))
			trimmed++;
		if (trimmed && *trimmed != '\0' && *trimmed != '\n')
			parse_scene_element(&(game->textures), trimmed, line);
		free(line);
		line = NULL;
		if (is_scene_data_complete(&(game->textures)))
			break ;
	}
	if (!is_scene_data_complete(&(game->textures)))
	{
		ft_putstr_fd("Error\n", 2);
		ft_putstr_fd("Incomplete scene data\n", 2);
		cleanup_and_exit(game, fd, line, EXIT_FAILURE);
	}
	display_scene_data(&(game->textures));
	cleanup_and_exit(game, fd, line, EXIT_SUCCESS);
	return (EXIT_SUCCESS);
}
