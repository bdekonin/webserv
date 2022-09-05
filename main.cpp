/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/19 16:16:08 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/09/05 14:44:12 by bdekonin      ########   odam.nl         */
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

#include "inc/Server.hpp"
#include "inc/Job.hpp"
#include "inc/Request.hpp"

#define getString(n) #n
#define VAR(var) std::cerr << std::boolalpha << __LINE__ << ":\t" << getString(var) << " = [" <<  (var) << "]" << std::noboolalpha << std::endl;
#define PRINT(var) std::cout << var << std::endl

char* replace_char(char* str, char find, char replace){
    char *current_pos = strchr(str,find);
    while (current_pos) {
        *current_pos = replace;
        current_pos = strchr(current_pos,find);
    }
    return str;
}

int			openSocket(int port, const char *hostname = "")
{
	struct sockaddr_in		sock_struct;
	int						socketFD;

	socketFD = socket(AF_INET, SOCK_STREAM, 0);
	if (socketFD < 0)
		throw std::runtime_error("socket: failed to create socket.");
	// ret = setsockopt(socketFD, SOL_SOCKET, SO_REUSEPORT, &options, sizeof(options));SO_REUSEADDR
	int options = 1;
	setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &options, sizeof(options));
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

void add_job_to_queue(std::vector<Job*> &queue, Job *job)
{
	queue.push_back(job);
}

void create_job_and_add_to_queue(std::vector<Job*> &queue, int type, int fd, Server *server, void *client)
{
	Job *job = new Job(type, fd, server, client);
	add_job_to_queue(queue, job);
}

int accept_connection(Job *job, std::map<int, Job> &jobs, fd_set *set)
{
	std::cout << job->fd << " Accepting Connection" << std::endl;
	struct sockaddr_in client_address;
	int address_size = sizeof(struct sockaddr_in);
	VAR(job->fd);
	int client_fd = accept(job->fd, (struct sockaddr*)&client_address, (socklen_t*)&address_size);
	if (client_fd < 0)
		throw std::runtime_error("accept: failed to accept.");
	fcntl(client_fd, F_SETFL, O_NONBLOCK);
	jobs[client_fd] = Job(CLIENT_READ, client_fd, job->server, NULL);
	
	FD_SET(client_fd, set);
	return (client_fd);
}

int main(int argc, char const *argv[])
{
	(void)argc;
	std::vector<ServerConfiguration> configs;
	std::map<int, Server> servers; // port, server
	Parser parser(argv[1]);

	configs = parser.init();

	std::cout << configs.size() << std::endl;

	std::vector<std::pair<std::string, size_t> > ports;
	int s = 0;
	char *h = NULL;
	in_port_t p = 0;
	for (size_t i = 0; i < configs.size(); i++)
	{
		ports = configs[i].get_listen();
		for (size_t j = 0; j < ports.size(); j++)
		{
			std::map<int, Server>::iterator it = servers.find(ports[j].second);
			if (it == servers.end())
			{
				std::cout << "Creating server on port " << ports[j].second << std::endl;
				s = openSocket(ports[j].second);
				h = (char*)ports[j].first.c_str();
				p = ports[j].second;
				servers[ports[j].second] = Server(s, h, p, configs[i]);
			}
			else
			{
				it->second.push_back(configs[i]);
			}
		}
		ports.clear();
	}

	std::map<int, Job> jobs;
	Job *job;
	fd_set read_fds, write_fds;
	
	// ZERO THE FD SET
	FD_ZERO(&read_fds);
	FD_ZERO(&write_fds);

	for (auto it = servers.begin(); it != servers.end(); it++)
	{
		std::cout << "server " << it->second.get_port() << " has " << it->second.get_configurations().size() << " configurations" << std::endl;
		int fd = it->second.get_socket();
		jobs[fd] = Job(WAIT_FOR_CONNECTION, fd, &it->second, NULL);
		FD_SET(fd, &read_fds);
	}

	while (true)
	{
		fd_set copy_readfds = read_fds;
		fd_set copy_writefds = write_fds;
		PRINT("Waiting on Select");
		if (select(FD_SETSIZE, &copy_readfds, &copy_writefds, NULL, NULL) < 0)
			throw std::runtime_error("select: failed to select.");
		PRINT("Done with Select");
		for (int loop_job_counter = 0; loop_job_counter < jobs.size(); loop_job_counter++)
		{
			if (FD_ISSET(loop_job_counter, &copy_readfds))
			{
				job = &jobs[loop_job_counter];
				if (job->type == WAIT_FOR_CONNECTION)
					accept_connection(job, jobs, &read_fds);
				else if (job->type == CLIENT_READ)
				{
						std::cout << job->fd << " Reading request Client\n";
						char	buffer[4096 + 1];
						int		bytesRead;
						bzero(buffer, 4096 + 1);
						bytesRead = recv(job->fd, buffer, 4096, 0);
						if (bytesRead <= 0)
						{
							close(job->fd);
							std::cout << "Client " << job->fd << " disconnected." << std::endl;
							FD_CLR(job->fd, &read_fds);
							FD_CLR(job->fd, &write_fds);
							jobs.erase(job->fd);
							continue;
						}

						std::string request(buffer);

						job->request = new Request(request); // TODO check how to free correctly

						job->type = CLIENT_RESPONSE;
						FD_SET(loop_job_counter, &copy_writefds);
				}
			}
		}
		for (int loop_job_counter = 0; loop_job_counter < jobs.size(); loop_job_counter++)
		{
			job = &jobs[loop_job_counter];
			if (FD_ISSET(loop_job_counter, &copy_writefds))
			{
				if (job->type == CLIENT_RESPONSE)
				{
					std::cout << job->fd << " Responding to Client\n";
					std::cout << job->fd << " CLIENT_RESPONSE" << std::endl;
					std::string string = "HTTP/1.1 200 OK\nContent-Type:text/html\nContent-Length: ";

					std::vector<ServerConfiguration> configs_of_job = job->server->get_configurations();

					std::string host_header = job->request->_headers_map["host"];
					int pos = host_header.find(":");

					// Find Correct Server to be used
					std::vector<std::string> server_names;
					ServerConfiguration *config_by_request;
					for (int k = 0; k < configs_of_job.size(); k++)
					{
						server_names = configs_of_job[k].get_server_names();
						for (int j = 0; j < server_names.size(); j++)
						{
							if (host_header.substr(0, pos) == server_names[j])
								config_by_request = &configs_of_job[k];
						}
					}

					// Create good response

					
					std::string content = "\n\n";

					std::ifstream file("/Users/bdekonin/Documents/webserv/parallel_commands");
					std::string tempString;
					while (std::getline(file, tempString))
						content.append(tempString);
					file.close();
					
					// content.append("Hallo Bobbie\n");
					string.append(std::to_string(content.size()));
					string.append(content);

					char *response = strdup(string.c_str());
					ssize_t bytes = send(job->fd, response,  strlen(response) + 1, 0);
					jobs[job->fd].type = CLIENT_READ;
					free(response);
				}
			}
		}
		std::cout << std::endl;
	}

	std::cout << std::endl;

	for (auto it = jobs.begin(); it != jobs.end(); it++)
	{
		std::cout << "Closing " << it->second.fd <<  std::endl;
		close(it->second.fd);
	}

	return 0;
}
