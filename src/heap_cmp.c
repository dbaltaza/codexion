/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_cmp.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbaltaza <dbaltaza@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/11 22:24:38 by dbaltaza          #+#    #+#             */
/*   Updated: 2026/07/15 16:05:17 by dbaltaza         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	higher_priority(t_hnode a, t_hnode b)
{
	if (a.key != b.key)
		return (a.key < b.key);
	if (a.seq != b.seq)
		return (a.seq < b.seq);
	return (a.id < b.id);
}

int	heap_find(t_heap *heap, int id)
{
	int	i;

	i = 0;
	while (i < heap->size)
	{
		if (heap->nodes[i].id == id)
			return (i);
		i++;
	}
	return (-1);
}

void	heap_remove(t_heap *heap, int i)
{
	if (i < 0 || i >= heap->size)
		return ;
	heap->size--;
	if (i == heap->size)
		return ;
	heap->nodes[i] = heap->nodes[heap->size];
	sift_up(heap, i);
	sift_down(heap, i);
}

int	shares_dongle(int a, int b, int n)
{
	if (a == b)
		return (1);
	return (a % n == b - 1 || b % n == a - 1);
}

int	blocked_by_queue(t_sim *sim, int id)
{
	int		i;
	t_hnode	me;

	i = heap_find(&sim->queue, id);
	if (i < 0)
		return (1);
	me = sim->queue.nodes[i];
	i = 0;
	while (i < sim->queue.size)
	{
		if (sim->queue.nodes[i].id != id
			&& shares_dongle(sim->queue.nodes[i].id, id, sim->cfg.n_coders)
			&& higher_priority(sim->queue.nodes[i], me)
			&& can_compile(sim, sim->queue.nodes[i].id))
			return (1);
		i++;
	}
	return (0);
}
