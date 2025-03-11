/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line.h                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: msennane <msennane@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/28 01:08:05 by msennane          #+#    #+#             */
/*   Updated: 2024/12/03 15:21:51 by msennane         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GET_NEXT_LINE_H
# define GET_NEXT_LINE_H
# include "../include/garbage_collector.h"
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>

# ifndef BUFFER_SIZE
#  define BUFFER_SIZE 1024
# endif

struct					s_list_node
{
	char				data;
	struct s_list_node	*next;
};

typedef struct s_list
{
	struct s_list_node	*head;
	struct s_list_node	*tail;
}						t_list;

void					initialize_queue(t_list *node);
void					push_node(t_list *node, char c);
char					pop_node(t_list *node);
int						has_new_line(t_list *node);
int						is_it_empty(t_list *node);
char					*get_next_line(int fd);

#endif
