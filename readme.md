# ft_durex

UNIX trojan server and client working as a daemon using an advanced encryption algorithm

## Usage

### Server

```
./ft_durex
```

### Client

Default launch on host 127.0.0.1 an port 4242

```
./ft_manix [<host> <port>]
```

Commands (in-app)

```
help
	Display usage

shell
	Spawn an encrypted remote shell connection on daemon server

exit
	Terminate the remote shell connection

quit
	Stop the daemon server and terminate all client(s) connection(s)

bye
	Exit the client without stopping the daemon server
```

## Running

On startup the daemon server sets himself as a UNIX daemon.
It creates two files ; one for logging at /var/log/durex/durex.log and another one for lock at /var/lock/durex.lock
