/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Webserv.cpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/11/06 20:25:27 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/11/10 15:28:20 by bdekonin      ########   odam.nl         */
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
	std::cerr << CLRS_GRN << "server : starting" << CLRS_reset << std::endl;
	Job *job;
	while (g_is_running == true)
	{
		fd_set copy_readfds = this->fds;
		fd_set copy_writefds = this->fds;
		if (select((int)this->_max_fd + 1, &copy_readfds, &copy_writefds, 0, 0) < 0)
			throw std::runtime_error("select() failed");
		for (iterator it = this->jobs.begin(); it != this->jobs.end(); it++)
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
					if (ret == -1)
					{
						it->second.client->type = Job::CLIENT_REMOVE;
						it->second.type = Job::TASK_REMOVE;
					}
					if (ret > 0)//Client is done
					{
						it->second.client->type = Job::READY_TO_WRITE;
						it->second.type = Job::TASK_REMOVE;
						FD_SET(it->second.client->fd, &copy_writefds);
					}
				}
			}
		}
		for (iterator it = this->jobs.begin(); it != this->jobs.end(); it++)
		{
			if (FD_ISSET(it->first, &copy_writefds))
			{
				job = &it->second;
				if (job->type == Job::WAIT_FOR_DELETING)
				{
					if (remove(job->_getRequest()._uri.c_str()) < 0)
 					{
 						if (errno == EACCES)
 							job->set_xxx_response(job->correct_config, 403);
 						else
 							job->set_xxx_response(job->correct_config, 404);
 					}
 					else
 					{
 						if (DEBUG == 1)
 							std::cerr << CLRS_RED <<  "FILE DELETED \'" << CLRS_MAG << job->_getRequest()._uri << CLRS_RED << "\'" << CLRS_reset << std::endl;
 						job->set_xxx_response(job->correct_config, 204);
 					}
					job->type = Job::READY_TO_WRITE;
				}
				else if (job->type == Job::READY_TO_WRITE)
				{
					this->betterClientResponse(job);
				}
				else if (job->type == Job::WRITING)
				{
					this->postHandler(job);
				}
				else if ( job->type == Job::READY_TO_CGI)
					this->do_cgi(job, &copy_writefds);
			}
		}

		std::map<int, Job>::iterator it = this->jobs.begin();
		while(it != this->jobs.end())
		{
			if (it->second.type == Job::TASK_REMOVE)
			{
				this->closeConnection(it, NULL);
				this->jobs.erase(it++);
			}
			else if (it->second.type == Job::CLIENT_REMOVE)
			{
				this->closeConnection(it, "client");
				this->jobs.erase(it++);
			}
			else
			{
				++it;
			}
		}

	}
}

/* Private Main Methods */
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
void					Webserv::closeConnection(iterator &it, const char *connectionClosedBy)
{
	if (DEBUG == 1 && connectionClosedBy)
			std::cerr << CLRS_RED << "server : connection closed by " << connectionClosedBy << " " << it->first << CLRS_reset << std::endl;

	FD_CLR(it->first, &this->fds);
	close(it->first);
}

void 					Webserv::do_cgi(Job *job, fd_set *copy_writefds)
{
	int ret;
	bool post;
	bool get;
	char *body;
	size_t bodyVec_size;
	std::string extension;
	std::string path;
	pid_t pid;
	int fd_out[2];
	int fd_in[2];

	Request &req = job->_getRequest();
	std::vector<unsigned char>	&bodyVector = req._body;
	bodyVector.push_back('\0');
	body = reinterpret_cast<char*>(&bodyVector[0]);
	bodyVec_size = bodyVector.size();
	extension = req._uri.substr(req._uri.find_last_of("."));
	path = job->correct_config.get_cgi().find(extension)->second;
	post = req.is_method_post();
	get = req.is_method_get();
	ret = 0;

	if (DEBUG == 1)
	{
		std::stringstream ss;
		ss << CLRS_BLU;
		ss << "server : >> [cgi: " << path << "] ";
		ss << "[client: " << job->fd << "] ";
		ss << CLRS_reset;
		std::cerr << ss.str() << std::endl;
	}


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
	else if (get == true || post == true)
	{
		ret = 0;
		job->_getResponse().set_status_code(200);
		while (!ret)
			ret = job->fileReader(fd_out[0]);
	}
	else
	{
		std::cerr << "Removing client in " << __FILE__ << ":" << __LINE__ << std::endl;
		job->type = Job::CLIENT_REMOVE;
	}
	close(fd_out[0]);

	job->type = Job::READY_TO_WRITE;
	FD_SET(job->fd, copy_writefds);
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
	ret = setsockopt(socketFD, SOL_SOCKET, SO_NOSIGPIPE, &options, sizeof(options));
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

void	Webserv::betterClientResponse(Job *job)
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

void Webserv::reset(Job *job)
{
	job->clear();
	this->jobs[job->fd].clear();
	this->jobs[job->fd].type = Job::READY_TO_READ;
}

int Webserv::requestRead(iterator &it, fd_set *fds, fd_set *wr, fd_set *rd)
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

int Webserv::setupNewJobs(Job *job, Job::JOB_TYPE type)
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
		if (this->createCGIJobs(job) == 0)
			return (0);
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

int Webserv::createReadingJobs(Job *job)
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

int Webserv::createWritingJobs(Job *job)
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

int Webserv::createCGIJobs(Job *job)
{
	std::string extension;
	Request &req = job->_getRequest();

	extension = req._uri.substr(req._uri.find_last_of("."));
	std::string &path = job->correct_config.get_cgi().find(extension)->second;
	if (access(path.c_str(), F_OK) != 0) // File doesn't exist
	{
		job->set_xxx_response(job->correct_config, 404);
		return (0);
	}
	return (1);
}

int Webserv::openFileForReading(std::string const &path)
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

int Webserv::readFile(int fd, std::string const &uri, Response &resp)
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

int Webserv::openFileForWriting(std::string const &path)
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

void Webserv::getCorrectConfigForJob(Job *job, std::string &newpath)
{
	ServerConfiguration &ref = this->get_correct_server_configuration(job);
	this->create_correct_configfile(job->request, ref, job->correct_config, newpath);
}

int Webserv::isError(Job *job, Request &request) // change name
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

void Webserv::postHandler(Job *job)
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

int Webserv::autoindex_module(Job *job, Configuration &config)
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
