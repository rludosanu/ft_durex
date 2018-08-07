#include "manix.h"
#include "rj.h"

t_client	g_manix;

/*
** ***********************************************************************
** NAME
**		strempty
**
** DESCRIPTION
**		Checks if the string is null or empty (blank spaces only).
**
** RETURN VALUE
**		On success 0 is returned, 1 otherwise.
** ***********************************************************************
*/

static int		strempty(char *s)
{
	size_t		i;
	size_t		l;

	if (s == NULL)
		return (1);
	i = 0;
	l = strlen(s);
	while (i < l)
	{
		if (s[i] != '\0' && s[i] != ' ')
			return (0);
		i++;
	}
	return (1);
}

void	signal_handler(int sig)
{
	(void)sig;
	write(1, INFOMSG_SIGINT, 24);
	disconnect_host();
	exit(EXIT_SUCCESS);
}

/*
** ***********************************************************************
** NAME
**		connection_loop
**
** DESCRIPTION
**		Exchanges messages with the server.
** ***********************************************************************
*/

void	connection_loop(void)
{
	fd_set		readfs;
	char		msg_out[4096] = { 0 };
	char		msg_in[4096] = { 0 };
	ssize_t		bytes;
	int			is_logged = FALSE;
	// int			is_shelled = FALSE;
	t_client        client;
	char		*safe;
	char		*clear;
	rijn_keysched_t *sched = NULL;
	
	while (1)
	{
		/* Clear and set fd sets for select */
		FD_ZERO(&readfs);
		FD_SET(STDIN_FILENO, &readfs);
		FD_SET(g_manix.sockfd, &readfs);

		/* Poll fd sets */
		if (select(g_manix.sockfd + 1, &readfs, NULL, NULL, NULL) == -1)
		{
			printf(ERRORMSG_SELECT);
			continue ;
		}

		/* Keyboard input */
		if (FD_ISSET(STDIN_FILENO, &readfs))
		{
			/* Get input */
			bytes = read(STDIN_FILENO, msg_out, 1023);

			/* Ctrl+D or EOF */
			if (bytes <= 0)
			  {
			    if (sched)
			      free(sched);
			    sched = NULL;
			    return ;
			  }

			msg_out[bytes] = '\0';
			if (bytes > 1 && msg_out[bytes - 1] == '\n')
			{
				msg_out[bytes - 1] = '\0';
				bytes -= 1;
			}

			/* Exit the client */
			if (strcmp(msg_out, "bye") == 0)
			  {
			    if (sched)
			      free(sched);
			    sched = NULL;
			    return ;
			  }

			/* Display the help menu */
			else if (strcmp(msg_out, "help") == 0)
			{
				printf(HELPMSG_HELP);
				printf(HELPMSG_SHELL);
				printf(HELPMSG_QUIT);
				printf(HELPMSG_BYE);
			}

			/* Send message */
			else
			{
				/* Check if input is not empty or space-blank */
				if (!strempty(msg_out) && msg_out[0] != '\n')
				{
				  /* dont crypt if is not log ok key accepted */
				  if (is_logged == TRUE && client.is_key == TRUE)
				    {
				      if (sched == NULL)
					{
					  sched = (rijn_keysched_t *)malloc(sizeof(rijn_keysched_t) * 100);
					  rijn_init(sched, client.shared_key);
					}

				      safe = rijn_build_encrypt(sched, (unsigned char *)msg_out);
				      if (send(g_manix.sockfd, safe, bytes + 16 - bytes%16, 0) <= 0)
					printf(ERRORMSG_SEND);
				      usleep(50000);
				      if (safe)
					free(safe);
				      safe = NULL;
				    }
				    else
					/* Send message to daemon */
					if (send(g_manix.sockfd, msg_out, bytes, 0) <= 0)
						printf(ERRORMSG_SEND);
				}
			}

			if (strcmp(msg_out, "quit") != 0 && is_logged == TRUE)
				write(1, PROMPT, 7);
		}
		/* Rcve msg */
		else if (FD_ISSET(g_manix.sockfd, &readfs))
		{
		  client.sockfd = g_manix.sockfd;
			bytes = recv(g_manix.sockfd, msg_in, 1025, 0);
			/* if recv success*/
			if (bytes > 0)
			{
			  /* decrypt msg if user is logged and key valid */
			  if (is_logged == TRUE && client.is_key == TRUE)
			    {
			      if (sched == NULL)
				{
				  sched = (rijn_keysched_t *)malloc(sizeof(rijn_keysched_t) * 100);
				  rijn_init(sched, client.shared_key);
				}

			      msg_in[bytes] = '\0';
			      clear = rijn_build_decrypt(sched, (unsigned char *)msg_in);
			      if (clear == NULL)
				{
				  continue ;
				}
			      bzero(msg_in, 4095);
			      write(1, clear, bytes );
			      if (clear)
				free(clear);
			      clear = NULL;
			    }
			  msg_in[bytes] = '\0';

			  /* write first prompt recv by the server*/
			  if ((is_logged == FALSE && client.is_key == FALSE))
			    write(1, msg_in, bytes);

			  /* check if the access is authorized after login*/
			  if (strcmp(msg_in, "Access granted\n") == 0)
			    {
			      is_logged = TRUE;
			      write(1, msg_in, bytes);;
			      printf("Generating exchange keys...\n");
			      init_key(&client);
			    }
			  /* if the user is loged start the key exchange. */
			  if (is_logged == TRUE && client.is_key == FALSE)
			    {
			      if (key_exchange(&client, (unsigned char *)msg_in) == -2)
				{
				  printf("Error during the exchange of keys\n");
				  if (sched)
				    free(sched);
				  sched = NULL;
				  return ;
				}
			    }

			  /*
			   * write prompt after
			   * login only if message lenght < 1025 ()
			   * promt will not be print between big messages
			   */
			  if (is_logged == TRUE && client.is_key == TRUE && bytes != 1025)
			    write(1, PROMPT, 7);
			}
			else if (bytes == 0)
			{
				printf(INFOMSG_CONNLOST);
				if (sched)
				  free(sched);
				sched = NULL;
				return ;
			}
			else
				printf(ERRORMSG_RECV);
		}
	}
}

/*
** ***********************************************************************
** NAME
**		disconnect_host
**
** DESCRIPTION
**		Closes previously opened TCP connection.
** ***********************************************************************
*/

void	disconnect_host(void)
{
	if (g_manix.sockfd != -1)
	{
		printf(INFOMSG_CLSSOCKET);
		printf(INFOMSG_EXIT);
		close(g_manix.sockfd);
	}
}

/*
** ***********************************************************************
** NAME
**		resolve_host
**
** DESCRIPTION
**		Resolves a host by its FQDN or IP.
**
** RETURN VALUE
**		On success 0 is retuned, -1 otherwise.
**
** ***********************************************************************
*/

int		resolve_host(const char *machine, struct sockaddr_in *sockin, struct addrinfo *addr)
{
	int					ret;
	struct addrinfo		*res;
	struct addrinfo		hints;

	ret = -1;
	hints.ai_flags = 0;
	hints.ai_family = AF_INET;
	hints.ai_socktype = 0;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_addrlen = 0;
	hints.ai_addr = NULL;
	hints.ai_canonname = NULL;
	hints.ai_next = NULL;
	if (getaddrinfo(machine, NULL, &hints, &res) == 0)
	{
		addr = res;
		while (addr != NULL)
		{
			if (addr->ai_family == AF_INET)
			{
				memcpy(sockin, addr->ai_addr, sizeof(struct sockaddr_in));
				ret = 0;
				break ;
			}
			addr = addr->ai_next;
		}
		freeaddrinfo(res);
	}
	return ret;
}

/*
** ***********************************************************************
** NAME
**		connect_host
**
** DESCRIPTION
**		Creates a TCP socket to connect to Durex server.
**
** RETURN VALUE
**		On successful connection 0 is retuned, -1 otherwise.
**
** ***********************************************************************
*/

int		connect_host(const char *machine, const char *port)
{
	int					sockfd;
	struct sockaddr_in	sockin;
	struct addrinfo		addr;

	printf(INFOMSG_CONNECT, machine, port);
	printf(INFOMSG_CRTSOCKET);
	sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockfd < 0)
	{
		printf(ERRORMSG_CRTSOCKET);
		return (-1);
	}
	else
		printf(SUCCESSMSG_CRTSOCKET);

	printf(INFOMSG_RSVLHOST);
	if (resolve_host(machine, &sockin, &addr) == -1)
	{
		printf(ERRORMSG_RSVLHOST);
		close(sockfd);
		return (-1);
	}
	else
		printf(SUCCESSMSG_RSVLHOST);

	printf(INFOMSG_CONNHOST);
	sockin.sin_family = AF_INET;
	sockin.sin_port = htons(atoi(port));
	if (connect(sockfd, (struct sockaddr *)&sockin, sizeof(sockin)) < 0)
	{
		printf(ERRORMSG_CONNHOST);
		close(sockfd);
		return (-1);
	}
	else
		printf(SUCCESSMSG_CONNHOST);

	g_manix.sockfd = sockfd;

	printf(INFOMSG_WELCOME);
	return (0);
}

/*
** ***********************************************************************
** NAME
**		Manix
**
** DESCRIPTION
**		Client for Durex daemon.
**		Tries to connect to the couple machine:port.
**		By default, it will try to connect to 127.0.0.1:4242.
** ***********************************************************************
*/

int		main(int argc, char **argv)
{
	signal(SIGINT, signal_handler);

	if (argc == 1)
	{
		printf(INFOMSG_DEFAULT);
		if (connect_host(DEFAULT_HOST, DEFAULT_PORT) == -1)
		{
			printf(INFOMSG_EXIT);
			return (1);
		}
	}
	else if (argc == 3)
	{
		if (connect_host(argv[1], argv[2]) == -1)
		{
			printf(INFOMSG_EXIT);
			return (1);
		}
	}
	else
	{
		printf(INFOMSG_USAGE, argv[0]);
		return (1);
	}
	connection_loop();
	disconnect_host();
	return (0);
}
