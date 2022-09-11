/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/19 16:16:08 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/09/10 11:48:26 by bdekonin      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>

# include "inc/Configuration.hpp" // Base Class
# include "inc/ServerConfiguration.hpp" // Derived from Configuration
# include "inc/LocationConfiguration.hpp" // Derived from Configuration

#include <string.h>

# include "inc/Parser.hpp"

#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h>

# include <iostream>
# include <iomanip>
# include <sstream>
# include <fstream>
# include <string>
# include <limits>
# include <cstdio>
# include <vector>

#include <arpa/inet.h>
#include "inc/Webserv.hpp"

#include "inc/Server.hpp"
#include "inc/Job.hpp"
#include "inc/Request.hpp"
#include "inc/Response.hpp"

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
	for (auto it = server.jobs.begin(); it != server.jobs.end(); it++)
	{
		std::cout << "Closing " << it->second.fd <<  std::endl;
		close(it->second.fd);
	}

	return 0;
}
