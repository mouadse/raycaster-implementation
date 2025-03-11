/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_itoa.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: msennane <msennane@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/08 03:05:55 by msennane          #+#    #+#             */
/*   Updated: 2024/12/03 15:23:38 by msennane         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static int	calculate_len(int n)
{
	int	len;

	len = 0;
	while (n)
	{
		n /= 10;
		len++;
	}
	return (len);
}

static void	gen_char_array(char *int_str, long number, int len, int sign)
{
	while (number && len--)
	{
		int_str[len + sign] = (number % 10) + 48;
		number /= 10;
	}
}

char	*ft_itoa(int n)
{
	int		sign;
	int		len;
	char	*int_str;
	long	number;

	number = (long)n;
	if (n == 0)
		return (ft_strdup("0"));
	sign = 0;
	if (n < 0)
	{
		sign = 1;
		number = -number;
	}
	len = calculate_len(number);
	int_str = (char *)gc_malloc(sizeof(char) * (len + sign + 1));
	if (!int_str)
		return (NULL);
	if (sign)
		int_str[0] = '-';
	int_str[len + sign] = '\0';
	gen_char_array(int_str, number, len, sign);
	return (int_str);
}
