#include "durex.h"

extern t_daemon		g_durex;

/*
** ***********************************************************************
** NAME
**		set_lock
**
** DESCRIPTION
**		Try to flock(2) the file /var/lock/durex.lock. If the file
**		doesn't exist, it will be created. This function is called on
**		daemon startup and is mainly used to check if the daemon is not 
**		already running.
**
** RETURN VALUE
**		On success 0 is returned, -1 on error.
**
** ***********************************************************************
*/

void	set_lock(void)
{
	int		fd;
	
	/*
	** Create a lock file to avoid launching more than 1 time the daemon
	*/
	daemon_report(LOG_INFO, "Creating/Opening lock file...");
	if ((fd = open(LOCK_FILE, O_CREAT, 0600)) == -1)
	{
		daemon_report(LOG_ERROR, "Unable to create/open lock file.");
		exit(EXIT_FAILURE);
	}
	else
		daemon_report(LOG_INFO, "Lock file created/opened.");

	/*
	** Lock file exclusively to make sure this process is the only one
	** to be able to access it
	*/
	daemon_report(LOG_INFO, "Acquiring lock on file...");
	if (flock(fd, LOCK_EX | LOCK_NB) == -1)
	{
		daemon_report(LOG_ERROR, "Unable to acquire lock.");
		exit(EXIT_FAILURE);
	}
	else
		daemon_report(LOG_INFO, "Lock acquired.");

	g_durex.lock = fd;
}

/*
** ***********************************************************************
** NAME
**		unset_lock
**
** DESCRIPTION
**		Try to flock(2) and unlink(2) the file /var/lock/durex.lock.
**		This function is called on daemon shutdown.
**
** RETURN VALUE
**		On success 0 is returned, -1 on error.
**
** ***********************************************************************
*/

int		unset_lock(void)
{
	/*
	** Check if we have a valid lock file fd
	*/
	if (g_durex.lock == -1)
	{
		daemon_report(LOG_ERROR, "Unable to retreive lock file fd.");
		return (-1);
	}

	/*
	** Release file lock
	*/
	daemon_report(LOG_INFO, "Releasing lock file...");
	if (flock(g_durex.lock, LOCK_UN) == -1)
	{
		daemon_report(LOG_ERROR, "Unable to release lock.");
		return (-1);
	}
	else
		daemon_report(LOG_INFO, "Lock released.");

	/*
	** Close lock file fd
	*/
	close(g_durex.lock);

	/*
	** Remove file
	*/
	daemon_report(LOG_INFO, "Removing lock file...");
	if (unlink(LOCK_FILE) == -1)
	{
		daemon_report(LOG_ERROR, "Unable to unlink lock file.");
		return (-1);
	}
	else
		daemon_report(LOG_INFO, "Lock file removed.");

	return (0);
}
