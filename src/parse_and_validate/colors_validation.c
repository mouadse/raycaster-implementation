/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   colors_validation.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: msennane <msennane@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/12 19:48:22 by msennane          #+#    #+#             */
/*   Updated: 2025/03/13 18:00:29 by msennane         ###   ########.fr       */
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
