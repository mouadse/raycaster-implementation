/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   colors_validation.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: msennane <msennane@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/12 19:48:22 by msennane          #+#    #+#             */
/*   Updated: 2025/03/14 01:59:41 by msennane         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/cub3d.h"

unsigned int	convert_rgb(int r, int g, int b)
{
	// Visualization of the conversion: [Red byte][Green byte][Blue byte][Alpha byte]
	// 0xFF000000 - Red byte
	// 0x00FF0000 - Green byte
	// 0x0000FF00 - Blue byte
	// 0x000000FF - Alpha byte
	// Example: 0x00FF0000 | 0x0000FF00 | 0x000000FF | 0xFF000000 = 0x00FF00FF
	return (r << 24 | g << 16 | b << 8 | 255);
}
// Helper function to handle errors and cleanup
static void	handle_error(char **rgb, char *original, char *message)
{
	int	j;

	j = 0;
	ft_putstr_fd("Error\n", 2);
	ft_putstr_fd(message, 2);
	ft_putstr_fd(original, 2);
	ft_putstr_fd("\n", 2);
	if (rgb)
	{
		while (rgb[j])
			free(rgb[j++]);
		free(rgb);
	}
	exit(ONE);
}

// Helper function to free RGB array
static void	free_rgb_array(char **rgb)
{
	int	i;

	i = 0;
	if (rgb)
	{
		while (rgb[i])
			free(rgb[i++]);
		free(rgb);
	}
}

void	process_rgb(unsigned int *color, char *rgb_color, char *original,
		char del)
{
	char	**rgb;
	int		del_count;
	int		space_count;
	char	*tmp;
	char	*trimmed;
	int		i;
	int		j;

	if (!color || !rgb_color)
		handle_error(NULL, original ? original : "NULL",
			"Invalid color parameters: ");
	rgb = NULL;
	del_count = 0;
	space_count = 0;
	// Check for required delimiter and spaces
	tmp = rgb_color;
	while (*tmp && (ft_isspace(*tmp) || *tmp == del))
	{
		if (*tmp == del)
			del_count++;
		if (ft_isspace(*tmp))
			space_count++;
		tmp++;
	}
	// We need exactly one delimiter and at least one space
	if (del_count != 1 || space_count < 1)
		handle_error(NULL, original, "Invalid color format: ");
	// Split the string after the delimiter
	rgb = ft_split(tmp, ',');
	if (!rgb)
		handle_error(NULL, original, "Memory allocation failed: ");
	// Count components
	for (i = 0; rgb[i]; i++)
		;
	// Validate we have exactly 3 components (R,G,B)
	if (i != 3)
		handle_error(rgb, original,
			"Color must have exactly 3 components (R,G,B): ");
	// Process each color component
	for (i = 0; rgb[i]; i++)
	{
		// Check for invalid components (newlines, leading spaces)
		if (ft_strncmp(rgb[i], "\n", 1) == 0 || ft_isspace(rgb[i][0]))
			handle_error(rgb, original, "Invalid color component: ");
		// Trim whitespace
		trimmed = ft_strtrim(rgb[i], " \t\v\f\r\n\b");
		free(rgb[i]);
		if (!trimmed)
			handle_error(rgb, original, "Memory allocation failed: ");
		rgb[i] = trimmed;
		// Validate that all characters are digits
		for (j = 0; rgb[i][j]; j++)
		{
			if (!ft_isdigit(rgb[i][j]))
				handle_error(rgb, original,
					"Color component must be a number: ");
		}
		// Validate range (0-255)
		if (ft_atoi(rgb[i]) < 0 || ft_atoi(rgb[i]) > 255)
			handle_error(rgb, original,
				"Color component must be between 0-255: ");
		printf("The rgb[%d] is %s\n", i, rgb[i]);
	}
	// Convert to color format
	*color = convert_rgb(ft_atoi(rgb[0]), ft_atoi(rgb[1]), ft_atoi(rgb[2]));
	// Free memory
	free_rgb_array(rgb);
}
