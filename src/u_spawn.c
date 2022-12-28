/*
 * FIG : Facility for Interactive Generation of figures
 * Copyright (c) 1985-1988 by Supoj Sutanthavibul
 * Parts Copyright (c) 1989-2015 by Brian V. Smith
 * Parts Copyright (c) 1991 by Paul King
 * Parts Copyright (c) 2016-2022 by Thomas Loimer
 *
 * Any party obtaining a copy of these files is granted, free of charge, a
 * full and unrestricted irrevocable, world-wide, paid up, royalty-free,
 * nonexclusive right and license to deal in this software and documentation
 * files (the "Software"), including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense and/or sell copies of
 * the Software, and to permit persons who receive copies from any such
 * party to do so, with the only requirement being that the above copyright
 * and this permission notice remain intact.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>
#include <fcntl.h>		/* open(), creat() */
#include <poll.h>
#include <unistd.h>		/* close() */
#include <stdio.h>
#include <stdlib.h>		/* EXIT_SUCCESS */
#include <string.h>		/* strerror() */
#include <sys/stat.h>		/* mode_t, in the open() call */
#include <sys/wait.h>
#ifdef HAVE_POSIX_SPAWNP
#include <spawn.h>
#endif

#include "w_msgpanel.h"

/*
 * Spawn the process argv[0], with the NULL-terminated argument list argv.
 * If any of the fd[] is not -1, re-direct stdin, stdout or stderr to the file
 * descriptor given in fd[0], fd[1] or fd[2], respectively.
 * Return 0 on success, an errno on failure.
 */
static int
spawn_process(pid_t *restrict pid, char *const argv[restrict], int fd[3])
{
#ifdef HAVE_POSIX_SPAWNP
	int				i;
	int				has_fa = 0;
	posix_spawn_file_actions_t	file_actions;

	for (i = 0; i < 3; ++i) {
		if (fd[i] != -1) {
			has_fa = 1;
			break;
		}
	}
	if (has_fa == 1) {
		posix_spawn_file_actions_init(&file_actions);
		for (i = 0; i < 3; ++i) {
			if (fd[i] != -1) {
				/* 0 must be stdin, 1 stdout, and 2 stderr */
				posix_spawn_file_actions_adddup2(
						&file_actions, fd[i], i);
				posix_spawn_file_actions_addclose(
						&file_actions, fd[i]);
			}
		}
	}
	return posix_spawnp(pid, argv[0], has_fa ? &file_actions : NULL,
				NULL, argv, NULL);

#else	/* HAVE_POSIX_SPAWNP undefined: fork() and exec() */
	*pid = fork();
	if (*pid == -1)
		return errno;
	if (*pid == 0) {
		/* the child */
		int	i;
		for (i = 0; i < 3; ++i) {
			if (fd[i] != -1) {
				if (dup2(fd[i], i) == -1)
					return errno;
				close(fd[i]);
			}
		}
		if (execvp(argv[0], argv) == -1)
			return errno;;
	}

	/* the parent */
	return 0;
#endif /* HAVE_POSIX_SPAWNP */
}

/*
 * Return a pointer to a statically allocated buffer containing
 * the concatenated argument list in argv[].
 */
static char *
full_command(char *const argv[restrict])
{
	static char	cmd_buf[256];
	char		*const *c = argv;
	size_t		pos = 0;
	size_t		len;

	while (*c != NULL && (pos + (len = strlen(*c)) < sizeof cmd_buf)) {
		memcpy(cmd_buf + pos, *c, len);
		pos += len;
		cmd_buf[pos++] = ' ';
		++c;
	}
	cmd_buf[pos <= sizeof cmd_buf ? pos - 1 : pos - len - 1] = '\0';
	return cmd_buf;
}

/*
 * Spawn the process argv[0] with the NULL-terminated arguments argv.
 * Search PATH for the command given in argv[0].
 * Write the output of the spawned process to the file descriptor fdout.
 * Standard error is captured in a buffer and reported to the user.
 * Return 0 on success.
 * On error, return -1, and output an error message.
 */
int
spawn_writefd(char *const argv[restrict], int fdout)
{
	int		ret;
	int		pd[2];
	int		fd[3] = {-1, -1, -1};
	pid_t		pid;
	struct pollfd	fds;

	if (pipe(pd)) {
		file_msg("Command %s, cannot create pipe: %s",
				full_command(argv), strerror(errno));
		return -1;
	}

	/* In the child, redirect stdout to fdout, stderr to the pipe */
	fd[1] = fdout;
	fd[2] = pd[1];
	if ((ret = spawn_process(&pid, argv, fd))) {
		file_msg("Error spawning process '%s': %s",
				full_command(argv), strerror(ret));
		return -1;
	}

	close(pd[1]);		/* close the write end here */
	fds.fd = pd[0];		/* and poll the read end */
	fds.events = POLLIN | POLLOUT;
	fds.revents = 0;

	/* the first noticeable event from the child, even when it terminates
	   without output, is a polling event, either POLLHUP or POLLIN */
	ret = 0;
	while (!(fds.revents & POLLHUP)) {
		ret = poll(&fds, 1, -1 /* no timeout */);
		if (ret < 0) {
			file_msg("Command %s, polling error: %s",
					full_command(argv), strerror(errno));
			return -1;
		}
		/* stat > 0; stat == 0 not possible without timeout */
		if (fds.revents & POLLIN) {
			char		buf[256];
			ssize_t		n = 0;
			size_t		num = 0;

			while ((n = read(pd[0], buf + num, sizeof buf - 1)) > 0
					&& (num += n) < sizeof buf - 1)
				;
			if (num > 0) {
				buf[num] = '\0';
				file_msg("Error message from command %s: %s",
						full_command(argv), buf);
			} else if (n < 0) {
				file_msg(
					"Cannot read error message from %s: %s",
					full_command(argv), strerror(errno));
			}
		}
		if (fds.revents & POLLHUP)
			break;
		if (fds.revents & (POLLERR | POLLNVAL)) {
			file_msg("Command %s, polling error:%s",
					full_command(argv),
					fds.revents & POLLERR ? " POLLERR" : "",
					fds.revents &POLLNVAL? " POLLNVAL": "");
			break;
		}
	}

	if (close(pd[0])) {
		file_msg("Error closing stderr to command %s: %s",
				full_command(argv), strerror(errno));
	}

	if (waitpid(pid, &ret, 0) == -1) {
		file_msg("Error waiting for completion of process %s: %s",
				full_command(argv), strerror(errno));
		return -1;
	}

	if (WIFEXITED(ret)) {
		ret = WEXITSTATUS(ret);
	} else {
		file_msg("Command %s interrupted by signal %d",
				full_command(argv), ret = WTERMSIG(ret));
	}
	return ret;
}
