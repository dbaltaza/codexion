/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbaltaza <dbaltaza@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/11 22:24:26 by dbaltaza          #+#    #+#             */
/*   Updated: 2026/07/07 00:20:22 by dbaltaza         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	stopped(t_sim *sim)
{
	int	s;

	pthread_mutex_lock(&sim->lock);
	s = sim->stop;
	pthread_mutex_unlock(&sim->lock);
	return (s);
}

int	check_burnout(t_sim *sim)
{
	int		i;
	long	now;

	now = now_ms();
	i = 0;
	while (i < sim->cfg.n_coders)
	{
		if (now - sim->coders[i].last_compile_start >= sim->cfg.time_to_burnout)
		{
			log_state(sim, sim->coders[i].id, "burned out");
			return (1);
		}
		i++;
	}
	return (0);
}

int	all_done(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->cfg.n_coders)
	{
		if (sim->coders[i].counter < sim->cfg.number_of_compiles_required)
			return (0);
		i++;
	}
	return (1);
}

long	next_wait_ms(t_sim *sim)
{
	int		i;
	long	earliest;
	long	ddl;

	earliest = sim->coders[0].last_compile_start + sim->cfg.time_to_burnout;
	i = 1;
	while (i < sim->cfg.n_coders)
	{
		ddl = sim->coders[i].last_compile_start + sim->cfg.time_to_burnout;
		if (ddl < earliest)
			earliest = ddl;
		i++;
	}
	ddl = earliest - now_ms();
	if (ddl < 0)
		return (0);
	return (ddl);
}

void	*monitor_routine(void *arg)
{
	t_sim			*sim;
	struct timespec	ts;

	sim = (t_sim *)arg;
	pthread_mutex_lock(&sim->lock);
	while (!check_burnout(sim) && !all_done(sim))
	{
		ms_to_timespec(next_wait_ms(sim), &ts);
		pthread_cond_timedwait(&sim->avail, &sim->lock, &ts);
	}
	sim->stop = 1;
	pthread_cond_broadcast(&sim->avail);
	pthread_mutex_unlock(&sim->lock);
	return (NULL);
}
