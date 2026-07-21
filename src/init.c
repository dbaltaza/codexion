/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbaltaza <dbaltaza@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/11 22:24:34 by dbaltaza          #+#    #+#             */
/*   Updated: 2026/07/08 17:03:58 by dbaltaza         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	init_coders(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->cfg.n_coders)
	{
		sim->coders[i].id = i + 1;
		sim->coders[i].counter = 0;
		sim->coders[i].sim = sim;
		sim->coders[i].last_compile_start = sim->start_time;
		i++;
	}
}

void	init_dongles(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->cfg.n_coders)
	{
		sim->dongles[i].occupied = 0;
		sim->dongles[i].owner = -1;
		sim->dongles[i].released_at = 0;
		i++;
	}
}

int	init_locks(t_sim *sim)
{
	if (pthread_mutex_init(&sim->lock, NULL) != 0)
		return (1);
	if (pthread_mutex_init(&sim->print_lock, NULL) != 0)
	{
		pthread_mutex_destroy(&sim->lock);
		return (1);
	}
	if (pthread_cond_init(&sim->avail, NULL) != 0)
	{
		pthread_mutex_destroy(&sim->lock);
		pthread_mutex_destroy(&sim->print_lock);
		return (1);
	}
	return (0);
}

int	init_heap(t_sim *sim)
{
	sim->queue.nodes = malloc(sim->cfg.n_coders * sizeof(t_hnode));
	if (!sim->queue.nodes)
		return (1);
	sim->queue.size = 0;
	sim->queue.capacity = sim->cfg.n_coders;
	sim->queue.policy = sim->cfg.scheduler;
	sim->seq = 0;
	return (0);
}

int	init_sim(t_sim *sim, t_config *cfg)
{
	sim->cfg = *cfg;
	sim->coders = malloc(cfg->n_coders * sizeof(t_coder));
	sim->dongles = malloc(cfg->n_coders * sizeof(t_dongle));
	if (!sim->coders || !sim->dongles)
	{
		free(sim->coders);
		free(sim->dongles);
		return (1);
	}
	sim->start_time = now_ms();
	sim->stop = 0;
	init_coders(sim);
	init_dongles(sim);
	if (init_heap(sim) != 0)
	{
		free_arrays(sim);
		return (1);
	}
	if (init_locks(sim) != 0)
	{
		free_arrays(sim);
		return (1);
	}
	return (0);
}
