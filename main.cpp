/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/19 16:16:08 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/08/25 10:34:53 by bdekonin      ########   odam.nl         */
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

# include <iostream>
# include <iomanip>
# include <sstream>
# include <fstream>
# include <string>
# include <limits>
# include <cstdio>
# include <vector>

#include <arpa/inet.h>

int			openSocket(int port, const char *hostname = "")
{
	struct sockaddr_in		serverAddress;
	int						ret;
	int						socketFD;

	socketFD = socket(AF_INET, SOCK_STREAM, 0);
	if (socketFD < 0)
		throw std::runtime_error("Failed to create socket.");
	int options = 1;
	ret = setsockopt(socketFD, SOL_SOCKET, SO_REUSEPORT, &options, sizeof(options));
	if (socketFD < 0)
		throw std::runtime_error("Failed to set socket options.");

	// Initialize the address struct that bind will use.
	bzero(&serverAddress, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	if (hostname == "")
		serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	else
		serverAddress.sin_addr.s_addr = inet_addr(hostname);
	// serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(port);

	// Bind the socket to a port.
	ret = bind(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
	if (ret < 0)
		throw std::runtime_error("Failed to bind socket to port.");

	// Set the socket to listen for incoming connections.
	ret = listen(socketFD, 10);
	if (ret < 0)
		throw std::runtime_error("Failed to make socket listen to incoming connections.");

	return socketFD;
}


int main(int argc, char const *argv[])
{
	std::vector<ServerConfiguration> servers;
	std::map<int, ServerConfiguration> server_map; // <port, 
	std::map<int, ServerConfiguration>::iterator server_map_it; // <port,  
	Parser parser(argv[1]);

	servers = parser.init();

	// misschien std::vector<std::map<int, ServerConfiguration> servers; // <server_fd, serverConfiguration> per server namelijk meerdere ports die open gaan.

	std::vector<std::pair<std::string, size_t> > ports;
	std::cout << "server.size(): " << servers.size() << std::endl;
	for (int i = 0; i < servers.size(); i++)
	{
		ports = servers[i].get_listen();
		std::cout << "Server " << i << " is ";
		for (int j = 0; j < ports.size(); j++)
		{
			if (server_map.find(ports[j].second) != server_map.end())
				std::cout << "already listening on port " << ports[j].second << std::endl;
			else
				server_map[openSocket(ports[j].second)] = servers[i];
			std::cout << std::setw(13) << "[" << ports[j].first << ":" << ports[j].second << "] ";
		}
		std::cout << std::endl << std::endl;
		ports.clear();
	}

	
		fd_set readfds, copy_readfds;
		
		FD_ZERO(&readfds);
		


		
		// print server_map
		int i = 0;
		for (server_map_it = server_map.begin(); server_map_it != server_map.end(); server_map_it++, i++)
		{
			std::cout << "Server " << std::distance(server_map.begin(), server_map_it) << " has fd: " << server_map_it->first << std::endl;
			FD_SET(server_map_it->first, &readfds); // adds fd to set
		}
		copy_readfds = readfds;

		if (select(FD_SETSIZE, &copy_readfds, NULL, NULL, NULL) < 0)
			throw std::runtime_error("Failed to select.");

		for (int i = 0; i<FD_SETSIZE; i++)
		{
			if (FD_ISSET(i, &copy_readfds))
			{
				std::cout << "incoming traffic on: " << i << std::endl;
				for (server_map_it = server_map.begin(); server_map_it != server_map.end(); server_map_it++)
				{
					close(server_map_it->first);
				}
				exit(1);
			}
		}
	return 0;
}