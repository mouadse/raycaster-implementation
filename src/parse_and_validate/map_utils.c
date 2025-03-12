/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   map_utils.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: msennane <msennane@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/12 19:48:10 by msennane          #+#    #+#             */
/*   Updated: 2025/03/12 19:52:31 by msennane         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/cub3d.h"

int	is_player_pos(char c)
{
	return (c == PLAYER_NORTH || c == PLAYER_SOUTH || c == PLAYER_EAST
		|| c == PLAYER_WEST); // using enums instead of hardcoded values or magic numbers
}
