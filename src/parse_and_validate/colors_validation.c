/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   colors_validation.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: msennane <msennane@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/12 19:48:22 by msennane          #+#    #+#             */
/*   Updated: 2025/03/13 18:14:44 by msennane         ###   ########.fr       */
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

void	process_rgb(unsigned int *color, char *rgb_color, char *original,
		char del)
{
	char	**rgb;
	int 	i;
	int 	j;
	int		del_count;
	int		space_count;
	char	*tmp;

	del_count = space_count = 0;
	tmp = rgb_color;
	while (*tmp && (ft_isspace(*tmp) || *tmp == del))
	{
		if (*tmp == del)
			del_count++;
		if (ft_isspace(*tmp))
			space_count++;
		tmp++;
	}
	if (del_count != 1 || space_count > 0)
	{
		ft_putstr_fd("Error\nInvalid color format: ", 2);
		ft_putstr_fd(original, 2);
		ft_putstr_fd("\n", 2);
		exit(ONE);
	}

	rgb = ft_split(rgb_color, del);
	if(!rgb)
	{
		ft_putstr_fd("Error\nMemory allocation failed\n", 2);
		exit(ONE);
	}

}
