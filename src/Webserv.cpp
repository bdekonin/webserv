/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Webserv.cpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/11/06 20:25:27 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/11/08 21:19:41 by bdekonin      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Webserv.hpp"

bool g_is_running = true;

/* Constructor */
Webserv::Webserv(std::vector<ServerConfiguration> &configs)
: configs(configs), _max_fd(0)
{
}

/* Destructor */
Webserv::~Webserv()
{
}

/* Signals */
// static void				interruptHandler(int sig_int)
// {
// 	(void)sig_int;
// 	g_is_running = false;
// 	std::cout << "\b\b \b\b";
// 	// std::cout << "Interrupt signal (" << sig_int << ") received." << std::endl;
// 	throw std::runtime_error("Interrupt signal (" + std::to_string(sig_int) + ") received.");
// 	// exit(EXIT_SUCCESS); // Dont use this because it will close the server without closing the connections.
// }

/* Public Main Methods */
void 					Webserv::setupServers()
{
	// Opening sockets for each ServerConfiguration.
	this->openingSockets();

	/* Setting up all the fd_sets for future reading with select() */
	this->setFdSets();
}
void 					Webserv::run()
{
	// signal(SIGINT, interruptHandler);
	// signal(SIGQUIT, interruptHandler);


	std::cerr << CLRS_GRN << "server : starting" << CLRS_reset << std::endl;
	Job *job;
	while (g_is_running == true)
	{
		fd_set copy_readfds = this->fds;
		fd_set copy_writefds = this->fds;
		if (select((int)this->_max_fd + 1, &copy_readfds, &copy_writefds, 0, 0) < 0)
			throw std::runtime_error("select() failed");
		for (auto it = this->jobs.begin(); it != this->jobs.end(); it++)
		{
			if (FD_ISSET(it->first, &copy_readfds))
			{
				job = &it->second;
				if (job->type == Job::WAIT_FOR_CONNECTION)
					accept_connection(job, &this->fds);
				else if (job->type == Job::READY_TO_READ)
				{
					this->requestRead(it, &this->fds, &copy_writefds, &copy_readfds);
				}
				else if (job->type == Job::READING)
				{
					int ret = it->second.fileReader(it->first);
					// if (ret > 0)
					// {
						it->second.client->type = Job::READY_TO_WRITE;
						it->second.type = Job::TASK_REMOVE;
						FD_SET(it->second.client->fd, &copy_writefds);
					// }
				}
				else if ( job->type == Job::CGI_WRITE || job->type == Job::CGI_READ)
					this->do_cgi(job, &copy_writefds);
			}
		}
		for (auto it = this->jobs.begin(); it != this->jobs.end(); it++)
		{
			if (FD_ISSET(it->first, &copy_writefds))
			{
				job = &it->second;
				if (job->type == Job::READY_TO_WRITE)
				{
					this->client_response(job);
					// exit(1);
					// this->responseWrite(job, &this->fds);
					// this->closeConnection(job, &this->fds);
				}
				else if (job->type == Job::WRITING)
				{
					// -1 means blocking
					this->postHandler(job);
				}
				else if (job->type == Job::DELETING)
				{
					
				}
				// if (job->type == Job::CLIENT_RESPONSE)
				// 	this->client_response(job);
				// else if (job->type == Job::FILE_WRITE) // DELETE
				// {
				// 	if (remove(job->get_request()._uri.c_str()) < 0)
				// 	{
				// 		if (errno == EACCES)
				// 			job->set_xxx_response(job->correct_config, 403);
				// 		else
				// 			job->set_xxx_response(job->correct_config, 404);
				// 	}
				// 	else
				// 	{
				// 		if (DEBUG == 1)
				// 			std::cerr << CLRS_RED <<  "FILE DELETED \'" << CLRS_MAG << job->get_request()._uri << CLRS_RED << "\'" << CLRS_reset << std::endl;
				// 		job->set_xxx_response(job->correct_config, 204);
				// 	}
				// 	this->client_response(job);
				// }
			}
		}
		for (std::map<int, Job>::iterator it = this->jobs.begin(); it != this->jobs.end(); it++)
		{
			if (it->second.type == Job::TASK_REMOVE)
			{
				this->closeConnection(it, "TASK");
			}
			if (it->second.type == Job::CLIENT_REMOVE)
			{
				this->closeConnection(it, "client");
			}
		}
	}
}
void 					Webserv::closeAll()
{
	Job *job;

	for (std::map<int, Job>::iterator it = this->jobs.begin(); it != this->jobs.end(); it++)
	{
		job = &it->second;
		if (job && job->fd > 2)
		{
			if (DEBUG == 1)
				std::cerr << CLRS_RED << "server : connection closed by " << "Webserv" << " " << job->fd << CLRS_reset << std::endl;
			close(job->fd);
		}
	}
}

/* Private Main Methods */
void					Webserv::closeConnection(iterator &it, const char *connectionClosedBy)
{
	if (DEBUG == 1 && connectionClosedBy)
			std::cerr << CLRS_RED << "server : connection closed by " << connectionClosedBy << " " << it->first << CLRS_reset << std::endl;

	FD_CLR(it->first, &this->fds);
	close(it->first);
	it = this->jobs.erase(it);
}
int 					Webserv::client_read(Job *job, size_t &loop_job_counter, fd_set *copy_writefds)
{
	std::string ConfigToChange_path = "";
	char	buffer[4096 + 1];
	int		bytesRead = 0;
	bzero(buffer, 4096 + 1);
	bytesRead = recv(job->fd, buffer, 4096, 0);
	if (bytesRead <= 0)
	{
		// this->closeConnection(job, "client");
		loop_job_counter--;
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
		// if (411)
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



	ServerConfiguration server_configuration = this->get_correct_server_configuration(job);
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
		job->type = Job::FILE_WRITE;
	}
	else if (extension != "" &&  it != job->correct_config.get_cgi().end())
	{
		if (get == true)
			job->type = Job::CGI_READ;
		else
			job->type = Job::CGI_WRITE;
	}
	else
	{
		if (get == true)
			job->type = Job::FILE_READ;
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
		ss << "[location: " << ConfigToChange_path << "] ";
		ss << "[client fd: " << job->fd << "]";
		ss << CLRS_reset << std::endl;
		std::cerr << ss.str();
	}
	return (0);
}
char 					Webserv::file_read(Job *job, fd_set *copy_writefds, bool isRecursive, Job::PATH_TYPE type)
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
		{
			std::cerr << "Setting 500 in " << __FILE__ << ":" << __LINE__ << std::endl;
			job->set_xxx_response(config, 500);
		}
		job->handle_file(fd, copy_writefds);
		close(fd);
	}
	else if (type == Job::DIRECTORY) // DIRECTORY
	{
		size_t i = 0;
		Job::PATH_TYPE copy_type;

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
	}
	else // ERROR
	{
		std::cerr << "Setting 500 in " << __FILE__ << ":" << __LINE__ << std::endl;
		job->set_xxx_response(config, 500);
	}
	if (isRecursive == false)
		job->set_client_response(copy_writefds); 
	return type;
}
void 					Webserv::client_response(Job *job)
{
	bool connection_close = false;
	int bytes;
	job->response.build_response_text();
	std::vector<unsigned char> &response = job->_getResponse().get_response();
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
		std::cout << "bytes: " << bytes << std::endl;
		if (bytes == -1)
		{
			job->type = Job::CLIENT_REMOVE;
			return ; 
		}
		response_size -= bytes;
		response_char += bytes;
	}

	if (job->get_request().get_header("connection").compare("Close") == 0)
		connection_close = true;

	job->clear();
	this->jobs[job->fd].clear();
	this->jobs[job->fd].type = Job::READY_TO_READ; // READY_TO_READ

	if (connection_close)
		job->type = Job::CLIENT_REMOVE;
}
void 					Webserv::do_cgi(Job *job, fd_set *copy_writefds)
{
	int ret;

	ret = 0;
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
			ret = write(fd_in[1], body, bodyVec_size);
			close(fd_in[1]);
		}

		if (ret < 0)
		{
			std::cerr << "Setting 500 in " << __FILE__ << ":" << __LINE__ << std::endl;
			job->set_xxx_response(job->correct_config, 500);
		}
		else if (get == true)
			job->handle_file(fd_out[0], copy_writefds);
		else if (post == true)
			job->handle_file(fd_out[0], copy_writefds);
		else
			throw std::runtime_error("Something went terribbly wrong");

		close(fd_out[0]);
	}
	job->set_client_response(copy_writefds);
}
size_t					Webserv::method_handling(Request &request, Configuration &config)
{
	if (config.is_method_allowed(request.method_to_s()) == false)
		return (405);
	return (200); // 200 OK
}
int 					Webserv::accept_connection(Job *job, fd_set *set)
{
	int client_fd = 0;
	size_t address_size = 0;

	struct sockaddr_in client_address;
	address_size = sizeof(struct sockaddr_in);
	bzero(&client_address, address_size);
	
	client_fd = accept(job->fd, (struct sockaddr*)&client_address, (socklen_t*)&address_size);
	if (client_fd < 0)
		throw std::runtime_error("accept: failed to accept.");

	fcntl(client_fd, F_SETFL, O_NONBLOCK);
	this->jobs[client_fd].setType(Job::READY_TO_READ);
	this->jobs[client_fd].setFd(client_fd);
	this->jobs[client_fd].setServer(job->server);
	this->jobs[client_fd].setAddress(&client_address);

	if (client_fd >this->_max_fd)
		this->_max_fd = client_fd;
	if (DEBUG == 1)
	{
		std::cerr << CLRS_GRN << "server : new connection on " << job->server->get_hostname() << ":" << job->server->get_port() << " [client: " << client_fd << "]";
		std::cerr << CLRS_reset << std::endl;
	}

	FD_SET(client_fd, set);
	return (client_fd);
}
void					Webserv::setFdSets()
{
	std::map<int, Server>::iterator it;
	int fd;
	
	// Zeroing the fd_sets.
	FD_ZERO(&this->fds);
	for (it = this->servers.begin(); it != this->servers.end(); it++)
	{
		fd = it->second.get_socket();
		// this->jobs[fd] = new Job(WAIT_FOR_CONNECTION, fd, &it->second, NULL);
		this->jobs[fd].setType(Job::WAIT_FOR_CONNECTION);
		this->jobs[fd].setFd(fd);
		this->jobs[fd].setServer(&it->second);
		FD_SET(fd, &this->fds);
		if (fd > this->_max_fd)
			this->_max_fd = fd;
	}
}
void					Webserv::openingSockets()
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
					std::cerr << CLRS_GRN << "Listening on [" << s << "] " << h << ":" << p << CLRS_reset << std::endl;
				this->servers[ports[j].second].set(s, h, p, this->configs[i]);
			}
			else
				it->second.push_back(this->configs[i]);
		}
		ports.clear();
	}
}
int						Webserv::openSocket(int port, const char *hostname)
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
		sock_struct.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	else if (strcmp(hostname, "localhost") == 0)
		sock_struct.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	else if (strcmp(hostname, "127.0.0.1") == 0)
		sock_struct.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	else if (strcmp(hostname, "0.0.0.0") == 0)
		sock_struct.sin_addr.s_addr = htonl(INADDR_ANY);
	else
		throw std::runtime_error("listen: failed to parse address on listen. " + std::string(hostname));




	sock_struct.sin_port = htons(port);

	if (bind(socketFD, (struct sockaddr*)&sock_struct, sizeof(sock_struct)) < 0)
		throw std::runtime_error("bind: Failed to bind.");
	if (listen(socketFD, 128) < 0)
		throw std::runtime_error("listen: failed to listen.");

	return socketFD;
}

int						Webserv::create_correct_configfile(Request &request, ServerConfiguration &config, Configuration &ConfigToChange, std::string &ConfigToChange_path)
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

ServerConfiguration 	&Webserv::get_correct_server_configuration(Job *job)
{
	size_t index = 0;
	size_t match = 0;
	int port = this->get_port_from_job(job);
	std::vector<ServerConfiguration> &job_configs = this->servers[port].get_configurations();

	std::vector<std::string> &server_names = job_configs[0].get_server_names();
	std::string host_header;
	size_t pos;

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
				match = index;
				break;
			}
		}
	}

	ServerConfiguration &ref = (match == 0) ? job_configs[0] : job_configs[match];

	return ref;
}
size_t 					Webserv::get_port_from_job(Job *job)
{
	return job->server->get_port();
}