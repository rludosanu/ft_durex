# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: rludosan <marvin@42.fr>                    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2017/03/31 10:26:56 by rludosan          #+#    #+#              #
#    Updated: 2017/09/29 14:58:40 by rludosan         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# Common
CC				=	gcc
CFLAGS			=	-Wall -Wextra -Werror
INC				=	-Iincludes
DIR				=	./sources/

# Server
SERVER			=	Durex
SERVER_DUP		=	/usr/sbin/Durex
SERVER_LOGS		=	/var/log/durex/durex.log
SERVER_LOCK		=	/var/lock/durex.lock
SRC_SERVER		=	durex \
					signal \
					path \
					daemon \
					replicate \
					lock \
					report \
					server \
					client \
					shell \
					key_serv \
					key_utils_serv \
					rj \
					librj

OBJ_SERVER		=	$(addsuffix .o, $(SRC_SERVER))
TGT_SERVER		=	$(addprefix $(DIR), $(OBJ_SERVER))

# Client
CLIENT			=	Manix
SRC_CLIENT		=	manix key_cli key_utils_cli rj librj
OBJ_CLIENT		=	$(addsuffix .o, $(SRC_CLIENT))
TGT_CLIENT		=	$(addprefix $(DIR), $(OBJ_CLIENT))

# Rules
all: $(SERVER) $(CLIENT)

$(SERVER): $(TGT_SERVER)
	$(CC) $(CFLAGS) $(INC) -o $@ $^

$(CLIENT): $(TGT_CLIENT)
	$(CC) $(CFLAGS) $(INC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $(INC) -o $@ -c $<

clean:
	rm -f $(TGT_SERVER) $(TGT_CLIENT)

fclean: clean
	rm -rf $(SERVER) $(CLIENT) $(SERVER_DUP)

re: fclean all

lclean:
	rm -f $(SERVER_LOGS) $(SERVER_LOCK) $(SERVER_DUP)

# Phony
.PHONY: all clean fclean re
