#include "../../include/cub3d.h"

int parse_color(char *line, int color_type, t_game *game)
{
    char *colors;
    char **rgb;
    int r, g, b;

	if (!line || !game || !*line)
		return (0);
    colors = line + 1;
    while (*colors && *colors == ' ')
        colors++;
	// ToDo - Check for whitespaces and invalid characters
    rgb = ft_split(colors, ',');
    if (!rgb || !rgb[0] || !rgb[1] || !rgb[2] || rgb[3])
    {
        // free_array(rgb);
        return (0);
    }

    r = ft_atoi(rgb[0]);
    g = ft_atoi(rgb[1]);
    b = ft_atoi(rgb[2]);

    // Validate range (0-255)
    if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255)
    {
        // free_array(rgb);
        return (0);
    }

    if (color_type == FLOOR)
    {
        game->colors.floor_r = r;
        game->colors.floor_g = g;
        game->colors.floor_b = b;
    }
    else if (color_type == CEILING)
    {
        game->colors.ceiling_r = r;
        game->colors.ceiling_g = g;
        game->colors.ceiling_b = b;
    }

    // free_array(rgb);
    return (1);
}
