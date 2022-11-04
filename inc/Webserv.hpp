
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
# include "utils.hpp"


#define getString(n) #n
#define VAR(var) std::cerr << std::boolalpha << __FILE__ ":"<< __LINE__ << ":\t" << getString(var) << " = [" <<  (var) << "]" << std::noboolalpha << std::endl;
#define PRINT(var) std::cout << var << std::endl

bool g_is_running = true;


class Webserv
{
	public:
		/* Constructor  */
		Webserv(std::vector<ServerConfiguration> &configs)
		: configs(configs), _max_fd(0)
		{
			
		}

		/* Destructor */
		virtual ~Webserv()
		{
		}

		/* Copy constructor */
		Webserv(const Webserv&);

		/* Operation overload = */
		Webserv& operator = (const Webserv& e);

		// Methods
		void setupServers()
		{
			// Opening sockets for each ServerConfiguration.
			this->openingSockets();

			/* Setting up all the fd_sets for future reading with select() */
			this->setFdSets();
		}

		static void interruptHandler(int sig_int)
		{
			(void)sig_int;
			std::cout << "\b\b \b\b";
			g_is_running = false;
			throw std::runtime_error("Interrupted");
		}
		void run()
		{
			signal(SIGINT, interruptHandler);
			signal(SIGQUIT, interruptHandler);

			std::cerr << CLRS_GRN << "server : starting" << CLRS_reset << std::endl;
			Job *job;
			while (g_is_running == true)
			{
				fd_set copy_readfds = this->fds;
				fd_set copy_writefds = this->fds;
				if (select((int)this->_max_fd + 1, &copy_readfds, &copy_writefds, NULL, NULL) < 0)
					throw std::runtime_error("select: failed to select.");
				for (size_t loop_job_counter = 0; loop_job_counter < this->jobs.size(); loop_job_counter++)
				{
					if (FD_ISSET(loop_job_counter, &copy_readfds))
					{
						job = this->jobs[loop_job_counter];
						if (job->type == WAIT_FOR_CONNECTION)
							accept_connection(job, this->jobs, &this->fds);
						else if (job->type == CLIENT_READ)
						{
							if (this->client_read(job, loop_job_counter, &copy_writefds) == 0)
								continue;
						}
						else if (job->type == FILE_READ)
						{
							// check redirect
							this->file_read(job, &copy_writefds);
						}
						else if ( job->type == CGI_WRITE || job->type == CGI_READ)
							this->do_cgi(job, &copy_writefds);
					}
				}
				for (size_t loop_job_counter = 0; loop_job_counter < this->jobs.size(); loop_job_counter++)
				{
					job = this->jobs[loop_job_counter];
					if (FD_ISSET(loop_job_counter, &copy_writefds))
					{
						if (job->type == CLIENT_RESPONSE)
							this->client_response(job);
						else if (job->type == FILE_WRITE) // DELETE
						{
							if (remove(job->get_request()._uri.c_str()) < 0)
							{
								if (errno == EACCES)
									job->set_xxx_response(job->correct_config, 403);
								else
									job->set_xxx_response(job->correct_config, 404);
							}
							else
							{
								if (DEBUG == 1)
									std::cerr << CLRS_RED <<  "FILE DELETED \'" << CLRS_MAG << job->get_request()._uri << CLRS_RED << "\'" << CLRS_reset << std::endl;
								job->set_xxx_response(job->correct_config, 204);
							}
							this->client_response(job);
						}
					}
				}
			}
		}
	public:
		std::vector<ServerConfiguration>	&configs;	// Vector of all the server configurations.
		std::map<int, Server>				servers;	// Map of all the servers that are running. | Key = port | Value = Server
		std::map<int, Job*>					jobs;		// List of all jobs. it includes the Servers and Clients. | Key = socket | Value = Job
		fd_set								fds;	// List of all file descriptors that are ready to read.

		int closeConnection(Job **ptr)
		{
			Job *job;
			int copyFD;

			job = *ptr;
			copyFD = job->fd;

			close(job->fd);
			FD_CLR(job->fd, &this->fds);
			delete job;
			this->jobs.erase(copyFD);
			*ptr = NULL;
			return copyFD;
		}
	private:
		int _max_fd;
		/* User Types Handling */
		int client_read(Job *job, size_t &loop_job_counter, fd_set *copy_writefds)
		{
			size_t server_index = 0;
			std::string ConfigToChange_path = "";
			char	buffer[4096 + 1];
			int		bytesRead = 0;
			bzero(buffer, 4096 + 1);
			bytesRead = recv(job->fd, buffer, 4096, 0);
			if (bytesRead <= 0)
			{

				// close(job->fd);
				if (DEBUG == 1)
					std::cerr << CLRS_GRN << "server : connection closed by client " << job->fd << CLRS_reset << std::endl;
				this->closeConnection(&job);
				// FD_CLR(job->fd, &this->fds);
				// delete job->_address_info;
				// this->jobs.erase(job->fd);
				return (0);
			}

			// TODO is Chunked? if so, read until 0\r
			job->request.add_incoming_data(buffer, bytesRead);

			Request::Type rtype = job->request.get_type();

			if (rtype == Request::MAX_ENTITY)
			{
				job->set_xxx_response(job->correct_config, 431);
				job->get_response().set_header("Connection: close");
				this->client_response(job);
				return (0);
			}
			if (job->get_request()._type != Request::ERROR && job->request.is_complete() == false)
				return (1); 


			bool get = job->request.is_method_get();
			bool post = job->request.is_method_post();
			bool del = job->request.is_method_delete();

			/* Checks */
			// 413 Request Entity Too Large
			// 400 Bad Request
			if (job->get_request().is_bad_request() == true)
			{
				if (job->get_request()._method == Request::UNSUPPORTED)
					job->set_xxx_response(job->correct_config, 405);
				else
					job->set_xxx_response(job->correct_config, 400);
				this->client_response(job);
				return (0);
			}
			// 505 HTTP Version Not Supported
			if (job->get_request().is_http_supported() == false)
			{
				job->set_xxx_response(job->correct_config, 505);
				this->client_response(job);
				return (0);
			}
			// 414 Request-URI Too Long
			if (job->get_request()._uri.size() > 1024)
			{
				job->set_xxx_response(job->correct_config, 414);
				this->client_response(job);
				return (0);
			}
			if (post && job->correct_config.get_client_max_body_size() < job->get_request()._body.size())
			{
				job->set_xxx_response(job->correct_config, 413);
				job->set_client_response(copy_writefds);
				return (0);
			}



			ServerConfiguration &server_configuration = this->get_correct_server_configuration(job, server_index);
			this->create_correct_configfile(job->request, server_configuration, job->correct_config, ConfigToChange_path);

			std::string &uri = job->get_request()._uri;
			std::string extension = uri.substr(uri.find_last_of(".") + 1);

			if (extension == uri)
				extension = "";
			if (get == true && job->correct_config.is_method_allowed("GET") == false)
				get = false;
			else if (post == true && job->correct_config.is_method_allowed("POST") == false)
				post = false;
			else if (del == true && job->correct_config.is_method_allowed("DELETE") == false)
				del = false;

			std::map<std::string, std::string>::iterator it;
			it = job->correct_config.get_cgi().find("." + extension);

			if (del == true) // method is DELETE
			{
				job->set_client_response(copy_writefds);
				job->type = FILE_WRITE;
			}
			else if (extension != "" &&  it != job->correct_config.get_cgi().end())
			{
				if (get == true)
					job->type = CGI_READ;
				else
					job->type = CGI_WRITE;
			}
			else
			{
				if (get == true)
					job->type = FILE_READ;
				else
				{
					job->set_405_response(job->correct_config);
					job->set_client_response(copy_writefds);
				}
			}
			loop_job_counter--;
			/* Parse Request */
			job->parse_request(ConfigToChange_path);

			if (DEBUG == 1)
			{
				// Print nice things
				std::stringstream ss;
				ss << CLRS_YEL;
				ss << "server : << [method: " << job->get_request().method_to_s() << "] ";
				ss << "[target: " << job->get_request().get_unedited_uri() << "] ";
				ss << "[server: " << server_index << "] ";
				ss << "[location: " << ConfigToChange_path << "] ";
				ss << "[client fd: " << job->fd << "]";
				ss << CLRS_reset << std::endl;
				std::cerr << ss.str();
			}
			return (0);
		}

		char file_read(Job *job, fd_set *copy_writefds, bool isRecursive = false, Job::PATH_TYPE type = Job::NOT_FOUND)
		{
			Configuration &config = job->correct_config;
			if (isRecursive == false)
				type = job->get_path_options();

			if (job->get_response().get_status_code() != 0)
			{
				job->set_3xx_response(config);
			}
			else if (type == Job::NOT_FOUND) // NOT FOUND
			{
				if (job->get_response().is_body_empty() == false)
					;
				else if (config.get_autoindex() == true && job->get_request()._uri.rbegin()[0] == '/')
					job->generate_autoindex_add_respone(config);
				else
					job->set_xxx_response(config, 404);
			}
			else if (type == Job::NO_PERMISSIONS) // FORBIDDEN
			{
				job->set_xxx_response(config, 403);
			}
			else if (type == Job::FILE_FOUND) // FILE
			{
				int fd;

				fd = open(job->get_request()._uri.c_str(), O_RDONLY);
				if (fd == -1)
					job->set_xxx_response(config, 500);
				job->handle_file(fd);
				close(fd);
			}
			else if (type == Job::DIRECTORY) // DIRECTORY
			{
				size_t i = 0;
				Job::PATH_TYPE copy_type;

				// if (config.get_autoindex() == true && job->get_request()._uri[job->get_request()._uri.size() - 1] != '/')
				// if (config.get_autoindex() == true &&)
				// 	job->generate_autoindex_add_respone(config);
				// else
				// {
					/* Do files */
				if (isRecursive == true)
				{
					// Should not go here
					job->set_xxx_response(config, 400);
					job->set_client_response(copy_writefds); 
				}
				else
				{
					if (config.get_index().size() == 0)
						config.get_index().push_back("index.html");

					for (i = 0; i < config.get_index().size(); i++)
					{
						Job copy = *job;
						copy.get_request()._uri = copy.get_request()._uri + config.get_index()[i];
						copy_type = copy.get_path_options();
						if (copy_type == Job::NOT_FOUND)
							continue;
						this->file_read(&copy, copy_writefds, true, copy_type);
						if (copy_type == Job::FILE_FOUND || copy_type == Job::NO_PERMISSIONS)
						{
							job->get_request() = copy.get_request();
							job->get_response() = copy.get_response();
							break;
						}
					}
					if (copy_type != Job::FILE_FOUND  && copy_type != Job::NO_PERMISSIONS && config.get_autoindex() == true)
						job->generate_autoindex_add_respone(config);
					else if (copy_type != Job::FILE_FOUND && copy_type != Job::NO_PERMISSIONS)
					{
						job->set_xxx_response(config, 404);
					}
				}

				// file read RECURIVE ???
				// this->fileread(a new job, copy_writefds, true) ???

				// Check AUTOINDEX
				// else check order of index
				// else 404
			}
			else // ERROR
				job->set_xxx_response(config, 500);
			if (isRecursive == false)
				job->set_client_response(copy_writefds); 
			return type;
		}

		void client_response(Job *job) // send response to client
		{
			bool connection_close = false;
			int bytes;
			job->response.build_response_text();
			std::vector<unsigned char> &response = job->response.get_response();
			size_t response_size = response.size();
			char *response_char = reinterpret_cast<char*> (&response[0]);


			if (DEBUG == 1)
			{
				std::stringstream ss;
				ss << CLRS_BLU;
				bytes = ft_strnstr(response_char, "\r\n", 60) - response_char;
				ss << "server : >> [status: " << std::string(response_char, bytes)  << "] ";
				ss << "[length: " << response_size << "] ";
				ss << "[client: " << job->fd << "] ";
				ss << CLRS_reset;
				std::cerr << ss.str() << std::endl;
			}

			bytes = 0;
			while (response_size > 0) 
			{
				bytes = send(job->fd, response_char, response_size, 0);
				if (bytes < 0) 
					throw std::runtime_error("Error sending response to client");
				response_size -= bytes;
				response_char += bytes;
			}

			if (job->get_response().get_headers().find("Connection: close") != std::string::npos)
				connection_close = true;

			job->clear();
			this->jobs[job->fd]->clear();
			this->jobs[job->fd]->type = CLIENT_READ; // TODO or job->type = CLIENT_READ


			if (connection_close)
			{
				if (DEBUG == 1)
					std::cerr << CLRS_GRN << "server : connection closed by server " << job->fd << CLRS_reset << std::endl;
				this->closeConnection(&job);
				// this->jobs.erase(job->fd);
			}
		}

		void do_cgi(Job *job, fd_set *copy_writefds)
		{
			std::vector<unsigned char>	&bodyVector = job->get_request()._body;
			bodyVector.push_back('\0');
			char						*body = reinterpret_cast<char*>(&bodyVector[0]);
			size_t bodyVec_size = bodyVector.size();

			std::string extension = job->get_request()._uri.substr(job->get_request()._uri.find_last_of("."));


			std::string path = job->correct_config.get_cgi().find(extension)->second;

			if (access(path.c_str(), F_OK) != 0) // File doesn't exist
			{
				job->set_xxx_response(job->correct_config, 404);
			}
			else // File Exist
			{
				pid_t pid;
				int fd_out[2];
				int fd_in[2];
				bool post = job->get_request().is_method_post();
				bool get = job->get_request().is_method_get();

				if (pipe(fd_out) < 0)
					throw std::runtime_error("pipe: failed to create pipe on fd_out.");
				
				if (post && pipe(fd_in) < 0)
					throw std::runtime_error("pipe: failed to create pipe on fd_in.");

				pid = fork();
				if (pid < 0)
					throw std::runtime_error("fork: failed to fork.");

				if (pid == 0)
				{
					job->set_environment_variables();

					close(fd_out[0]);

					dup2(fd_out[1], 1);
					close(fd_out[1]);
					if (post)
					{
						close(fd_in[1]);
						dup2(fd_in[0], STDIN_FILENO);
						close(fd_in[0]);
					}
					char	*args[2] = {const_cast<char*>(path.c_str()), NULL};
					execv(path.c_str(), args);
					exit(EXIT_FAILURE);
				}
				close(fd_out[1]);
				if (post)
				{
					close(fd_in[0]);
					write(fd_in[1], body, bodyVec_size);
				}

				if (get == true)
					job->handle_file(fd_out[0]);
				else if (post == true)
					job->handle_file(fd_out[0]);
				else
					throw std::runtime_error("Something went terribbly wrong");
			}
			job->set_client_response(copy_writefds);
		}


		/* Handle Connection */
		int	create_correct_configfile(Request &request, ServerConfiguration &config, Configuration &ConfigToChange, std::string &ConfigToChange_path)
		{
			LocationConfiguration *location;

			location = config.get_location_by_uri(request._uri);
			if (location == NULL) // nullpointer doesnt work
			{
				/* No Location Block found: use default config */
				ConfigToChange = config;
				ConfigToChange_path.clear();
				return (1);
			}
			ConfigToChange = *location;
			ConfigToChange_path = location->get_path();
			return (0);
		}
		size_t			method_handling(Request &request, Configuration &config)
		{
			if (config.is_method_allowed(request.method_to_s()) == false)
				return (405);
			return (200); // 200 OK
		}
		/* Methods */
		/* Accept a New Client */
		int accept_connection(Job *job, std::map<int, Job*> &jobs, fd_set *set)
		{
			int client_fd = 0;
			size_t address_size = 0;

			struct sockaddr_in *client_address = new struct sockaddr_in;
			address_size = sizeof(struct sockaddr_in);
			bzero(client_address, address_size);
			
			client_fd = accept(job->fd, (struct sockaddr*)client_address, (socklen_t*)&address_size);
			if (client_fd < 0)
				throw std::runtime_error("accept: failed to accept.");

			// User *user = new User(client_fd, client_address); // TODO FREE WHEN JOB IS
			fcntl(client_fd, F_SETFL, O_NONBLOCK);
			jobs[client_fd] = new Job(CLIENT_READ, client_fd, job->server, client_address);

			if (client_fd >this->_max_fd)
				this->_max_fd = client_fd;
			if (DEBUG == 1)
			{
				std::cerr << CLRS_GRN << "server : new connection on " << job->server->get_hostname() << ":" << job->server->get_port() << " [client: " << client_fd << "]";
				std::cerr << " [ip: " << inet_ntoa(client_address->sin_addr) << "]" << CLRS_reset << std::endl;
			}

			FD_SET(client_fd, set);
			return (client_fd);
		}
		/* Fd_sets Setting Methods */
		void	setFdSets()
		{
			std::map<int, Server>::iterator it;
			int fd;
			
			// Zeroing the fd_sets.
			FD_ZERO(&this->fds);

			for (it = this->servers.begin(); it != this->servers.end(); it++)
			{
				fd = it->second.get_socket();
				this->jobs[fd] = new Job(WAIT_FOR_CONNECTION, fd, &it->second, NULL);
				FD_SET(fd, &this->fds);
				if (fd > this->_max_fd)
					this->_max_fd = fd;
			}
		}
		/* Socket Opening Methods */
		void	openingSockets() /* Opening all the sockets and adding them to a vector */
		{
			std::vector<std::pair<std::string, size_t> > ports;
			int s = 0;
			char *h = NULL;
			in_port_t p = 0;

			for (size_t i = 0; i < this->configs.size(); i++)
			{
				ports = this->configs[i].get_listen();
				for (size_t j = 0; j < ports.size(); j++)
				{
					std::map<int, Server>::iterator it = this->servers.find(ports[j].second);
					if (it == this->servers.end())
					{
						s = openSocket(ports[j].second, ports[j].first.c_str());
						if (ports[j].first == "")
							h = strdup("127.0.0.1");
						else
							h = strdup(ports[j].first.c_str());
						if (h == NULL)
							throw std::runtime_error("strdup: failed to duplicate string.");
						p = ports[j].second;
						if (DEBUG == 1)
							std::cerr << CLRS_GRN << "Listening on " << h << ":" << p << CLRS_reset << std::endl;
						this->servers[ports[j].second] = Server(s, h, p, this->configs[i]);
					}
					else
						it->second.push_back(this->configs[i]);
				}
				ports.clear();
			}
		}
		int		openSocket(int port, const char *hostname = "")
		{
			int ret;
			struct sockaddr_in		sock_struct;
			int						socketFD;

			socketFD = socket(AF_INET, SOCK_STREAM, 0);
			if (socketFD < 0)
				throw std::runtime_error("socket: failed to create socket.");
			int options = 1;
			ret = setsockopt(socketFD, SOL_SOCKET, SO_REUSEPORT, &options, sizeof(options));
			if (ret < 0)
				throw std::runtime_error("Failed to set socket options.");

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
		/* Server Utilities Methods */
		ServerConfiguration &get_correct_server_configuration(Job *job, size_t &i)
		{
			size_t index = 0;
			int port = this->get_port_from_job(job);
			std::vector<ServerConfiguration> &job_configs = this->servers[port].get_configurations();

			std::vector<std::string> server_names;
			std::string host_header;
			size_t pos;

			ServerConfiguration &default_server = job_configs[0]; // correct server of the job


			host_header = job->request._headers_map["host"];
			pos = host_header.find(":");
			pos = (pos == std::string::npos) ? host_header.size() : pos;

			for (index = 0; index < job_configs.size(); index++)
			{
				server_names = job_configs[index].get_server_names();
				for (size_t j = 0; j < server_names.size(); j++)
				{
					if (host_header.substr(0, pos) == server_names[j])
					{
						default_server = job_configs[index];
					}
				}
			}
			i = index;
			return default_server;
		}
		size_t get_port_from_job(Job *job)
		{
			return job->server->get_port();
		}
};

#endif // WEBSERV_HPP