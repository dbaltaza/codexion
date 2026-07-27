/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbaltaza <dbaltaza@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/11 22:24:44 by dbaltaza          #+#    #+#             */
/*   Updated: 2026/07/07 00:33:48 by dbaltaza         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	log_running(t_sim *sim, int id, char *state)
{
	pthread_mutex_lock(&sim->lock);
	if (sim->stop)
	{
		pthread_mutex_unlock(&sim->lock);
		return (0);
	}
	log_state(sim, id, state);
	pthread_mutex_unlock(&sim->lock);
	return (1);
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	while (!stopped(coder->sim))
	{
		if (!acquire(coder->sim, coder->id))
			break ;
		usleep(coder->sim->cfg.time_to_compile * 1000);
		if (!release(coder->sim, coder->id))
			break ;
		usleep(coder->sim->cfg.time_to_debug * 1000);
		if (!log_running(coder->sim, coder->id, "is refactoring"))
			break ;
		usleep(coder->sim->cfg.time_to_refactor * 1000);
	}
	return (NULL);
}

void	start_coders(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->cfg.n_coders)
	{
		pthread_create(&sim->coders[i].thread, NULL,
			coder_routine, &sim->coders[i]);
		i++;
	}
}

void	join_coders(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->cfg.n_coders)
	{
		pthread_join(sim->coders[i].thread, NULL);
		i++;
	}
}
