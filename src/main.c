#include "../include/cub3d.h"
#include <stdio.h>

int	main(void)
{
	t_game	*game;
	char	*map_file;
	char	*line;
	char	*temp;
	int		fd;

	map_file = "map.cub";
	fd = open(map_file, O_RDONLY);
	if (fd < 0)
	{
		return (ONE);
	}
	game = malloc(sizeof(t_game));
	initialize_textures_data(&(game->textures));
	temp = get_next_line(fd);
	while (temp)
	{
		line = temp;
		while (ft_isspace(*temp))
			temp++;
		parse_scene_element(&(game->textures), temp, line);
		free(line);
		if (game->textures.south_path && game->textures.west_path
			&& game->textures.east_path) //  && game->textures.colors_complete
		{
			printf("Textures and colors are complete\n");
			break ;
		}
		temp = get_next_line(fd);
	}
	if (!temp)
	{
		ft_putstr_fd("Error\n", 2);
		ft_putstr_fd("Incomplete scene data\n", 2);
		return (ONE);
	}
	close(fd);
	printf("NO: %s\n", game->textures.north_path);
	printf("SO: %s\n", game->textures.south_path);
	printf("WE: %s\n", game->textures.west_path);
	printf("EA: %s\n", game->textures.east_path);
	free(game);
	return (ZERO);
}
