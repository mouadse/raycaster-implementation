/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: msennane <msennane@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/28 01:08:01 by msennane          #+#    #+#             */
/*   Updated: 2024/12/03 15:28:45 by msennane         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line.h"

static char	*extract_line(t_list *node)
{
	char				*line;
	int					i;
	struct s_list_node	*temp;

	i = 0;
	temp = node->head;
	while (temp && temp->data != '\n')
	{
		temp = temp->next;
		i++;
	}
	line = gc_malloc(sizeof(char) * (i + 2));
	if (!line)
		return (NULL);
	i = 0;
	while (node->head && node->head->data != '\n')
		line[i++] = pop_node(node);
	line[i++] = pop_node(node);
	line[i] = '\0';
	return (line);
}

static char	*extract_rest(t_list *node)
{
	char				*rest;
	int					i;
	struct s_list_node	*temp;

	i = 0;
	temp = node->head;
	while (temp)
	{
		temp = temp->next;
		i++;
	}
	rest = gc_malloc(sizeof(char) * (i + 1));
	if (!rest)
		return (NULL);
	i = 0;
	while (node->head)
		rest[i++] = pop_node(node);
	rest[i] = '\0';
	return (rest);
}

static char	*handle_cases(ssize_t bytes, t_list *node, char *rest, int *fd)
{
	ssize_t	i;

	if (bytes == -1)
	{
		return (NULL);
	}
	else if (bytes == 0)
	{
		if (is_it_empty(node))
			return (NULL);
		else
			return (extract_rest(node));
	}
	else
	{
		i = 0;
		while (i < bytes)
			push_node(node, rest[i++]);
		return (get_next_line(*fd));
	}
}

char	*get_next_line(int fd)
{
	static t_list	node;
	ssize_t			bytes;
	char			*rest;
	static int		flag = 0;

	if (fd < 0 || BUFFER_SIZE <= 0)
		return (NULL);
	if (flag == 0)
	{
		initialize_queue(&node);
		flag = 1;
	}
	if (!is_it_empty(&node) && has_new_line(&node))
		return (extract_line(&node));
	rest = gc_malloc(sizeof(char) * (BUFFER_SIZE));
	if (!rest)
		return (NULL);
	bytes = read(fd, rest, BUFFER_SIZE);
	return (handle_cases(bytes, &node, rest, &fd));
}
