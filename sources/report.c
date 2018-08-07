#include "durex.h"

/*
** ***********************************************************************
** NAME
**		daemon_report
**
** DESCRIPTION
**		Write daemon (server) logs / events / user inputs into daemon log
**		file /var/log/durex/durex.log .
**		If the directory /var/log/durex doesn't exist, this function
**		will try to create it.
**
** RETURN VALUE
**		On success 0 is returned, -1 on error.
**
** ***********************************************************************
*/

int		daemon_report(const char *type, const char *log)
{
	DIR			*dir;
	int			fd;
	time_t		t_time;
	char		t_buf[30] = { '\0' };
	struct tm	*t_tm;

	/*
	** If directory doesn't exist create it
	*/
	if ((dir = opendir(LOG_DIR)) == NULL)
	{
		if (mkdir(LOG_DIR, 0600) == -1)
			return (-1);
	}
	else
		closedir(dir);

	/*
	** Open/Create log file
	*/
	if ((fd = open(LOG_FILE, O_WRONLY | O_CREAT | O_APPEND, 0600)) == -1)
		return (-1);

	/*
	** Format current date/time
	*/
	time(&t_time);
	t_tm = localtime(&t_time);
	strftime(t_buf, 30, "[ %Y-%m-%d %H:%M:%S ]", t_tm);

	/*
	** Output into file
	*/
	dprintf(fd, "%s %s %s\n", t_buf, type, log);

	/*
	** Close fd and exit
	*/
	close(fd);
	return (0);
}
