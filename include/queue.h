/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   queue.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: msennane <msennane@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/30 00:35:15 by msennane          #+#    #+#             */
/*   Updated: 2024/11/21 16:32:15 by msennane         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef QUEUE_H
# define QUEUE_H

typedef struct s_node
{
	void				*data;
	struct s_node		*next;
}						t_node;

typedef struct s_queue
{
	t_node				*front;
	t_node				*rear;
}						t_queue;

void					init_queue(t_queue *queue);
void					enqueue(t_queue *queue, void *data);
void					*dequeue(t_queue *queue);
void					*queue_str_convert(t_queue *queue);
void					free_queue(t_queue *queue);

/* =====	============= 2nd Queue type interfaces ================== */

typedef struct s_node_char
{
	char				data;
	struct s_node_char	*next;
}						t_node_char;

typedef struct s_queue_char
{
	t_node_char			*front;
	t_node_char			*rear;
}						t_queue_char;

void					init_queue_char(t_queue_char *queue);
void					enqueue_char(t_queue_char *queue, char data);
void					enqueue_str(t_queue_char *queue, char *str);
char					dequeue_char(t_queue_char *queue);
char					*queue_char_str_convert(t_queue_char *queue);
void					free_queue_char(t_queue_char *queue);

#endif
