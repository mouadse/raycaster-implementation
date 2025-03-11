/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_substr.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: msennane <msennane@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/06 16:20:42 by msennane          #+#    #+#             */
/*   Updated: 2024/12/03 15:24:11 by msennane         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_substr(char const *s, unsigned int start, size_t len)
{
	char	*sub_str;
	size_t	new_len;

	if (!s)
		return (NULL);
	if (ft_strlen(s) < start)
	{
		sub_str = gc_malloc(sizeof(char));
		if (!sub_str)
			return (NULL);
		sub_str[0] = '\0';
	}
	else
	{
		new_len = ft_strlen(s + start);
		if (new_len < len)
			len = new_len;
		sub_str = gc_malloc(sizeof(char) * (len + 1));
		if (!sub_str)
			return (NULL);
		ft_strlcpy(sub_str, s + start, len + 1);
	}
	return (sub_str);
}
