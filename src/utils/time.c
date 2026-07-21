/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbaltaza <dbaltaza@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/11 22:24:51 by dbaltaza          #+#    #+#             */
/*   Updated: 2026/07/06 18:08:31 by dbaltaza         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../codexion.h"

long	now_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

void	ms_to_timespec(long ms_from_now, struct timespec *ts)
{
	long	total;

	total = now_ms() + ms_from_now;
	ts->tv_sec = total / 1000;
	ts->tv_nsec = (total % 1000) * 1000000;
}
