/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_cmp.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbaltaza <dbaltaza@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/11 22:24:38 by dbaltaza          #+#    #+#             */
/*   Updated: 2026/07/15 16:05:17 by dbaltaza         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	higher_priority(t_hnode a, t_hnode b)
{
	if (a.key != b.key)
		return (a.key < b.key);
	return (a.id < b.id);
}
