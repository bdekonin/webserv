
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

		ServerConfiguration					get_correct_server_configuration(Job *job);

		/**
		 * @brief Get the port from job object
		 * 
		 * @param job 
		 * @return size_t 
		 */
		size_t								get_port_from_job(Job *job);





















































			/**
			 * @brief 
			 * 
			 * @param job 
			 * @param i 
			 * @param fds 
			 * @return int 1 if ready to write 0 if not ready
			 */
			int requestRead(Job *job, fd_set *fds, fd_set *wr, fd_set *rd)
			{
				if (job->type != Job::READY_TO_READ)
				{
					// set error
					std::cerr << "Setting 500 in " << __FILE__ << ":" << __LINE__ << std::endl;
					// TODO 
					exit(EXIT_FAILURE);
				}

				Request::Type type;
				char buffer[4096 + 1];
				size_t bytes;
				int ret;

				bzero(buffer, 4096 + 1);
				bytes = recv(job->fd, buffer, 4096, 0);
				if (bytes <= 0)
				{
					this->closeConnection(job, "client");
					return (0);
				}

				Request &req = job->get_request();

				type = req.add_incoming_data(buffer, bytes);
				if (type == Request::MAX_ENTITY) // 413 Payload Too Large
				{
					// 413
					// Close Connection
				}

				if (type == Request::ERROR || req.is_complete() == false)
					return (1);

				ret = this->isError(job, req);

				// delete 
				// do_cgi
				// read
				// write

				if (ret == 0)
				{
					job->type = Job::READY_TO_WRITE;
					FD_SET(job->fd, wr);
					return (0);
				}


			}
			void createReadingJobs();
			void createWritingJobs();
			void createCGIJobs();
			void createDeletingJobs();







			void getCorrectConfigForJob(Job *job, std::string &newpath)
			{
				ServerConfiguration server_configuration = this->get_correct_server_configuration(job);
				this->create_correct_configfile(job->request, server_configuration, job->correct_config, newpath);
			}
			int isError(Job *job, Request &request) // change name
			{
				std::string newpath;
				bool get = request.is_method_get();
				bool post = request.is_method_post();
				bool del = request.is_method_delete();

				/* Checking if a Host header exists */
				if (request.get_header("host").empty() == true)
				{
					// There is no data inside the job->correct_config because it could not match a server with a configuration.
					job->set_xxx_response(job->correct_config, 400);
					return (0);
				}

				/* This functions finds the best configuration for the request */
				this->getCorrectConfigForJob(job, newpath);

				if (request.is_bad_request() == true)
				{
					if (request.get_header("content-length").empty() == true)
						job->set_xxx_response(job->correct_config, 411);
					else if (request._method == Request::UNSUPPORTED) // Just checks in general if its a GET POST DELETE
						job->set_xxx_response(job->correct_config, 405);
					else
						job->set_xxx_response(job->correct_config, 400);
					return (0);
				}
				/* 505 HTTP Version Not Supported */
				if (request.is_http_supported() == false)
				{
					job->set_xxx_response(job->correct_config, 505);
					return (0);
				}
				/* 414 URI Too Long */
				if (request._uri.size() > 256)
				{
					job->set_xxx_response(job->correct_config, 414);
					return (0);
				}
				/* 413 Payload Too Large */
				if (post && request._body.size() > job->correct_config.get_client_max_body_size())
				{
					job->set_xxx_response(job->correct_config, 413);
					return (0);
				}

				std::string extension = request._uri.substr(request._uri.find_last_of(".") + 1);

				if (extension == request._uri)
					extension.clear();
				if (get == true && job->correct_config.is_method_allowed(Request::Method::GET) == false)
					get = false;
				else if (post == true && job->correct_config.is_method_allowed(Request::Method::POST) == false)
					post = false;
				else if (del == true && job->correct_config.is_method_allowed(Request::Method::DELETE) == false)
					del = false;

				std::map<std::string, std::string>::iterator it;
				it = job->correct_config.get_cgi().find("." + extension);

				if (del == true)
				{
					job->type = Job::WAIT_FOR_DELETING;
				}
				else if (extension.empty() != true && it != job->correct_config.get_cgi().end())
				{
					job->type = Job::WAIT_FOR_CGIING;
				}
				else
				{
					if (get == true)
						job->type = Job::WAIT_FOR_READING;
					else if (post == true)
						job->type = Job::WAIT_FOR_WRITING;
					else
					{
						job->set_xxx_response(job->correct_config, 405);
						return (0);
					}
				}

				job->parse_request(newpath);

				if (DEBUG == 1)
				{
					// Print nice things
					std::stringstream ss;
					ss << CLRS_YEL;
					ss << "server : << [method: " << job->get_request().method_to_s() << "] ";
					ss << "[target: " << job->get_request().get_unedited_uri() << "] ";
					ss << "[location: " << newpath << "] ";
					ss << "[client fd: " << job->fd << "]";
					ss << CLRS_reset << std::endl;
					std::cerr << ss.str();
				}
				return (1);
			}

			void fileRead();
			


			void http_index_module(); // Function that calls functiosn below when nessecasry
				void http_file_module();
				void autoindex_module();
				void http_dir_module();
				void http_error_module();
};

#endif // WEBSERV_HPP