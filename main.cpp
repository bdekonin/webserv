/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/19 16:16:08 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/08/28 20:58:56 by bdekonin      ########   odam.nl         */
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
	struct sockaddr_in		sock_struct;
	int						socketFD;

	socketFD = socket(AF_INET, SOCK_STREAM, 0);
	if (socketFD < 0)
		throw std::runtime_error("socket: failed to create socket.");
	// ret = setsockopt(socketFD, SOL_SOCKET, SO_REUSEPORT, &options, sizeof(options));
	// if (socketFD < 0)
	// 	throw std::runtime_error("error");

	bzero(&sock_struct, sizeof(sock_struct));
	sock_struct.sin_family = AF_INET;
	if (strcmp(hostname, "") == 0)
		sock_struct.sin_addr.s_addr = htonl(INADDR_ANY);
	else
		sock_struct.sin_addr.s_addr = inet_addr(hostname);
	sock_struct.sin_port = htons(port);

	if (bind(socketFD, (struct sockaddr*)&sock_struct, sizeof(sock_struct)) < 0)
		throw std::runtime_error("bind: Failed to bind.");
	if (listen(socketFD, 10) < 0)
		throw std::runtime_error("listen: failed to listen.");

	return socketFD;
}

#include "inc/Server.hpp"


bool has_port_occured(std::vector<Server> &s, int port, int *index)
{
	bool occured = false;

	for (size_t i = 0; i < s.size(); i++)
	{
		if (s[i].get_port() == port)
		{
			if (index != NULL)
				*index = i;
			occured = true;
			break;
		}
	}

	return occured;
}


					// int s = openSocket(ports[j].second);
					// std::cout << "socket: " << s << ":" << ports[j].second << std::endl;
					// char *h = (char*)ports[j].first.c_str();
					// in_port_t p = ports[j].second;
					// servers.push_back(Server(s, h, p, configs[i]));
					// std::cout << "opening socket on client " << ports[j].second << " to server " << server_i << std::endl;
					// break;

#define getString(n) #n
#define VAR(var) std::cerr << std::boolalpha << __LINE__ << ":\t" << getString(var) << " = [" <<  (var) << "]" << std::noboolalpha << std::endl;

int main(int argc, char const *argv[])
{
	(void)argc;
	std::vector<ServerConfiguration> configs;
	std::vector<Server> servers;
	Parser parser(argv[1]);

	configs = parser.init();

	std::vector<std::pair<std::string, size_t> > ports;
	int s = 0;
	char *h = NULL;
	in_port_t p = 0;
	// std::cout << "configs.size(): " << configs.size() << std::endl;
	servers.clear();
	for (size_t i = 0; i < configs.size(); i++)
	{
		ports = configs[i].get_listen();
		// std::cout << "Server " << i << " is \n";
		for (size_t j = 0; j < ports.size(); j++)
		{
			int index_of_server = 0;

			if (!has_port_occured(servers, ports[j].second, &index_of_server))
			{
				s = openSocket(ports[j].second);
				// std::cout << __LINE__ << " socket: " << s << ":" << ports[j].second << std::endl;
				VAR(s);
				h = (char*)ports[j].first.c_str();
				p = ports[j].second;
				servers.push_back(Server(s, h, p, configs[i]));
				// std::cout << "opening socket on client " << ports[j].second << ":" << s << " to server " << i << std::endl;
			}
			else
			{
			// 	servers[index_of_server].push_back(configs[i]);
				std::cout << "Port " << ports[j].second << " already in use.\n";
			}
			// std::cout << "Server " << i << " is " << openSocket(ports[j].second) << std::endl;
			// std::cout << std::setw(13) << "[" << ports[j].first << " : " << ports[j].second << "]\n";
			index_of_server = 0;
		}
		// std::cout << std::endl << std::endl;
		ports.clear();
	}
	// std::cout << "server.size(): " << servers.size() << std::endl;
	std::cout << std::endl;



	
		fd_set readfds, copy_readfds;
		
		FD_ZERO(&readfds);
		


		
		// print server_map
		for (size_t i = 0; i < servers.size(); i++)
		{
			std::cout << "port " << servers[i].get_port() << " has fd: " << servers[i].get_socket() << std::endl;
			// FD_SET(servers[i].get_socket(), &readfds); // adds fd to set
		}
	exit(1);
		copy_readfds = readfds;

		if (select(FD_SETSIZE, &copy_readfds, NULL, NULL, NULL) < 0)
			throw std::runtime_error("Failed to select.");

		for (size_t i = 0; i<FD_SETSIZE; i++)
		{
			if (FD_ISSET(i, &copy_readfds))
			{
				std::cout << "incoming traffic on: " << i << std::endl;
				for (size_t j = 0; j < servers.size(); i++)
				{
					close(servers[i].get_socket());
				}
				exit(1);
			}
		}
	return 0;
}