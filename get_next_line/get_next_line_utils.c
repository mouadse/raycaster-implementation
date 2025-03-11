/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line_utils.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: msennane <msennane@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/28 01:08:03 by msennane          #+#    #+#             */
/*   Updated: 2024/12/03 15:22:03 by msennane         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line.h"

void	initialize_queue(t_list *node)
{
	node->head = NULL;
	node->tail = NULL;
}

void	push_node(t_list *node, char c)
{
	struct s_list_node	*new_node;

	new_node = gc_malloc(sizeof(struct s_list_node));
	if (!new_node)
		return ;
	new_node->data = c;
	new_node->next = NULL;
	if (node->head == NULL)
	{
		node->head = new_node;
		node->tail = new_node;
	}
	else
	{
		node->tail->next = new_node;
		node->tail = new_node;
	}
}

char	pop_node(t_list *node)
{
	struct s_list_node	*temp;
	char				data;

	temp = node->head;
	data = temp->data;
	node->head = node->head->next;
	if (node->head == NULL)
		node->tail = NULL;
	return (data);
}

int	has_new_line(t_list *node)
{
	struct s_list_node	*temp;

	temp = node->head;
	while (temp)
	{
		if (temp->data == '\n')
			return (1);
		temp = temp->next;
	}
	return (0);
}

int	is_it_empty(t_list *node)
{
	return (node->head == NULL);
}
