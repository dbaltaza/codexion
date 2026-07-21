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

void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	while (!stopped(coder->sim))
	{
		if (!acquire(coder->sim, coder->id))
			break ;
		log_state(coder->sim, coder->id, "is compiling");
		usleep(coder->sim->cfg.time_to_compile * 1000);
		release(coder->sim, coder->id);
		log_state(coder->sim, coder->id, "is debugging");
		usleep(coder->sim->cfg.time_to_debug * 1000);
		log_state(coder->sim, coder->id, "is refactoring");
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
