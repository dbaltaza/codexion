/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbaltaza <dbaltaza@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/05 14:42:05 by dbaltaza          #+#    #+#             */
/*   Updated: 2026/06/21 18:19:37 by dbaltaza         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	is_number(char *s)
{
	int	i;

	i = 0;
	while (s[i])
	{
		if (!(s[i] >= '0' && s[i] <= '9'))
			return (0);
		i++;
	}
	if (!(i))
		return (0);
	return (1);
}

int	validate_args(int argc, char **argv)
{
	int	i;

	i = 1;
	if (argc != 9)
		return (1);
	while (i <= 7)
	{
		if (!(is_number(argv[i])))
			return (1);
		i++;
	}
	if (atoi(argv[1]) < 1)
		return (1);
	if (strcmp(argv[8], "fifo") != 0 && strcmp(argv[8], "edf") != 0)
		return (1);
	return (0);
}

void	fill_config(char **argv, t_config *cfg)
{
	cfg->n_coders = atoi(argv[1]);
	cfg->time_to_burnout = atoi(argv[2]);
	cfg->time_to_compile = atoi(argv[3]);
	cfg->time_to_debug = atoi(argv[4]);
	cfg->time_to_refactor = atoi(argv[5]);
	cfg->number_of_compiles_required = atoi(argv[6]);
	cfg->dongle_cooldown = atoi(argv[7]);
	if (strcmp(argv[8], "fifo") == 0)
		cfg->scheduler = POLICY_FIFO;
	else
		cfg->scheduler = POLICY_EDF;
}

int	parse_args(int argc, char **argv, t_config *cfg)
{
	if (validate_args(argc, argv) != 0)
		return (1);
	fill_config(argv, cfg);
	return (0);
}
