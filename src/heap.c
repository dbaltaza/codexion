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

void	swap(t_hnode *a, t_hnode *b)
{
	t_hnode	tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

void	sift_up(t_heap *heap, int i)
{
	while (i > 0 && higher_priority(heap->nodes[i], heap->nodes[(i - 1) / 2]))
	{
		swap(&heap->nodes[i], &heap->nodes[(i - 1) / 2]);
		i = (i - 1) / 2;
	}
}

void	sift_down(t_heap *heap, int i)
{
	int	child;

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
}

void	heap_push(t_heap *heap, int id, long key, long seq)
{
	heap->nodes[heap->size].id = id;
	heap->nodes[heap->size].key = key;
	heap->nodes[heap->size].seq = seq;
	heap->size++;
	sift_up(heap, heap->size - 1);
}
