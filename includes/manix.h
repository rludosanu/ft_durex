#ifndef MANIX_H
# define MANIX_H

# include <limits.h>
# include <stdio.h>
# include <unistd.h>
# include <signal.h>
# include <string.h>
# include <stdlib.h>
# include <fcntl.h>
# include <netdb.h>
# include <sys/stat.h>
# include <sys/file.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/select.h>
# include <sys/wait.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <math.h>
# include <stdint.h>
# include <time.h>

# define DEFAULT_HOST			"127.0.0.1"
# define DEFAULT_PORT			"4242"

# define FALSE					0
# define TRUE					1

# define PROMPT					"Manix$ "

# define HELPMSG_HELP			"\"help\"\n    Display this help menu.\n"
# define HELPMSG_SHELL			"\"shell\"\n    Spawn a remote shell. Type \"exit\" to quit the shell.\n"
# define HELPMSG_QUIT			"\"quit\"\n    Stop the daemon.\n"
# define HELPMSG_BYE			"\"bye\"\n    Exit the client.\n"

# define INFOMSG_USAGE			"Usage: %s <machine> <port>\n"
# define INFOMSG_EXIT			"Exiting client...\n"
# define INFOMSG_CLSSOCKET		"Closing client socket...\n"
# define INFOMSG_DEFAULT		"Using default connection settings.\n"
# define INFOMSG_CONNECT		"Connecting client to %s:%s...\n"
# define INFOMSG_CRTSOCKET		"Creating client socket...\n"
# define INFOMSG_RSVLHOST		"Resolving host...\n"
# define INFOMSG_CONNHOST		"Connecting to host...\n"
# define INFOMSG_WELCOME		"Welcome ! Type \"help\" to see available commands.\n"
# define INFOMSG_SIGINT			"\nCtrl+C signal caught !\n"
# define INFOMSG_CONNLOST		"Daemon socket connection lost...\n"

# define SUCCESSMSG_CRTSOCKET	"Socket created.\n"
# define SUCCESSMSG_RSVLHOST	"Host resolved.\n"
# define SUCCESSMSG_CONNHOST	"Connected to host.\n"

# define ERRORMSG_CRTSOCKET		"Error: Failed to create TCP socket.\n"
# define ERRORMSG_RSVLHOST		"Error: Failed to resolve host.\n"
# define ERRORMSG_CONNHOST		"Error: Failed to connect to host.\n"
# define ERRORMSG_SELECT		"Error: Select failed.\n"
# define ERRORMSG_SEND			"Error: Unable to send message.\n"
# define ERRORMSG_RECV			"Error: Unable to read message.\n"

typedef struct			s_client
{
	int					sockfd;
  	unsigned long long int                  p;
	unsigned long long int                  g;
	unsigned long long int                  b;
	unsigned long long int                  x;
	unsigned long long int                  y;
	unsigned long long int                  shared_key;
	int                                     is_key;
}						t_client;

int						connect_host(const char *machine, const char *port);
void					disconnect_host(void);
int						resolve_host(const char *machine, struct sockaddr_in *sockin, struct addrinfo *addr);
void					connection_loop(void);


/*
 * keys
 */

uint64_t        powmodp(uint64_t a, uint64_t b, uint64_t P);
uint64_t        rand_uint64(void);
int             is_prime(int num);

void            init_key(t_client *c);
int             key_exchange(t_client *c, unsigned char *buf);

uint64_t        ft_atollu(const char *str);
char            *ft_itoa_a(uint64_t value, int base);
#endif
