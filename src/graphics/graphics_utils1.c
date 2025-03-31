/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   graphics_utils1.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: msennane <msennane@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/30 03:41:00 by msennane          #+#    #+#             */
/*   Updated: 2025/03/31 02:03:01 by msennane         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/cub3d.h"

int	put_pixel(t_params *params, int x, int y, int color)
{
	int	pixel_index;

	pixel_index = y * params->window_img.line_length + x
		* params->window_img.bpp;
	if (pixel_index >= 0 && pixel_index < params->window_img.line_length
		* params->window_img.height)
		*(unsigned int *)(params->window_img.addr + pixel_index) = color;
	return (0);
}

// void	bresenham_algorithm(t_point p1, t_point p2, t_point *delta,
// 		t_point *sign)
// {
// 	delta->x = abs(p2.x - p1.x);
// 	delta->y = -abs(p2.y - p1.y);
// 	if (p1.x < p2.x)
// 		sign->x = 1;
// 	else
// 		sign->x = -1;
// 	if (p1.y < p2.y)
// 		sign->y = 1;
// 	else
// 		sign->y = -1;
// }

// void	draw_line(t_params *params, t_point p1, t_point p2, int color)
// {
// 	t_point	d;
// 	t_point	s;
// 	int		err;
// 	int		e2;

// 	bresenham_algorithm(p1, p2, &d, &s);
// 	err = d.x + d.y;
// 	while (1)
// 	{
// 		mlx_pixel_put(params->mlx, params->win, p1.x, p1.y, color);
// 		if (p1.x == p2.x && p1.y == p2.y)
// 			break ;
// 		e2 = 2 * err;
// 		if (e2 >= d.y)
// 		{
// 			err += d.y;
// 			p1.x += s.x;
// 		}
// 		if (e2 <= d.x)
// 		{
// 			err += d.x;
// 			p1.y += s.y;
// 		}
// 	}
// }

// void	draw_line_img(t_params *params, t_point p1, t_point p2, int color)
// {
// 	t_point	d;
// 	t_point	s;
// 	int		err;
// 	int		e2;

// 	bresenham_algorithm(p1, p2, &d, &s);
// 	err = d.x + d.y;
// 	while (1)
// 	{
// 		put_pixel(params, p1.x, p1.y, color);
// 		if (p1.x == p2.x && p1.y == p2.y)
// 			break ;
// 		e2 = 2 * err;
// 		if (e2 >= d.y)
// 		{
// 			err += d.y;
// 			p1.x += s.x;
// 		}
// 		if (e2 <= d.x)
// 		{
// 			err += d.x;
// 			p1.y += s.y;
// 		}
// 	}
// }

void	draw_line(t_params *params, t_point p1, t_point p2, int color)
{
	float	deltaX;
	float	deltaY;
	float	longestSideLength;
	float	xIncrement;
	float	yIncrement;
	float	currentX;
	float	currentY;
	int		steps;

	deltaX = (float)(p2.x - p1.x);
	deltaY = (float)(p2.y - p1.y);
	if (fabsf(deltaX) >= fabsf(deltaY))
	{
		longestSideLength = fabsf(deltaX);
	}
	else
	{
		longestSideLength = fabsf(deltaY);
	}
	if (longestSideLength == 0)
	{
		xIncrement = 0;
		yIncrement = 0;
	}
	else
	{
		xIncrement = deltaX / longestSideLength;
		yIncrement = deltaY / longestSideLength;
	}
	currentX = (float)p1.x;
	currentY = (float)p1.y;
	steps = (int)(longestSideLength + 0.5f);
	for (int i = 0; i <= steps; i++)
	{
		mlx_pixel_put(params->mlx, params->win, (int)roundf(currentX),
			(int)roundf(currentY), color);
		currentX += xIncrement;
		currentY += yIncrement;
	}
}

void	draw_line_img(t_params *params, t_point p1, t_point p2, int color)
{
	float	deltaX;
	float	deltaY;
	float	longestSideLength;
	float	xIncrement;
	float	yIncrement;
	float	currentX;
	float	currentY;
	int		steps;

	deltaX = (float)(p2.x - p1.x);
	deltaY = (float)(p2.y - p1.y);
	if (fabsf(deltaX) >= fabsf(deltaY))
	{
		longestSideLength = fabsf(deltaX);
	}
	else
	{
		longestSideLength = fabsf(deltaY);
	}
	if (longestSideLength == 0)
	{
		xIncrement = 0;
		yIncrement = 0;
	}
	else
	{
		xIncrement = deltaX / longestSideLength;
		yIncrement = deltaY / longestSideLength;
	}
	currentX = (float)p1.x;
	currentY = (float)p1.y;
	steps = (int)(longestSideLength + 0.5f);
	for (int i = 0; i <= steps; i++)
	{
		put_pixel(params, (int)roundf(currentX), (int)roundf(currentY), color);
		currentX += xIncrement;
		currentY += yIncrement;
	}
}
