/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memmove.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: msennane <msennane@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/06 01:45:37 by msennane          #+#    #+#             */
/*   Updated: 2023/12/03 16:00:21 by msennane         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	*ft_memmove(void *dst, const void *src, size_t len)
{
	char	*d;
	char	*s;
	char	*lasts;
	char	*lastd;

	if (dst == src)
		return (dst);
	if (!dst && !src)
		return (NULL);
	d = (char *)dst;
	s = (char *)src;
	lasts = s + (len - 1);
	lastd = d + (len - 1);
	if (dst < src)
	{
		while (len--)
			*d++ = *s++;
	}
	else
	{
		while (len--)
			*lastd-- = *lasts--;
	}
	return (dst);
}
