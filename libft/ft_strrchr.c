/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strrchr.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: msennane <msennane@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/05 17:09:56 by msennane          #+#    #+#             */
/*   Updated: 2023/11/27 01:08:22 by msennane         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strrchr(const char *s, int c)
{
	size_t	i;
	char	*str;

	i = 0;
	str = (char *)s;
	while (str[i])
		i++;
	while (i > 0)
	{
		if (str[i] == (char)c)
			return (&str[i]);
		i--;
	}
	if (str[i] == (char)c)
		return (&str[i]);
	return (NULL);
}
