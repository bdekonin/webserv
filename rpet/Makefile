NAME =			webserv

CC =			g++
# CFLAGS =		-Wall -Werror -Wextra -std=c++98 -pedantic
UNUSED = 		-Wno-unused-variable -Wno-unused-parameter -Wno-unused-function -Wno-unused-private-field

SDIR =			src
ODIR =			obj
IDIR =			inc

CLASSES =		Server Parser Config ServerConfig LocationConfig StringUtils WebServer Task Client Request Response CGI

_OBJS =			main $(CLASSES)
OBJS =			$(addsuffix .o, $(addprefix $(ODIR)/, $(_OBJS)))
HEADERS =		$(addsuffix .hpp, $(addprefix $(IDIR)/, $(CLASSES)))

ifeq ($(DEBUG),1)
	CFLAGS += -fsanitize=address -g3
endif

.PHONY: all clean fclean re debug test

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

$(ODIR)/%.o: $(SDIR)/%.cpp $(HEADERS)
	@mkdir -p $(ODIR)
	$(CC) $(CFLAGS) -I$(IDIR) -c $< -o $@

clean:
	$(RM) $(OBJS)
	$(RM) *.dSYM

fclean: clean
	$(RM) $(NAME)
	$(RM) -rf .vscode

re: fclean all

debug:
	@make DEBUG=1

test: all
	clear
	./$(NAME)

chunk:
	curl -X POST -H "Transfer-Encoding: chunked" -H Expect: -F 'file=@inc/Parser.hpp' localhost:8000/post/upload2.php

upload:
	curl -X POST -H Expect: -F 'file=@inc/Parser.hpp' localhost:8000/post/upload2.php
