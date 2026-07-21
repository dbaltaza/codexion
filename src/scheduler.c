/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbaltaza <dbaltaza@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/11 22:24:29 by dbaltaza          #+#    #+#             */
/*   Updated: 2026/07/15 16:05:23 by dbaltaza         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long	queue_key(t_sim *sim, int id)
{
	if (sim->cfg.scheduler == POLICY_FIFO)
		return (sim->seq++);
	return (sim->coders[id - 1].last_compile_start + sim->cfg.time_to_burnout);
}

int	is_my_turn(t_sim *sim, int id)
{
	return (sim->queue.size > 0 && sim->queue.nodes[0].id == id
		&& can_compile(sim, id));
}

int	acquire(t_sim *sim, int id)
{
	struct timespec	deadline;

	pthread_mutex_lock(&sim->lock);
	heap_push(&sim->queue, id, queue_key(sim, id));
	while (!sim->stop && !is_my_turn(sim, id))
	{
		ms_to_timespec(sim->cfg.dongle_cooldown, &deadline);
		pthread_cond_timedwait(&sim->avail, &sim->lock, &deadline);
	}
	if (sim->stop)
	{
		pthread_mutex_unlock(&sim->lock);
		return (0);
	}
	heap_pop(&sim->queue);
	take_dongles(sim, id);
	pthread_mutex_unlock(&sim->lock);
	return (1);
}

void	release(t_sim *sim, int id)
{
	pthread_mutex_lock(&sim->lock);
	sim->coders[id - 1].counter++;
	release_dongles(sim, id);
	pthread_cond_broadcast(&sim->avail);
	pthread_mutex_unlock(&sim->lock);
}
