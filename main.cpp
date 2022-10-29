/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/19 16:16:08 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/10/29 19:07:05 by bdekonin      ########   odam.nl         */
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
	(void)argc;
	std::vector<ServerConfiguration> configs;
	std::map<int, Server> servers; // port, server
	Parser parser(argv[1]);

	configs = parser.init();

	Webserv server(configs);
	server.setupServers();

	server.run();
	for (std::map<int, Job>::const_iterator it = server.jobs.begin(); it != server.jobs.end(); it++)
	{
		std::cout << "Closing " << it->second.fd <<  std::endl;
		close(it->second.fd);
	}

	return 0;
}
