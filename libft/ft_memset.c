/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memset.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: msennane <msennane@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/05 20:15:10 by msennane          #+#    #+#             */
/*   Updated: 2023/12/05 18:11:57 by msennane         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	*ft_memset(void *s, int c, size_t n)
{
	size_t			i;
	unsigned char	*tab;

	i = 0;
	tab = (unsigned char *)s;
	while (i < n)
	{
		tab[i] = (unsigned char)c;
		i++;
	}
	return (s);
}
