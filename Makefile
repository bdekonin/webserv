################################################################################
#                                     CONFIG                                   #
################################################################################

NAME        := webserv
CC         := c++
# FLAGS    := -Wall -Wextra -Werror 
FLAGS += -g -D DEBUG=1 -fsanitize=address


UNAME := $(shell uname)
ifeq ($(UNAME), Darwin)
FLAGS += -std=c++98
endif

################################################################################
#                                 PROGRAM'S SRCS                               #
################################################################################

SRCS        :=      main.cpp \
                          utils.cpp \
						  src/Configuration.cpp \
						  src/Job.cpp \
						  src/LocationConfiguration.cpp \
						  src/Parser.cpp \
						  src/Request.cpp \
						  src/Response.cpp \
						  src/Server.cpp \
						  src/ServerConfiguration.cpp \
						  src/Utils.cpp \
						  src/Webserv.cpp \
                          
OBJS        := ${SRCS:.cpp=.o}

.cpp.o:
	${CC} ${FLAGS} -c $< -o ${<:.cpp=.o}

################################################################################
#                                  Makefile  objs                              #
################################################################################


CLR_RMV		:= \033[0m
RED		    := \033[1;31m
GREEN		:= \033[1;32m
YELLOW		:= \033[1;33m
BLUE		:= \033[1;34m
CYAN 		:= \033[1;36m
RM		    := rm -f

${NAME}:	${OBJS}
			@echo "$(GREEN)Compilation ${CLR_RMV}of ${YELLOW}$(NAME) ${CLR_RMV}..."
			${CC} ${FLAGS} -o ${NAME} ${OBJS}
			@echo "$(GREEN)$(NAME) created[0m ✔️"

all:		${NAME}

test: re
		@echo "$(GREEN)Running tests...${CLR_RMV}"
		@./${NAME} config/redirecttest_config.conf

default: re
		@echo "$(GREEN)Running tests...${CLR_RMV}"
		@./${NAME} config/default_configfile.conf

autoindex: re
		@echo "$(GREEN)Running tests...${CLR_RMV}"
		@./${NAME} config/autoindextest_config.conf

bonus:		all

clean:
			@ ${RM} *.o */*.o */*/*.o
			@ echo "$(RED)Deleting $(CYAN)$(NAME) $(CLR_RMV)objs ✔️"

fclean:		clean
			@ ${RM} ${NAME}
			@ echo "$(RED)Deleting $(CYAN)$(NAME) $(CLR_RMV)binary ✔️"

re:			fclean all

.PHONY:		all clean fclean re


chunk:
	curl -X POST -H "Transfer-Encoding: chunked" -H Expect: -F 'file=@inc/Parser.hpp' localhost:8080/post/upload2.php

upload:
	curl -X POST -H Expect: -F 'file=@inc/Parser.hpp' localhost:8080/post/upload2.php
