/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   log.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbaltaza <dbaltaza@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/11 22:24:32 by dbaltaza          #+#    #+#             */
/*   Updated: 2026/07/05 15:51:05 by dbaltaza         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	log_state(t_sim *sim, int id, char *state)
{
	long	ts;

	pthread_mutex_lock(&sim->print_lock);
	ts = now_ms() - sim->start_time;
	printf("%ld %d %s\n", ts, id, state);
	pthread_mutex_unlock(&sim->print_lock);
}
