/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbaltaza <dbaltaza@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/11 22:24:40 by dbaltaza          #+#    #+#             */
/*   Updated: 2026/07/08 13:31:00 by dbaltaza         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	dongle_free(t_sim *sim, int index)
{
	long	now;

	now = now_ms();
	return (sim->dongles[index].occupied == 0
		&& now - sim->dongles[index].released_at >= sim->cfg.dongle_cooldown);
}

int	can_compile(t_sim *sim, int id)
{
	int	i;

	i = id - 1;
	if (sim->cfg.n_coders == 1)
		return (0);
	return (dongle_free(sim, i)
		&& dongle_free(sim, (i + 1) % sim->cfg.n_coders));
}

void	take_dongles(t_sim *sim, int id)
{
	int	i;

	i = id - 1;
	sim->dongles[i].occupied = 1;
	sim->dongles[i].owner = id;
	sim->dongles[(i + 1) % sim->cfg.n_coders].occupied = 1;
	sim->dongles[(i + 1) % sim->cfg.n_coders].owner = id;
	sim->coders[id - 1].last_compile_start = now_ms();
	log_state(sim, id, "has taken a dongle");
	log_state(sim, id, "has taken a dongle");
}

void	release_dongles(t_sim *sim, int id)
{
	int		i;
	long	now;

	i = id - 1;
	now = now_ms();
	sim->dongles[i].occupied = 0;
	sim->dongles[i].released_at = now;
	sim->dongles[(i + 1) % sim->cfg.n_coders].occupied = 0;
	sim->dongles[(i + 1) % sim->cfg.n_coders].released_at = now;
}
