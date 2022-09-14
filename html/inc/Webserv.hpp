
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

# include <cstring>
# include <sys/socket.h>
# include <netinet/in.h>
# include <sys/stat.h> // stat


# include "Configuration.hpp" // Base Class
# include "ServerConfiguration.hpp" // Derived from Configuration
# include "LocationConfiguration.hpp" // Derived from Configuration

# include "Parser.hpp" // Class That parses a config file.
# include "Job.hpp" // Class that handles a job. (write read to users)
# include "Server.hpp" // Class that handles a server.
# include "Request.hpp" // Class that handles a request.
# include "Response.hpp" // Class that handles a response.


#define getString(n) #n
#define VAR(var) std::cerr << std::boolalpha << __LINE__ << ":\t" << getString(var) << " = [" <<  (var) << "]" << std::noboolalpha << std::endl;
#define PRINT(var) std::cout << var << std::endl

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

		void run()
		{
			Job *job;
			while (true)
			{
				fd_set copy_readfds = this->read_fds;
				fd_set copy_writefds = this->write_fds;
				if (select((int)this->_max_fd + 1, &copy_readfds, &copy_writefds, NULL, NULL) < 0)
					throw std::runtime_error("select: failed to select.");
				for (size_t loop_job_counter = 0; loop_job_counter < this->jobs.size(); loop_job_counter++)
				{
					if (FD_ISSET(loop_job_counter, &copy_readfds))
					{
						job = &this->jobs[loop_job_counter];
						if (job->type == WAIT_FOR_CONNECTION)
							accept_connection(job, this->jobs, &this->read_fds);
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
						// else other job->type
					}
				}
				for (size_t loop_job_counter = 0; loop_job_counter < this->jobs.size(); loop_job_counter++)
				{
					job = &this->jobs[loop_job_counter];
					if (FD_ISSET(loop_job_counter, &copy_writefds))
					{
						if (job->type == CLIENT_RESPONSE)
							this->client_response(job);
					}
				}
			}
		}
	public:
		std::vector<ServerConfiguration>	&configs;	// Vector of all the server configurations.
		std::map<int, Server>				servers;	// Map of all the servers that are running. | Key = port | Value = Server
		std::map<int, Job>					jobs;		// List of all jobs. it includes the Servers and Clients. | Key = socket | Value = Job
		fd_set								read_fds;	// List of all file descriptors that are ready to read.
		fd_set								write_fds;	// List of all file descriptors that are ready to write.

	private:
		size_t _max_fd;
		/* User Types Handling */
		int client_read(Job *job, size_t &loop_job_counter, fd_set *copy_writefds)
		{
			// std::cout << job->fd << " Reading request Client\n";
			char	buffer[4096 + 1];
			int		bytesRead;
			bzero(buffer, 4096 + 1);
			bytesRead = recv(job->fd, buffer, 4096, 0);
			if (bytesRead <= 0)
			{
				close(job->fd);
				std::cout << "Client " << job->fd << " disconnected." << std::endl;
				FD_CLR(job->fd, &this->read_fds);
				FD_CLR(job->fd, &this->write_fds);
				this->jobs.erase(job->fd);
				return (0);
			}
			// TODO is Chunked? if so, read until 0\r
			Request request(buffer);

			job->request = request;

			ServerConfiguration &server_config = job->server->get_configurations()[0];
			Configuration &config = this->get_correct_server_configuration(job);
			try
			{
				config = this->create_correct_configfile(request, server_config); // the good
			}
			catch(const std::exception& e)
			{
				config = server_config;
			}

			/* set file read */
			if (request._method == "GET" && config.is_method_allowed("GET") == true)
			{
				loop_job_counter--;
				job->type = FILE_READ; // MOET FILE_READ ZIJN
			}
			else if (request._method == "POST" && config.is_method_allowed("POST") == true)
			{
				// *loop_job_counter--;
				job->type = FILE_WRITE;
			}
			else if (request._method == "DELETE" && config.is_method_allowed("DELETE") == true)
			{
				// *loop_job_counter--;
				job->type = FILE_WRITE;
			}
			else
			{
				// *loop_job_counter--;
				job->set_405_response(config);
				job->set_client_response(copy_writefds);
			}
			return (0);
		}

		void file_read(Job *job, fd_set *copy_writefds)
		{
			// check if uri is a file or directory
			// add root infront of uri
			// if file, read file
			// if directory && autoindex, list directory
			// else directory, read directory based on index order
			// if not found, set 404 response else auto index

			char type; // is file or directory
			int fd; // file descriptor)
			Configuration &config = this->get_correct_server_configuration(job);

			type = job->get_path_options(config.get_root());

			VAR(type);

			if (type == '0') // NOT FOUND
			{
				job->get_response().set_404_response(config);
			}
			else if (type == 'X') // FORBIDDEN
			{
				job->get_response().set_403_response(config);
			}
			else if (type == 'F') // FILE
			{
				std::string &uri = job->get_request()._uri;
				std::string extension = uri.substr(uri.find_last_of(".") + 1);

				if (extension == uri)
					extension = "";
				
				if (extension != "") // file has extension
				{
					if (config.get_cgi().find(extension) != config.get_cgi().end())
					{
						// DO CGI STUFF
						job->get_response().set_405_response(config); // TESTING
						job->set_client_response(copy_writefds);
						return ;
					}
				}
				std::string path = config.get_root() + job->get_request()._uri;
				std::ifstream in(path, std::ios::in);
				if (!in)
					job->get_response().set_500_response(config); // TODO check which error


				job->get_response().set_status_code(200);
				job->get_response().set_default_headers(extension);

				std::stringstream contents;
				contents << in.rdbuf();
				in.close();
				job->get_response().set_body(contents.str());

				job->get_response().set_content_length();
				// job->set_client_response(copy_writefds);
			}
			else if (type == 'D') // DIRECTORY
			{
				// Check AUTOINDEX
				// else check order of index
				// else 404
				job->get_response().set_405_response(config); // TESTING
			}
			else // ERROR
			{
				job->get_response().set_500_response(config);
			}

			job->set_client_response(copy_writefds);
		}

		void client_response(Job *job) // send response to client
		{
			std::vector<char> vec_response = job->response.build_response();
			size_t response_size = vec_response.size();
			char *response_char = reinterpret_cast<char*> (&vec_response[0]);

			size_t bytes = 0;
			while (response_size > 0) 
			{
				bytes = send(job->fd, response_char, response_size, 0);

				if (bytes < 0) 
					throw std::runtime_error("Error sending response to client");
				response_size -= bytes;
				response_char += bytes;
			}

			job->clear();
			this->jobs[job->fd].type = CLIENT_READ; // TODO or job->type = CLIENT_READ
		}




		/* Handle Connection */
		Configuration	create_correct_configfile(Request &request, ServerConfiguration &config)
		{
			LocationConfiguration *location;

			// TODO Putting a / at the end if there is not a slash (/) Makes sense??

			location = config.get_location_by_uri(request._uri);
			if (location == nullptr)
			{
				std::string string_with_slash = request._uri + "/";

				location = config.get_location_by_uri(string_with_slash); //  TODO MOET DIT???? zodat /redirect ook werkt en niet alleen /redirect/
				if (location == nullptr)
					throw std::runtime_error("Location block not found.");
			}
			return (*location);
		}
		size_t			method_handling(Request &request, Configuration &config)
		{
			if (config.is_method_allowed(request._method) == false)
				return (405);
			return (200); // 200 OK
		}
		/* Methods */
		void get(Job *job, Request &request, ServerConfiguration &config)
		{
			std::cout << "GET" << std::endl;
			(void)job;
			(void)request;
			(void)config;
		}
		void post(Job *job, Request &request, ServerConfiguration &config)
		{
			std::cout << "POST" << std::endl;
			(void)job;
			(void)request;
			(void)config;
		}
		void put(Job *job, Request &request, ServerConfiguration &config)
		{
			std::cout << "PUT" << std::endl;
			(void)job;
			(void)request;
			(void)config;
		}
		/* Accept a New Client */
		int accept_connection(Job *job, std::map<int, Job> &jobs, fd_set *set)
		{
			struct sockaddr_in client_address;
			int address_size = sizeof(struct sockaddr_in);
			size_t client_fd = accept(job->fd, (struct sockaddr*)&client_address, (socklen_t*)&address_size);
			if (client_fd < 0)
				throw std::runtime_error("accept: failed to accept.");

			User user(client_fd, &client_address);
			fcntl(client_fd, F_SETFL, O_NONBLOCK);
			jobs[client_fd] = Job(CLIENT_READ, client_fd, job->server, &user);

			if (client_fd > this->_max_fd)
				this->_max_fd = client_fd;
			
			FD_SET(client_fd, set);
			return (client_fd);
		}
		/* Fd_sets Setting Methods */
		void	setFdSets()
		{
			std::map<int, Server>::iterator it;
			size_t fd;
			
			// Zeroing the fd_sets.
			FD_ZERO(&this->read_fds);
			FD_ZERO(&this->write_fds);

			for (it = this->servers.begin(); it != this->servers.end(); it++)
			{
				// std::cout << "server " << it->second.get_port() << " has " << it->second.get_configurations().size() << " configurations" << std::endl;
				fd = it->second.get_socket();
				this->jobs[fd] = Job(WAIT_FOR_CONNECTION, fd, &it->second, NULL);
				FD_SET(fd, &this->read_fds);
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
						std::cout << "Creating server on port " << ports[j].second << std::endl;
						s = openSocket(ports[j].second);
						h = (char*)ports[j].first.c_str();
						p = ports[j].second;
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
		/* Server Utilities Methods */
		ServerConfiguration &get_correct_server_configuration(Job *job)
		{

			int port = this->get_port_from_job(job);
			std::vector<ServerConfiguration> &job_configs = this->servers[port].get_configurations();

			std::vector<std::string> server_names;
			std::string host_header;
			size_t pos;

			ServerConfiguration &default_server = job_configs[0]; // correct server of the job


			host_header = job->request._headers_map["host"];
			pos = host_header.find(":");

			for (size_t k = 0; k < job_configs.size(); k++)
			{
				server_names = job_configs[k].get_server_names();
				for (size_t j = 0; j < server_names.size(); j++)
				{
					if (host_header.substr(0, pos) == server_names[j])
						default_server = job_configs[k];
				}
			}
			return default_server;
		}
		size_t get_port_from_job(Job *job)
		{
			return job->server->get_port();
		}
};

#endif // WEBSERV_HPP