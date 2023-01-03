/*
 * FIG : Facility for Interactive Generation of figures
 * Copyright (c) 1985-1988 by Supoj Sutanthavibul
 * Parts Copyright (c) 1989-2015 by Brian V. Smith
 * Parts Copyright (c) 1991 by Paul King
 * Parts Copyright (c) 2016-2023 by Thomas Loimer
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
#include <poll.h>
#include <unistd.h>		/* close() */
#include <signal.h>		/* SIGPIPE */
#include <stdio.h>
#include <stdlib.h>		/* malloc() */
#include <string.h>		/* strerror() */
#include <sys/stat.h>		/* mode_t, in the open() call */
#include <sys/wait.h>
#ifdef HAVE_POSIX_SPAWNP
#include <spawn.h>
#endif

#include "w_msgpanel.h"

/*
 * The first element is statically allocated.
 * All other elements, if they ever appear, must be allocated on the heap.
 */
static struct process_info {
	int			fd;
	int			fderr;
	pid_t			pid;
	struct process_info	*next;
} process_info = { -1, 0, -1, NULL };

static struct process_info *const first = &process_info;


/*
 * Fill the first or add an element to the linked list of process information.
 * The elements are identified based on the file descriptor fd.
 */
static void
add_info(int fd, int fderr, pid_t pid)
{
	if (first->fd == -1) {
		first->fd = fd;
		first->fderr = fderr;
		first->pid = pid;
	} else {
		struct process_info	*info = first;
		while (info->next)
			info = info->next;
		if (!(info->next = malloc(sizeof(struct process_info))))
			return;
		info = info->next;
		info->fd = fd;
		info->fderr = fderr;
		info->pid = pid;
		info->next = NULL;
	}
}

/*
 * Input: fd
 * Outputs: fderr, pid
 */
static int
retrieve_info(int fd, int *fderr, pid_t *pid)
{
	if (first->fd == fd) {
		*fderr = first->fderr;
		*pid = first->pid;
		if (first->next) {
			struct process_info	*info = first->next;
			first->fd = info->fd;
			first->fderr = info->fderr;
			first->pid = info->pid;
			first->next = info->next;
			free(info);
		} else {
			first->fd = -1;
		}
		return 0;
	} else {
		struct process_info	*info = first->next;
		struct process_info	*prev = first;
		if (!info)
			return -1;
		while (info->fd != fd && info->next) {
			prev = info;
			info = info->next;
		}
		if (info->fd == fd) {
			*fderr = info->fderr;
			*pid = info->pid;
			prev->next = info->next;
			free(info);
			return 0;
		}
		return -1;
	}
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
 * Spawn the process argv[0], with the NULL-terminated argument list argv.
 * If any of the fd[] is not -1, re-direct stdin, stdout or stderr to the file
 * descriptor given in fd[0], fd[1] or fd[2], respectively.
 * The file descriptors given in cfd are closed in the spawned process.
 * At least one of all file descriptors in fd[] and cfd[] should be valid.
 * Return 0 on success, an errno on failure.
 */
static int
spawn_process(pid_t *restrict pid, char *const argv[restrict], int fd[3],
			int cfd[2])
{
#ifdef HAVE_POSIX_SPAWNP
	int				i;
	posix_spawn_file_actions_t	file_actions;

	posix_spawn_file_actions_init(&file_actions);
	if (cfd[0] > -1)
		posix_spawn_file_actions_addclose(&file_actions, cfd[0]);
	if (cfd[1] > -1)
		posix_spawn_file_actions_addclose(&file_actions, cfd[1]);
	for (i = 0; i < 3; ++i) {
		if (fd[i] > -1) {
			/* 0 must be stdin, 1 stdout, and 2 stderr */
			posix_spawn_file_actions_adddup2(
					&file_actions, fd[i], i);
			posix_spawn_file_actions_addclose(
					&file_actions, fd[i]);
		}
	}

	return posix_spawnp(pid, argv[0], &file_actions, NULL, argv, NULL);

#else	/* HAVE_POSIX_SPAWNP undefined: fork() and exec() */
	*pid = fork();
	if (*pid == -1)
		return errno;
	if (*pid == 0) {
		/* the child */
		int	i;
		if (cfd[0] > -1)
			close(cfd[0]);
		if (cfd[1] > -1)
			close(cfd[1]);
		for (i = 0; i < 3; ++i) {
			if (fd[i] > -1) {
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
 * Spawn the process argv[0] with the NULL-terminated arguments argv.
 * Search PATH for the command given in argv[0].
 * If either of the fd[i] is non-negative, redirect stdin (fd[0]) and/or stdout
 * (fd[1]) in the spawned process to the open file descriptor fd[i].
 * Close the file descriptor cd in the spawned process.
 * Standard error is captured in a buffer and reported to the user.
 * Return the process id in pid and a file descriptor in fderr from which
 * to read stderr of the spawned process.
 * Return 0 on success.
 * On error, return -1, and output an error message.
 */
static int
open_process(char *const argv[restrict], int fd[2], int cd,
		pid_t *pid, int *fderr)
{
	int		i;
	int		pd[2];
	int		ffd[3] = {-1, -1, -1};
	int		cfd[2] = {cd, -1};

	if (pipe(pd)) {
		file_msg("Spawning %s, cannot create pipe: %s",
				full_command(argv), strerror(errno));
		return -1;
	}

	/* for example, if fd[1] = fdout, redirect stdout to fdout */
	for (i = 0; i < 2; ++i) {
		if (fd[i] > -1)
			ffd[i] = fd[i];
	}
	ffd[2] = pd[1];
	cfd[1] = pd[0];
	/* and return the read end of the pipe for polling */
	*fderr = pd[0];
	if ((i = spawn_process(pid, argv, ffd, cfd))) {
		file_msg("Error spawning process %s: %s",
				full_command(argv), strerror(i));
		return -1;
	}

	/* close the write end of the pipe to stderr */
	close(pd[1]);
	/* and the file descriptors used by the spawned process */
	for (i = 0; i < 2; ++i)
		if (fd[i] > -1)
			close(fd[i]);
	return 0;
}

static int
closefderr_wait(pid_t pid, int fderr, int ignore_signal)
{
	int	ret;

	if (close(fderr))
		file_msg("Error closing stderr: %s", strerror(errno));

	if (waitpid(pid, &ret, 0) == -1) {
		file_msg("Error waiting for spawned process: %s",
				strerror(errno));
		return -1;
	}

	if (WIFEXITED(ret)) {
		ret = WEXITSTATUS(ret);
	} else {
		ret = WTERMSIG(ret);
		if (ignore_signal && ret == ignore_signal)
			ret = 0;
		else
			file_msg("Spawned process interrupted by signal %d",
					ret);
	}
	return ret;
}

/*
 * Poll fderr and output the first 255 bytes received on fderr.
 * Continue reading until fderr is empty.
 * If a process did not write to stderr, anyhow a POLLHUP is detected when the
 * process finishes.
 */
static void
poll_fderr(int fderr)
{
	struct pollfd	fds;

	fds.fd = fderr;
	fds.events = POLLIN;		/* POLLHUP is anyhow reported */
	fds.revents = 0;

	if (poll(&fds, 1, -1 /* no timeout */) < 0) {
		file_msg("Error polling stderr: %s", strerror(errno));
		return;
	}
	/* poll > 0; 0 only possible if a timeout is given */
	if (fds.revents & POLLIN) {
		/* read the first 255 bytes, not more */
		char		buf[256];
		ssize_t		n = 0;
		size_t		num = 0;

		while ((n = read(fderr, buf + num, sizeof buf - 1 - num)) > 0 &&
				(num += n) < sizeof buf - 1)
			;
		if (num > 0) {
			buf[num] = '\0';
			file_msg("Error message from spawned process: %s", buf);
		}
		if (n == -1) {
			file_msg("Error reading error message: %s",
					strerror(errno));
		} else if (n == 0 && num == sizeof buf - 1) {
			/* more data is available; Read the rest,
			   to not cause Broken pipe (SIGPIPE). */
			while (read(fderr, buf, sizeof buf) > 0)
				;
		}
	}
	if (fds.revents & (POLLERR | POLLNVAL)) {
		file_msg("Error polling stderr:%s%s%s",
				fds.revents & POLLERR ? " POLLERR" : "",
				fds.revents & POLLERR && fds.revents & POLLNVAL?
					"," : "" ,
				fds.revents & POLLNVAL ?
					" file descriptor not open" : "");
	}
}


/*
 * A spawned process might either
 * - have completed writing to the output file and terminated, or waiting to
 *   write an error, or
 * - be still running, writing output and probably waiting to write error, or
 * - was terminated by us, the terminating signal and errors should be ignored.
 * In the first case, the output file descriptor might have been closed by the
 * process, or it is still open.
 */

/*
 * Spawn the process argv[0] with the NULL-terminated arguments argv.
 * Search PATH for the command given in argv[0].
 * Write the output of the spawned process to the open file descriptor fdout,
 * which shall be closed by the process.
 * Standard error is captured in a buffer and reported to the user.
 * Return the exit status of the spawned process, and output error messages.
 */
int
spawn_writefd(char *const argv[restrict], int fdout)
{
	int	fderr;
	int	fd[2] = {-1, fdout};
	pid_t	pid;

	if (open_process(argv, fd, -1, &pid, &fderr))
		return -1;
	poll_fderr(fderr);
	return closefderr_wait(pid, fderr, 0);
}

/*
 * Spawn a process and open a pipe, either for reading ("r") or for writing.
 * Return a file desrciptor for reading the output of the process, or for
 * writing to stdin of the process. If fd is non-negative, the process itself
 * will read from fd ("r") or write to it.
 */
int
spawn_popen_fd(char *const argv[restrict], const char *restrict type, int fd)
{
	int	fderr;
	int	parent;
	int	child;
	int	pd[2];
	pid_t	pid;

	if (pipe(pd)) {
		file_msg("Command %s, cannot create pipe: %s",
				full_command(argv), strerror(errno));
		return -1;
	}
	if (!strcmp(type, "r")) {
		parent = pd[0];
		pd[0] = fd;
		child = 1;
	} else {
		parent = pd[1];
		pd[1] = fd;
		child = 0;
	}

	if (open_process(argv, pd, parent, &pid, &fderr)) {
		close(parent);
		parent = -1;
	} else {
		add_info(parent, fderr, pid);
	}
	close(pd[child]);

	return parent;
}

/*
 * Spawn a process and open a pipe, either for reading ("r") or for writing.
 * Return a file desrciptor for reading the output of the process, or for
 * writing to stdin of the process.
 * It is expected that the process needs to consume some data, or writes data
 * ("r"), and terminates after a certain amount of data has been fed or been
 * retrieved from it.
 * Example:
 *   fd = spawn_popen("date", "r");	char buf[256];
 *   if (read(fd, buf, sizeof buf) < sizeof buf)
 *		spawn_pclose(fd);
 */
int
spawn_popen(char *const argv[restrict], const char *restrict type)
{
	return spawn_popen_fd(argv, type, -1);
}

/*
 * Terminate the process that reads or writes to the other end of fd,
 * and close fd.
 */
int
spawn_pclose(int fd)
{
	int		fderr;
	pid_t		pid;

	if (retrieve_info(fd, &fderr, &pid)) {
		file_msg("Error retrieving process id of spawned process!");
		file_msg("Can you reproduce this error?");
		return -1;
	}

	/*
	 * It was tried to poll fdread and check, whether the process still
	 * wants to write data. However, for bunzip2 the pipe remained empty,
	 * but bunzip2 issued an I/O error message (trying to be smart?)
	 * Therefore, unconditionally send SIGHUP. If the process exited before,
	 * the signal is ignored. SIGUSR1 was also tried, but unxz exited
	 * with 1 and did not report terminatin by a signal.
	 */
	kill(pid, SIGHUP);
	close(fd);
	return closefderr_wait(pid, fderr, SIGHUP /* ignore this signal */);
}
