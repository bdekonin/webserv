/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/19 16:16:08 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/08/26 15:42:24 by bdekonin      ########   odam.nl         */
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
	{
		std::cout << "Thowing exception\n";
		throw std::logic_error("Failed to bind socket to port");
	}
		// throw std::exception("Failed to bind socket to port.");

	// Set the socket to listen for incoming connections.
	ret = listen(socketFD, 10);
	if (ret < 0)
		throw std::runtime_error("Failed to make socket listen to incoming connections.");

	return socketFD;
}


int main(int argc, char const *argv[])
{
	std::vector<ServerConfiguration> servers;
	std::map<int, std::vector<ServerConfiguration> > server_map; // <port, 
	std::map<int, std::vector<ServerConfiguration> >::iterator server_map_it; // <port,  
	Parser parser(argv[1]);

	servers = parser.init();

	// misschien std::vector<std::map<int, ServerConfiguration> servers; // <server_fd, serverConfiguration> per server namelijk meerdere ports die open gaan.'

// 	int fd = openSocket(8080, "0.0.0.0");
// 	int fd2 = openSocket(8081, "0.0.0.0");
// 	std::cout << fd << std::endl;
// 	std::cout << fd2 << std::endl;


// close(fd);
// close(fd2);

	std::vector<std::pair<std::string, size_t> > ports;
	std::cout << "server.size(): " << servers.size() << std::endl;
	for (int i = 0; i < servers.size(); i++)
	{
		ports = servers[i].get_listen();
		std::cout << "Server " << i << " is ";
		for (int j = 0; j < ports.size(); j++)
		{
			std::cout << std::setw(13) << "[" << ports[j].first << " : " << ports[j].second << "] ";
			try
			{
				// server_map[ports[j].second].push_back(servers[i]);
				server_map[openSocket(ports[j].second)].push_back(servers[i]);
			}
			catch(const std::exception& e)
			{
				std::cout << "SDKJHAKSJDA\n";
				std::cerr << e.what() << '\n';
				struct sockaddr_in		sin;
				for (server_map_it = server_map.begin(); server_map_it != server_map.end(); server_map_it++)
				{
					
					socklen_t len = sizeof(sin);
					if (getsockname(server_map_it->first, (struct sockaddr*)&sin, &len) == -1)
						throw std::runtime_error("Failed to get socket name.");
					std::cout << "Looking for port: " << ports[j].second << " and found " << ntohs(sin.sin_port) << std::endl;
					if (ntohs(sin.sin_port) == ports[j].second)
					{
						std::cout << "Adding Port " << ntohs(sin.sin_port) << " To the back of server_map." << std::endl;
						server_map[server_map_it->first].push_back(servers[i]);
					}
				}
			}
			// server_map_it = server_map.find(ports[j].second);
			// bool found = false;
			
			// for (int foundI = 0; foundI < i; foundI++)
			// {
			// 	std::vector<std::string, size_t> foundPorts = servers[foundI].get_listen();
			// 	if (std::find(foundPorts.begin(), foundPorts.end(), ports[j].second) == ports)
			// 	{
			// 		found = true;
			// 		break;
			// 	}
			// }
			// if (server_map_it != server_map.end())
			// {
			// 	std::cout << "already listening on port " << ports[j].second << std::endl;
			// 	// server_map[ports[j].second].push_back(servers[i]);
			// 	server_map_it.operator*().second.push_back(servers[i]);
			// }
			// else
			// {
			// 	std::vector <ServerConfiguration> v;
			// 	v.push_back(servers[i]);
			// 	server_map[openSocket(ports[j].second)] = v;
			// }
		}
		std::cout << std::endl << std::endl;
		ports.clear();
	}
	// print server_map;
	for (server_map_it = server_map.begin(); server_map_it != server_map.end(); server_map_it++)
	{
		std::cout << "Server listening on fd " << server_map_it->first << std::endl;
	}



	exit(1);

	
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