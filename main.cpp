/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/19 16:16:08 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/11/05 11:11:13 by bdekonin      ########   odam.nl         */
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
	Parser parser(argv[1]);

	configs = parser.init();

	Webserv server(configs);
	server.setupServers();

	// try
	// {
		server.run();
		std::cout << "Closing all sockets" << std::endl;
		for (std::map<int, Job*>::const_iterator it = server.jobs.begin(); it != server.jobs.end(); it++)
		{
			std::cout << "Closing " << it->second->fd <<  std::endl;
			close(it->second->fd);
		}
	// }
	// catch(const std::exception& e)
	// {
	// 	std::cerr << e.what() << '\n';

	// 	std::cout << "Closing all sockets" << std::endl;
	// 	// for (std::map<int, Server>::iterator it = servers.begin(); it != servers.end(); it++)
	// 	// 	close(it->first);
	// 	for (std::map<int, Job*>::const_iterator it = server.jobs.begin(); it != server.jobs.end(); it++)
	// 	{
	// 		std::cout << "Closing " << it->second->fd <<  std::endl;
	// 		close(it->second->fd);
	// 	}
	// 	// free all memory
	// }
	return 0;
}
