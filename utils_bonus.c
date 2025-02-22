/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_bonus.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yabarhda <yabarhda@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/21 08:51:55 by yabarhda          #+#    #+#             */
/*   Updated: 2025/02/08 09:03:54 by yabarhda         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "main_bonus.h"

void	open_pipes(t_list *args)
{
	int	i;

	i = 0;
	args->pipe = (int **)malloc(sizeof(int *) * (args->ac - 1));
	if (!args->pipe)
		return ;
	while (i < (args->ac - 1))
	{
		args->pipe[i] = (int *)malloc(sizeof(int) * 2);
		if (!args->pipe[i] || pipe(args->pipe[i]) == -1)
			free_n_exit(args, errno);
		i++;
	}
}

void	close_all_pipes(t_list *args)
{
	int	i;

	i = 0;
	while (i < (args->ac - 1))
	{
		close(args->pipe[i][0]);
		close(args->pipe[i][1]);
		i++;
	}
}

void	exec_cmd(char *a[], t_list *s, int i, char *envp[])
{
	char	**av;

	if (i == 0)
		(check_in(a, s), dup2(s->in, 0), close(s->in));
	else
		dup2(s->pipe[i - 1][0], 0);
	if (i == (s->ac - 1))
		(check_out(a, s), dup2(s->out, 1), close(s->out));
	else
		dup2(s->pipe[i][1], 1);
	close_all_pipes(s);
	av = ft_split(a[i + 2 + s->o_ap], ' ');
	if (!av || !av[0])
		(print_error(a[0], 13, av[0]), free(av), free_n_exit(s, 126));
	if (execve(filename(av[0], envp, s), av, envp) == -1)
	{
		if (errno == 2)
			(print_error(a[0], 127, av[0]), free_arr(av), free_n_exit(s, 127));
		else
		{
			(print_error(a[0], errno, av[0]), free_arr(av));
			free_n_exit(s, 126);
		}
	}
}

void	here_doc_read(char *argv[], t_list *args)
{
	char	*line;

	args->o_ap = 1;
	pipe(args->fds);
	while (1)
	{
		write(1, "heredoc> ", 9);
		line = get_next_line(0, 1);
		if (!line)
			break ;
		if (!ft_strncmp(argv[2], line))
			break ;
		write(args->fds[1], line, ft_strlen(line));
		free(line);
	}
	free(line);
	get_next_line(0, 0);
	close(args->fds[1]);
}

void	main_handle(char *argv[], t_list *args, char *envp[])
{
	int		i;

	i = 0;
	args->pid = (pid_t *)malloc(sizeof(pid_t) * args->ac);
	if (!args->pid)
		exit(1);
	open_pipes(args);
	while (i < args->ac)
	{
		args->pid[i] = fork();
		if (args->pid[i] == -1)
			free_n_exit(args, errno);
		if (args->pid[i] == 0)
			exec_cmd(argv, args, i, envp);
		i++;
	}
	close_all_pipes(args);
	i = 0;
	while (i < args->ac && waitpid(args->pid[i], &args->status, 0) > 0)
		i++;
	if (WIFEXITED(args->status))
		free_n_exit(args, WEXITSTATUS(args->status));
	else if (WIFSIGNALED(args->status))
		free_n_exit(args, WTERMSIG(args->status));
}
