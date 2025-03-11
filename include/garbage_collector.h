/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   garbage_collector.h                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: msennane <msennane@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/03 15:03:49 by msennane          #+#    #+#             */
/*   Updated: 2024/12/24 18:23:17 by msennane         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// garbage_collector.h
#ifndef GARBAGE_COLLECTOR_H
# define GARBAGE_COLLECTOR_H

# include <stdlib.h>

typedef struct s_gc_node
{
	void				*ptr;
	struct s_gc_node	*next;
}						t_gc_node;

typedef struct s_garbage_collector
{
	t_gc_node			*allocations;
	int					exit_status;
}						t_garbage_collector;

extern t_garbage_collector	g_gc;

void					*gc_malloc(size_t size);
void					gc_free_all(void);

#endif // GARBAGE_COLLECTOR_H
