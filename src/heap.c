/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbaltaza <dbaltaza@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/11 22:24:36 by dbaltaza          #+#    #+#             */
/*   Updated: 2026/07/10 22:55:55 by dbaltaza         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	heap_push(t_heap *heap, int id, long key)
{
	int	i;

	heap->nodes[heap->size].id = id;
	heap->nodes[heap->size].key = key;
	i = heap->size;
	heap->size++;
	while (i > 0 && higher_priority(heap->nodes[i], heap->nodes[(i - 1) / 2]))
	{
		swap(&heap->nodes[i], &heap->nodes[(i - 1) / 2]);
		i = (i - 1) / 2;
	}
}

void	swap(t_hnode *a, t_hnode *b)
{
	t_hnode	tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

t_hnode	heap_pop(t_heap *heap)
{
	t_hnode	top;
	int		i;
	int		child;

	top = heap->nodes[0];
	heap->nodes[0] = heap->nodes[heap->size - 1];
	heap->size--;
	i = 0;
	while (2 * i + 1 < heap->size)
	{
		child = 2 * i + 1;
		if (child + 1 < heap->size
			&& higher_priority(heap->nodes[child + 1], heap->nodes[child]))
			child = child + 1;
		if (!higher_priority(heap->nodes[child], heap->nodes[i]))
			break ;
		swap(&heap->nodes[i], &heap->nodes[child]);
		i = child;
	}
	return (top);
}
