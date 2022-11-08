/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/19 16:16:08 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/11/08 21:07:45 by bdekonin      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

# include "inc/ServerConfiguration.hpp" // Derived from Configuration
# include "inc/Webserv.hpp"
# include "inc/Parser.hpp"
# include "inc/Server.hpp"
# include "inc/Job.hpp"

int main(int argc, char const *argv[])
{
	if (argc != 2) {
		std::cerr << "Usage: ./webserv <config_file>" << std::endl;
		return EXIT_FAILURE;
	}
	(void)argc;
	std::vector<ServerConfiguration> configs;
	std::map<int, Server> servers; // port, server

	Parser *parser = new Parser(argv[1]);
	configs = parser->init();
	delete parser;

	Webserv server(configs);
	server.setupServers();

	try
	{
		server.run();
		// server.closeAll();
	}
	catch(const std::exception& e)
	{
		std::cerr << "catch: "<< e.what() << '\n';
		// server.closeAll();
	}
	return EXIT_SUCCESS;
}
