/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbaltaza <dbaltaza@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/11 22:24:22 by dbaltaza          #+#    #+#             */
/*   Updated: 2026/07/08 13:34:12 by dbaltaza         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	main(int argc, char **argv)
{
	t_sim		sim;
	t_config	cfg;

	if (parse_args(argc, argv, &cfg))
		return (1);
	if (init_sim(&sim, &cfg))
		return (1);
	start_coders(&sim);
	pthread_create(&sim.monitor, NULL, monitor_routine, &sim);
	join_coders(&sim);
	pthread_join(sim.monitor, NULL);
	cleanup_sim(&sim);
	return (0);
}
