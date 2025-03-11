/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   queue_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: msennane <msennane@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/21 18:52:49 by msennane          #+#    #+#             */
/*   Updated: 2025/03/09 02:11:27 by msennane         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/cub3d.h"

void	init_queue(t_queue *queue)
{
	queue->front = NULL;
	queue->rear = NULL;
}

void	enqueue(t_queue *queue, void *data)
{
	t_node	*new_node;

	new_node = (t_node *)gc_malloc(sizeof(t_node));
	if (!new_node)
		return ;
	new_node->data = data;
	new_node->next = NULL;
	if (queue->rear)
		queue->rear->next = new_node;
	queue->rear = new_node;
	if (!queue->front)
		queue->front = queue->rear;
}

void	*dequeue(t_queue *queue)
{
	t_node	*tmp;
	void	*data;

	if (!queue || !queue->front)
		return (NULL);
	tmp = queue->front;
	data = tmp->data;
	queue->front = queue->front->next;
	if (!queue->front)
		queue->rear = NULL;
	return (data);
}

void	*queue_str_convert(t_queue *queue)
{
	int		len;
	char	*str;
	char	*line;
	t_node	*tmp;

	if (!queue || !queue->front)
		return (NULL);
	len = 0;
	tmp = queue->front;
	while (tmp)
	{
		if (tmp->data)
			len += ft_strlen((char *)tmp->data);
		tmp = tmp->next;
	}
	str = (char *)ft_calloc(len + 1, sizeof(char));
	while (queue->front)
	{
		line = dequeue(queue);
		if (line)
			ft_strlcat(str, line, len + 1);
	}
	return (str);
}

void	free_queue(t_queue *queue)
{
	if (!queue)
		return ;
	while (queue->front)
	{
		queue->front = queue->front->next;
	}
	queue->rear = NULL;
}
