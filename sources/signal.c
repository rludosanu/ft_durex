#include "durex.h"

extern t_daemon	g_durex;

/*
** ***********************************************************************
** NAME
**		signal_handler
**
** DESCRIPTION
**		Logs the intercepted signal. If the process is the "real" Durex
**		its stops the server and removes lock file.
** ***********************************************************************
*/

void			signal_handler(int sig)
{
	switch (sig)
	{
		case SIGHUP:
			daemon_report(LOG_SIGNAL, "SIGHUP signal caught.");
			break ;
		case SIGINT:
			daemon_report(LOG_SIGNAL, "SIGINT signal caught.");
			break ;
		case SIGQUIT:
			daemon_report(LOG_SIGNAL, "SIGQUIT signal caught.");
			break ;
		case SIGILL:
			daemon_report(LOG_SIGNAL, "SIGILL signal caught.");
			break ;
		case SIGABRT:
			daemon_report(LOG_SIGNAL, "SIGABRT signal caught.");
			break ;
		case SIGFPE:
			daemon_report(LOG_SIGNAL, "SIGFPE signal caught.");
			break ;
		case SIGSEGV:
			daemon_report(LOG_SIGNAL, "SIGSEGV signal caught.");
			break ;
		case SIGPIPE:
			daemon_report(LOG_SIGNAL, "SIGPIPE signal caught.");
			break ;
		case SIGALRM:
			daemon_report(LOG_SIGNAL, "SIGALRM signal caught.");
			break ;
		case SIGUSR1:
			daemon_report(LOG_SIGNAL, "SIGUSR1 signal caught.");
			break ;
		case SIGUSR2:
			daemon_report(LOG_SIGNAL, "SIGUSR2 signal caught.");
			break ;
		case SIGCONT:
			daemon_report(LOG_SIGNAL, "SIGCONT signal caught.");
			break ;
		case SIGTERM:
			daemon_report(LOG_SIGNAL, "SIGTERM signal caught.");
			break ;
		default:
			daemon_report(LOG_SIGNAL, "SIGUNKNOWN signal caught.");
			break ;
	}
	if (getpid() == g_durex.pid)
	{
		server_destroy();
		unset_lock();
	}
	exit(EXIT_SUCCESS);
}

/*
** ***********************************************************************
** NAME
**		signal_setup
**
** DESCRIPTION
**		Sets up the signal handlers.
** ***********************************************************************
*/

void		signal_setup(void)
{
	/*
	 ** Ignore TTY signals
	 */
	signal(SIGTSTP, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);

	/*
	 ** Catchable signals
	 */
	signal(SIGHUP, signal_handler);
	signal(SIGINT, signal_handler);
	signal(SIGQUIT, signal_handler);
	signal(SIGILL, signal_handler);
	signal(SIGABRT, signal_handler);
	signal(SIGFPE, signal_handler);
	signal(SIGSEGV, signal_handler);
	signal(SIGPIPE, signal_handler);
	signal(SIGALRM, signal_handler);
	signal(SIGUSR1, signal_handler);
	signal(SIGUSR2, signal_handler);
	signal(SIGCONT, signal_handler);
	signal(SIGTERM, signal_handler);
}
