/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbaltaza <dbaltaza@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/21 00:19:05 by dbaltaza          #+#    #+#             */
/*   Updated: 2026/07/15 15:59:21 by dbaltaza         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <unistd.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <pthread.h>
# include <sys/time.h>

typedef enum e_sched
{
	POLICY_FIFO,
	POLICY_EDF
}	t_sched;

typedef struct s_config
{
	int			n_coders;
	long		time_to_burnout;
	long		time_to_compile;
	long		time_to_debug;
	long		time_to_refactor;
	int			number_of_compiles_required;
	long		dongle_cooldown;
	t_sched		scheduler;
}	t_config;

typedef struct s_hnode
{
	int		id;
	long	key;
}	t_hnode;

typedef struct s_heap
{
	t_hnode	*nodes;
	int		size;
	int		capacity;
	t_sched	policy;
}	t_heap;

typedef struct s_sim	t_sim;

typedef struct s_coder
{
	int			id;
	int			counter;
	long		last_compile_start;
	pthread_t	thread;
	t_sim		*sim;
}	t_coder;

typedef struct s_dongle
{
	int		occupied;
	int		owner;
	long	released_at;
}	t_dongle;

typedef struct s_sim
{
	t_config		cfg;
	t_coder			*coders;
	t_dongle		*dongles;
	pthread_t		monitor;
	pthread_mutex_t	lock;
	pthread_mutex_t	print_lock;
	pthread_cond_t	avail;
	long			start_time;
	int				stop;
	t_heap			queue;
	long			seq;
}	t_sim;

int		is_number(char *s);
int		validate_args(int argc, char **argv);
void	fill_config(char **argv, t_config *cfg);
int		parse_args(int argc, char **argv, t_config *cfg);
int		init_sim(t_sim *sim, t_config *cfg);
int		init_locks(t_sim *sim);
void	init_coders(t_sim *sim);
void	init_dongles(t_sim *sim);
int		init_heap(t_sim *sim);

void	*coder_routine(void *arg);
void	start_coders(t_sim *sim);
void	join_coders(t_sim *sim);

long	now_ms(void);
void	ms_to_timespec(long ms_from_now, struct timespec *ts);

void	log_state(t_sim *sim, int id, char *state);

int		dongle_free(t_sim *sim, int index);
int		can_compile(t_sim *sim, int id);
void	take_dongles(t_sim *sim, int id);
void	release_dongles(t_sim *sim, int id);

int		acquire(t_sim *sim, int id);
void	release(t_sim *sim, int id);
int		is_my_turn(t_sim *sim, int id);
long	queue_key(t_sim *sim, int id);

int		stopped(t_sim *sim);
int		check_burnout(t_sim *sim);
int		all_done(t_sim *sim);
long	next_wait_ms(t_sim *sim);
void	*monitor_routine(void *arg);

void	cleanup_sim(t_sim *sim);
void	free_arrays(t_sim *sim);

int		higher_priority(t_hnode a, t_hnode b);

void	heap_push(t_heap *heap, int id, long key);
void	swap(t_hnode *a, t_hnode *b);
t_hnode	heap_pop(t_heap *heap);

#endif
