/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   queue_utils1.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: msennane <msennane@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/21 18:52:49 by msennane          #+#    #+#             */
/*   Updated: 2025/03/09 02:11:38 by msennane         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/cub3d.h"

void	init_queue_char(t_queue_char *queue)
{
	queue->front = NULL;
	queue->rear = NULL;
}

void	enqueue_char(t_queue_char *queue, char data)
{
	t_node_char	*new_node;

	new_node = (t_node_char *)gc_malloc(sizeof(t_node_char));
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

char	dequeue_char(t_queue_char *queue)
{
	t_node_char	*tmp;
	char		data;

	if (!queue || !queue->front)
		return ('\0');
	tmp = queue->front;
	data = tmp->data;
	queue->front = queue->front->next;
	if (!queue->front)
		queue->rear = NULL;
	return (data);
}

void	enqueue_str(t_queue_char *queue, char *str)
{
	int	i;

	i = 0;
	while (str[i])
	{
		enqueue_char(queue, str[i]);
		i++;
	}
}

char	*queue_char_str_convert(t_queue_char *queue)
{
	t_node_char	*tmp;
	char		*str;
	int			i;
	int			len;

	if (!queue || !queue->front)
		return (NULL);
	len = 0;
	tmp = queue->front;
	while (tmp)
	{
		len++;
		tmp = tmp->next;
	}
	str = (char *)gc_malloc(sizeof(char) * (len + 1));
	if (!str)
		return (NULL);
	i = 0;
	while (queue->front)
	{
		str[i++] = dequeue_char(queue);
	}
	str[i] = '\0';
	return (str);
}
