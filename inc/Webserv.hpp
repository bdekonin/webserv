
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
		typedef std::map<int, Job>::iterator	iterator;

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


		void						closeConnection(iterator &it, const char *connectionClosedBy);
		
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
		void								betterClientResponse(Job *job)
		{
			bool connection_close = false;
			int bytes;
			job->_getResponse().build_response_text();
			std::vector<unsigned char> &response = job->_getResponse().get_response();
			size_t response_size = response.size();
			char *response_char = reinterpret_cast<char*> (&response[0]);

			bytes = send(job->fd, response_char + job->bytes_sent, response_size - job->bytes_sent, SO_NOSIGPIPE);
			if (bytes == -1)
			{
				job->type = Job::CLIENT_REMOVE;
				return ; 
			}
			job->bytes_sent += bytes;
			if (job->bytes_sent >= response_size)
			{
				if (DEBUG == 1)
				{
					std::stringstream ss;
					ss << CLRS_BLU;
					bytes = strstr(response_char, "\r\n") - response_char;
					ss << "server : >> [status: " << std::string(response_char, bytes)  << "] ";
					ss << "[length: " << response_size - job->bytes_sent << "] ";
					ss << "[client: " << job->fd << "] ";
					ss << CLRS_reset;
					std::cerr << ss.str() << std::endl;
				}
				if (job->get_request().get_header("connection").compare("Close") == 0)
					connection_close = true;

				this->reset(job);



				if (connection_close)
					job->type = Job::CLIENT_REMOVE;
			}
		}
		
			void reset(Job *job) {
				job->clear();
				this->jobs[job->fd].clear();
				this->jobs[job->fd].type = Job::READY_TO_READ;
			}

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

		ServerConfiguration					&get_correct_server_configuration(Job *job);

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
			int requestRead(iterator &it, fd_set *fds, fd_set *wr, fd_set *rd)
			{
				Job *job;

				job = &it->second;
				if (job->type != Job::READY_TO_READ)
				{
					// set error
					std::cerr << "Setting 500 in " << __FILE__ << ":" << __LINE__ << std::endl;
					// TODO 
					exit(EXIT_FAILURE);
				}

				Request::Type type;
				char buffer[4096 + 1];
				int bytes;
				int ret;

				bzero(buffer, 4096 + 1);
				bytes = recv(job->fd, buffer, 4096, 0);
				// /* Blocking */
				if (bytes < 0)
				{
					// this->closeConnection(it, "client");
					job->type = Job::CLIENT_REMOVE;
					return (0);
				}
				/* Keep reading */
				if (bytes == 0)
				{
					// this->closeConnection(it, "client");
					job->type = Job::CLIENT_REMOVE;
					return (0);
				}

				Request &req = job->get_request();

				type = req.add_incoming_data(buffer, bytes);
				if (type == Request::MAX_ENTITY) // 413 Payload Too Large
				{
					// 413
					// Close Connection
					job->set_xxx_response(job->correct_config, 413);
					job->get_response().set_header("Connection: close");
					job->type = Job::READY_TO_WRITE;
					FD_SET(job->fd, wr);
					return (0);
				}


				if (job->get_request()._type != Request::ERROR && job->request.is_complete() == false)
					return (1); 

				// if (type == Request::ERROR || req.is_complete() == false)
				// 	return (1);

				ret = this->isError(job, req);
				if (ret == 0)
				{
					job->type = Job::READY_TO_WRITE;
					FD_SET(job->fd, wr);
					return (0);
				}
				ret = this->setupNewJobs(job, (Job::JOB_TYPE)job->type);
				if (ret == 0) // means somethinig to be written as error or autoindex?
				{
					job->type = Job::READY_TO_WRITE;
					FD_SET(job->fd, wr);
					return (0);
				}
				if (ret > 0)
				{
					if (job->type == Job::WAIT_FOR_READING)
					{
						FD_SET(ret, rd);
						FD_SET(ret, fds);
					}
					else if (job->type == Job::WAIT_FOR_WRITING)
					{
						FD_SET(ret, wr);
						FD_SET(ret, fds);
					}
					else if (job->type == Job::WAIT_FOR_CGIING)
					{
						FD_SET(job->fd, wr);
						job->type = Job::READY_TO_CGI;
					}
					else if (job->type == Job::WAIT_FOR_DELETING)
					{
						FD_SET(ret, wr);
					}
					return (0);
				}
				else
				{
					// Disconnect client from server
					std::cerr << "Setting 500 in " << __FILE__ << ":" << __LINE__ << std::endl;
					// TODO 
					exit(EXIT_FAILURE);
				}

				return 1;
			}

			int setupNewJobs(Job *job, Job::JOB_TYPE type)
			{
				int fd;

				fd = 0;
				Job::JOB_TYPE taskType;
				if (type == Job::WAIT_FOR_READING)
				{
					fd = this->createReadingJobs(job);
					taskType = Job::READING;
				}
				else if (type == Job::WAIT_FOR_WRITING)
				{
					fd = this->createWritingJobs(job);
					taskType = Job::WRITING;
				}
				else if (type == Job::WAIT_FOR_DELETING)
				{
					return (job->fd);
				}
				else if (type == Job::WAIT_FOR_CGIING)
				{
					this->createCGIJobs(job);
					return (job->fd);
				}
				else
				{
					std::cerr << "Setting 500 in " << __FILE__ << ":" << __LINE__ << std::endl;
					// TODO 
					exit(EXIT_FAILURE);
				}
				if (fd < 0)
					return (fd);
				if (fd == 0)
					return (fd);

				fcntl(fd, F_SETFL, O_NONBLOCK);
				this->jobs[fd].setType(taskType);
				this->jobs[fd].setFd(fd);
				this->jobs[fd].setServer(NULL);
				this->jobs[fd].setAddress("");
				this->jobs[fd].setClient(job);
				return (fd);
			}
			int createReadingJobs(Job *job)
			{
				Job::PATH_TYPE type;

				type = job->get_path_options();
				if (type == Job::NOT_FOUND)
				{
					bool autoindex;
					bool endsWithSlash;

					endsWithSlash = job->_getRequest()._uri.rbegin()[0] == '/';
					autoindex = job->correct_config.get_autoindex();
					if (autoindex == true && endsWithSlash == true)
						this->autoindex_module(job, job->correct_config);
					else
						job->set_xxx_response(job->correct_config, 404);
					return (0);
				}
				else if (type == Job::NO_PERMISSIONS)
				{
					job->set_xxx_response(job->correct_config, 403);
					return (0);
				}
				else if (type == Job::FILE_FOUND)
				{
					std::string ext;

					ext = job->_getRequest()._uri.substr(job->_getRequest()._uri.find_last_of(".") + 1);
					job->_getResponse().set_status_code(200);
					job->_getResponse().set_default_headers(ext);

					return this->openFileForReading(job->_getRequest()._uri);
				}
				else if (type == Job::DIRECTORY)
				{
					Configuration &config = job->correct_config;
					if (config.get_index().size() == 0)
						config.get_index().push_back("index.html");

					for (size_t i = 0; i < config.get_index().size(); i++)
					{
						std::string newpath = job->_getRequest()._uri + config.get_index()[i];

						type = job->get_path_options(newpath);
						if (type == Job::NOT_FOUND)
							continue;
						else if (type == Job::NO_PERMISSIONS)
						{
							job->set_xxx_response(job->correct_config, 403);
							return (0);
						}
						else if (type == Job::FILE_FOUND)
						{
								std::string ext;

								ext = newpath.substr(newpath.find_last_of(".") + 1);
								job->_getResponse().set_status_code(200);
								job->_getResponse().set_default_headers(ext);

							return this->openFileForReading(newpath);
						}
					}
					if (config.get_autoindex() == true)
						this->autoindex_module(job, job->correct_config);
					else
						job->set_xxx_response(config, 404);
					return (0);
				}
				else
				{
					std::cerr << "Setting 500 in " << __FILE__ << ":" << __LINE__ << std::endl;
					job->set_xxx_response(job->correct_config, 500);
					return (0);
				}
			}
			int openFileForReading(std::string const &path)
			{
				int fd;

				fd = open(path.c_str(), O_RDONLY);
				if (fd >this->_max_fd)
					this->_max_fd = fd;
				if (fd < 0)
				{
					// Disconnect client
					exit(1);
					return (-1);
				}
				return (fd);
			}
			void searchInDirectory();

			int readFile(int fd, std::string const &uri, Response &resp)
			{
				resp.set_status_code(200);
				resp.set_default_headers(uri.substr(uri.find_last_of(".") + 1));

				int ret, pos = 0;
				char *pointer = NULL;
				char buf[4096 + 1];
				
				bzero(buf, 4096 + 1);
				while ((ret = read(fd, buf, 4096)) > 0)
				{
					if (ret < 0)
						return 1;
					pointer = ft_strnstr(buf, "\r\n\r\n", ret);
					if (pointer != NULL)
						pos = ft_strnstr(buf, "\r\n\r\n", ret) - buf;
					else
						pos = 0;
					if (pointer != NULL && pos > 0)
						resp.set_header(std::string(buf, pos));
					resp.set_body(buf, ret, pos);
					if (ret < 4096)
						break;
					bzero(buf, 4096);
					pos = 0;
					pointer = NULL;
				}
				return (0);
			}


			int createWritingJobs(Job *job)
			{
				int fd;
				Job::PATH_TYPE type;
				std::string const &uri = job->_getRequest()._uri;

				type = job->get_path_options();

				if (type == Job::NO_PERMISSIONS)
				{
					job->set_xxx_response(job->correct_config, 403);
					return (0);
				}
				if (type == Job::DIRECTORY || job->get_path_options(uri + "/") == Job::DIRECTORY)
				{
					job->set_xxx_response(job->correct_config, 400);
					return (0);
				}
				fd = this->openFileForWriting(uri);
				return (fd);
			}
			int createCGIJobs(Job *job)
			{
				Job::PATH_TYPE type;
				std::string extension;
				Request &req = job->_getRequest();

				std::string &path = job->correct_config.get_cgi().find(extension)->second;
				std::string const &uri = req._uri;
				extension = req._uri.substr(req._uri.find_last_of("."));

				type = job->get_path_options(path);
				if (type == Job::NO_PERMISSIONS)
				{
					job->set_xxx_response(job->correct_config, 403);
					return (0);
				}
				if (type == Job::DIRECTORY || job->get_path_options(uri + "/") == Job::DIRECTORY)
				{
					job->set_xxx_response(job->correct_config, 400);
					return (0);
				}
				if (type == Job::NOT_FOUND)
				{
					job->set_xxx_response(job->correct_config, 404);
					return (0);
				}
				return (1);
			}


			int openFileForWriting(std::string const &path)
			{
				int fd;
				fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
				if (fd >this->_max_fd)
					this->_max_fd = fd;
				if (fd < 0)
				{
					// Disconnect client
					return (-1);
				}
				return (fd);
			}
			void getCorrectConfigForJob(Job *job, std::string &newpath)
			{
				ServerConfiguration &ref = this->get_correct_server_configuration(job);
				this->create_correct_configfile(job->request, ref, job->correct_config, newpath);
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
					if (request.is_method_post() == true && request.get_header("content-length").empty() == true)
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
				if (get == true && job->correct_config.is_method_allowed(Request::GET) == false)
					get = false;
				else if (post == true && job->correct_config.is_method_allowed(Request::POST) == false)
					post = false;
				else if (del == true && job->correct_config.is_method_allowed(Request::DELETE) == false)
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

				if (job->get_response().get_status_code() != 0)
				{
					job->set_3xx_response(job->correct_config);
					return (0);
				}
				return (1);
			}

			void postHandler(Job *job)
			{
				Request &req = job->_getRequest();
				std::vector<unsigned char>	&bodyVector = req._body;
				char	*body = reinterpret_cast<char*>(&bodyVector[0]);
				size_t bodyVec_size = bodyVector.size();

				int ret = write(job->fd, body, bodyVec_size);
				if (ret == -1)
				{
					job->bytes_sent = 0;
					job->type = Job::TASK_REMOVE;
					return ;
				}
				job->bytes_sent += ret;
				if (job->bytes_sent >= bodyVec_size)
				{
					// std::cout 
					job->bytes_sent = 0;
					job->client->type = Job::READY_TO_WRITE;
					job->type = Job::TASK_REMOVE;
					job->client->set_xxx_response(job->client->correct_config, 201);
					return ;
				}
			}

			// void http_index_module() // Function that calls functiosn below when nessecasry
			// {
			// 	std::cout << "http_index_module" << std::endl;
			// }
			// void http_file_module()
			// {
			// 	std::cout << "http_file_module" << std::endl;
			// }
			int autoindex_module(Job *job, Configuration &config)
			{
				std::string temp;
				Request &req = job->_getRequest();
				Response &res = job->_getResponse();

				// check root options

				int ret = get_root_options(req._uri.c_str());
				if (ret == -1 || ret == 1)
				{
					job->set_xxx_response(config, 404);
					return (0);
				}


				if (job->generate_autoindex(job, req._uri, temp) == 0)
					res.set_body(temp.c_str(), temp.size(), 0);
				else
				{
					std::cerr << "Setting 500 in " << __FILE__ << ":" << __LINE__ << std::endl;
					job->set_xxx_response(config, 500);
				}
				return (0);
			}
			// void http_dir_module()
			// {
			// 	std::cout << "http_dir_module" << std::endl;
			// }
			// void http_error_module()
			// {
			// 	std::cout << "http_error_module" << std::endl;
			// }


























};

#endif // WEBSERV_HPP
