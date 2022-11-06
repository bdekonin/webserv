
/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Webserv.hpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/22 22:57:55 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/09/06 19:09:18 by bdekonin      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

#ifndef DEBUG
# define DEBUG 0
#endif

# include <cstring>
# include <sys/socket.h>
# include <netinet/in.h>
#include <sys/types.h>
#include <sys/wait.h>
# include "Colors.h"
# include <iostream>

# include "Configuration.hpp" // Base Class
# include "ServerConfiguration.hpp" // Derived from Configuration
# include "LocationConfiguration.hpp" // Derived from Configuration

# include "Parser.hpp" // Class That parses a config file.
# include "Job.hpp" // Class that handles a job. (write read to users)
# include "Server.hpp" // Class that handles a server.
# include "Request.hpp" // Class that handles a request.
# include "Response.hpp" // Class that handles a response.
# include "Utils.hpp"

class Webserv
{
	public:
		/* Constructor  */
		Webserv(std::vector<ServerConfiguration> &configs);

		/* Destructor */
		virtual								~Webserv();

		/**
		 * @brief Starts the webserv
		 * 
		 */
		void								run();

		/**
		 * @brief Creates a server for each server configuration
		 * 
		 */
		void								setupServers();
		
		/**
		 * @brief This function gets called at end of program to close all sockets.
		 * 
		 */
		void								closeAll();
	private:
		std::vector<ServerConfiguration>	&configs;	// Vector of all the server configurations.
		std::map<int, Server>				servers;	// Map of all the servers that are running. | Key = port | Value = Server
		std::map<int, Job>					jobs;		// List of all jobs. it includes the Servers and Clients. | Key = socket | Value = Job
		fd_set								fds;	// List of all file descriptors that are ready to read.
		int									_max_fd; // max fd. this is used so select doesnt have to check all fds. but only til max fd

		/**
		 * @brief This function closes a seperate connection based by the job param
		 * 
		 * @param job Connection to be closed. Has to be pointer to this job.
		 * @param connectionClosedBy Who closes it? Server, Client, Webserv?
		 */
		void								closeConnection(Job *job, const char *connectionClosedBy);
		
		/**
		 * @brief Main function that reads from a connection using recv,
		 * 
		 * @param job The connection that needs to be read.
		 * @param loop_job_counter The counter of the job in the loop.
		 * @param copy_writefds The copy of the writefds.
		 * @return int 
		 */
		int									client_read(Job *job, size_t &loop_job_counter, fd_set *copy_writefds);
		
		/**
		 * @brief Main function that reads a file and stores it in job.response
		 * 
		 * @param job The connection
		 * @param copy_writefds The copy of the writefds.
		 * @param isRecursive Is this function called recursively?
		 * @param type The type of file that needs to be read.
		 * @return char 
		 */
		char								file_read(Job *job, fd_set *copy_writefds, bool isRecursive, Job::PATH_TYPE type);
		
		/**
		 * @brief Main function that writes to a connection using send,
		 * 
		 * @param job The connection that needs to be written to.
		 */
		void								client_response(Job *job);
		
		/**
		 * @brief The CGI Function that handles CGI requests.
		 * 
		 * @param job The connection that needs to be written to.
		 * @param copy_writefds The copy of the writefds.
		 */
		void								do_cgi(Job *job, fd_set *copy_writefds);
		
		int									create_correct_configfile(Request &request, ServerConfiguration &config, Configuration &ConfigToChange, std::string &ConfigToChange_path);

		/**
		 * @brief This function checks if a method is allowed and returns a value of it
		 * 
		 * @param request The request that needs to be checked.
		 * @param config The config that needs to be checked.
		 * @return size_t status code of the method. either 200 or 405.
		 */
		size_t								method_handling(Request &request, Configuration &config);

		/**
		 * @brief This function accepts a connection from a serverFD and stores it in this->jobs
		 * 
		 * @param job The Server Job
		 * @param set The set of file descriptors that are ready to read.
		 * @return int error checking
		 */
		int									accept_connection(Job *job, fd_set *set);

		/**
		 * @brief Enables all fds sets
		 * 
		 */
		void								setFdSets();

		/**
		 * @brief Opening all the sockets and adding them to a vector
		 * 
		 */
		void								openingSockets();

		/**
		 * @brief This functions opens sockets for all the servers.
		 * 
		 * @param port The port that the socket is opened on.
		 * @param hostname The hostname that the socket is opened on. (localhost) or NULL when not specified.
		 * @return int The socket that is opened.
		 */
		int									openSocket(int port, const char *hostname);

		ServerConfiguration					get_correct_server_configuration(Job *job, size_t &i);

		/**
		 * @brief Get the port from job object
		 * 
		 * @param job 
		 * @return size_t 
		 */
		size_t								get_port_from_job(Job *job);
};

#endif // WEBSERV_HPP