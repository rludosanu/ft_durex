#include "durex.h"

/*
** ***********************************************************************
**
** DESCRIPTION
**		This is a collection of utils functions for t_client structure
**		management.
**
** client_new
**		Allocate a new client node
**
** client_add
**		Add a client node to the list (at tail)
**
** client_free
**		Free a client node and closes its socket
**
** client_ofree
**		Free a client node based on its socket fd
**
** client_lfree
**		Free all of the clients
**
** ***********************************************************************
*/

t_client	*client_new(int sock)
{
	t_client	*clt;

	if (sock == -1)
		return (NULL);
	if (!(clt = (t_client *)malloc(sizeof(t_client))))
		return (NULL);
	clt->sock = sock;
	clt->log = LOGIN_PENDING;
	clt->shell = FALSE;
	clt->is_key = FALSE;
	clt->sched = NULL;
	clt->pid = -1;
	clt->next = NULL;
	return (clt);
}

int			client_add(t_client **head, t_client *node)
{
	t_client	*tmp;

	if (node == NULL)
		return (-1);
	if (*head == NULL)
		*head = node;
	else
	{
		tmp = *head;
		while (tmp->next != NULL)
			tmp = tmp->next;
		tmp->next = node;
	}
	return (0);
}

void		client_free(t_client **node)
{
	if (*node == NULL)
		return ;
	if ((*node)->pid != -1)
	{
		daemon_report(LOG_INFO, "Killing forked process manually (SIGTERM)");
		kill((*node)->pid, SIGTERM);
	}
	if (close((*node)->sock) != -1)
		daemon_report(LOG_INFO, "Client socket closed.");
	if ((*node)->sched != NULL)
	  free((*node)->sched);
	free(*node);
	*node = NULL;
}

void			client_ofree(t_client **head, int sock)
{
	t_client	*curr;
	t_client	*save;

	if ((curr = *head) == NULL)
		return ;
	if ((*head)->sock == sock)
	{
		save = (*head)->next;
		client_free(head);
		*head = save;
	}
	else
	{
		while (curr->next != NULL)
		{
			if (curr->next->sock == sock)
			{
				save = curr->next->next;
				client_free(&(curr->next));
				curr->next = save;
			}
			else
				curr = curr->next;
		}
	}
}

void		client_lfree(t_client **head)
{
	t_client	*curr;
	t_client	*next;

	if (*head == NULL)
		return ;
	curr = *head;
	while (curr != NULL)
	{
		next = curr->next;
		client_free(&curr);
		curr = next;
	}
}
