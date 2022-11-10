/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Job.cpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/11/03 21:52:53 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/11/10 14:52:01 by bdekonin      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Job.hpp"

#include <unistd.h>
#include <arpa/inet.h>
# include "../inc/Request.hpp"

/* Constructors */
Job::Job()
: bytes_sent(0), request(Request()), response(Response()), correct_config(Configuration()), client(NULL)
{
}
Job::Job(int type, int fd, Server *server, Job *client)
: bytes_sent(0), type(type), fd(fd), server(server), request(Request()), response(Response()), correct_config(Configuration()), client(client)
{
}
Job::Job(const Job &src)
{
	*this = src;
}

/* Destructor */
Job::~Job()
{
}

/* Operation overload = */
Job& Job::operator = (const Job& e)
{
	this->type = e.type;
	this->fd = e.fd;
	this->server = e.server;
	this->request = e.request;
	this->response = e.response;
	this->correct_config = e.correct_config;

	this->client = e.client;
	return *this;
}

/* Public Member Functions */
bool 			Job::is_client()
{
	return (!(this->type == WAIT_FOR_CONNECTION));
}
Response		&Job::get_response()
{
	return this->response;
}
Request			&Job::get_request()
{
	return this->request;
}
void 			Job::clear()
{
	this->request.clear();
	this->response.clear();
	this->correct_config.clear();
	this->client = NULL;
	this->bytes_sent = 0;
}
void 			Job::parse_request(std::string &ConfigToChange_path) // config is config path file
{
	if (this->correct_config.get_return().size() != 0)
	{
		this->get_response().set_status_code(this->correct_config.get_return().begin()->first);
	}
	else
	{
		std::string path = this->request._uri;
		path.replace(path.find(ConfigToChange_path), ConfigToChange_path.size(), ConfigToChange_path);
		this->request._uri = this->correct_config.get_root() + path;
	}
}
void 			Job::generate_autoindex_add_respone(Configuration &config)
{
	std::string temp;

	// check root options

	int ret = get_root_options(this->request._uri.c_str());

	if (ret == -1 || ret == 1)
	{
		this->set_xxx_response(config, 404);
		return ;
	}


	if (this->generate_autoindex(this, this->request._uri, temp) == 0)
		this->get_response().set_body(temp.c_str(), temp.size(), 0);
	else
	{
		std::cerr << "Setting 500 in " << __FILE__ << ":" << __LINE__ << std::endl;
		this->set_xxx_response(config, 500);
	}
}
void 			Job::set_environment_variables()
{
	char buffer[1024];
	getcwd(buffer, 1024);

	std::string				cwd(buffer);
	cwd.push_back('/');
	
	Request &r = this->get_request();
	this->_set_environment_variable("CONTENT_TYPE", r.get_header("content-type").c_str());
	this->_set_environment_variable("CONTENT_LENGTH", std::to_string( r._content_length ).c_str());
	this->_set_environment_variable("GATEWAY_INTERFACE", "CGI/1.1");
	this->_set_environment_variable("HTTP_ACCEPT", r.get_header("accept").c_str());
	this->_set_environment_variable("HTTP_ACCEPT_CHARSET", r.get_header("accept-charset").c_str());
	this->_set_environment_variable("HTTP_ACCEPT_ENCODING", r.get_header("accept-encoding").c_str());
	this->_set_environment_variable("HTTP_ACCEPT_LANGUAGE", r.get_header("accept-language").c_str());
	this->_set_environment_variable("HTTP_CONNECTION", r.get_header("connection").c_str());
	this->_set_environment_variable("HTTP_HOST", r.get_header("host").c_str());
	this->_set_environment_variable("HTTP_USER_AGENT", r.get_header("user-agent").c_str());
	this->_set_environment_variable("PATH_INFO", r._uri.c_str());
	this->_set_environment_variable("QUERY_STRING", r._query_string.c_str()); // NO QUERY STRING
	this->_set_environment_variable("REDIRECT_STATUS", "true");
	this->_set_environment_variable("REMOTE_ADDR", this->address.c_str());
	this->_set_environment_variable("REQUEST_METHOD", r.method_to_s());
	this->_set_environment_variable("SCRIPT_FILENAME", cwd + r._uri);
	this->_set_environment_variable("SCRIPT_NAME", r._uri.c_str());
	this->_set_environment_variable("SERVER_PORT", std::to_string(this->server->get_port()).c_str());

	this->_set_environment_variable("SERVER_NAME", this->server->get_hostname());
	this->_set_environment_variable("SERVER_PROTOCOL", "HTTP/1.1");
	this->_set_environment_variable("SERVER_SOFTWARE", "Webserver Codam 1.0");
}
void 			Job::set_3xx_response(Configuration &config)
{
	this->response.set_3xx_response(config);
}
void 			Job::set_405_response(Configuration &config)
{
	this->response.set_405_response(config);
}
void 			Job::set_500_response(Configuration &config)
{
	this->response.set_xxx_response(config, 500);
}
void 			Job::set_xxx_response(Configuration &config, int code)
{
	if (code >= 300 && code <= 399)
		this->set_3xx_response(config);
	else if (code == 405)
		this->set_405_response(config);
	else if (code == 500)
		this->set_500_response(config);
	else
		this->response.set_xxx_response(config, code);
}

void Job::setType(int type)
{
	this->type = type;
}
void Job::setFd(int fd)
{
	this->fd = fd;
}
void Job::setServer(Server *server)
{
	this->server = server;
}
void Job::setClient(Job *client)
{
	this->client = client;
}
void Job::setAddress(struct sockaddr_in *address)
{
	char buffer[256];
	bzero(buffer, 256);
	inet_ntop(AF_INET, &address->sin_addr, buffer, 256);
	this->address.insert(0, buffer);
}
void Job::setAddress(std::string const &address)
{
	this->address = address;
}
/* Function that returns information about the file. See PATH_TYPE for more information. */
Job::PATH_TYPE Job::get_path_options(std::string const &uri)
{
	struct stat	sb;
	int			ret;
	int			returnstat;

	ret = stat(uri.c_str(), &sb);
	if (ret < 0)
		return this->NOT_FOUND; // NOT FOUND
	if (S_ISDIR(sb.st_mode) && uri[uri.size() - 1] == '/')
	{
		returnstat = sb.st_mode & (S_IRUSR | S_IRGRP | S_IROTH);
		if (returnstat == 0)
			return this->NO_PERMISSIONS; // NO PERMISSIONS
		else
			return this->DIRECTORY; // DIRECTORY
	}
	else if (ret == 0)
	{
		returnstat = sb.st_mode & (S_IRUSR | S_IRGRP | S_IROTH);
		if (returnstat == 0)
			return this->NO_PERMISSIONS; // NO PERMISSIONS
		else if (S_ISDIR(sb.st_mode) == false)
			return this->FILE_FOUND; // FILE FOUND
		else
			return this->NOT_FOUND;
	}
	else // should never go here but if its error
		return this->NOT_FOUND; // NOT FOUND
}
Job::PATH_TYPE Job::get_path_options()
{
	return this->get_path_options(this->request._uri);
}

void	Job::_set_environment_variable(const char *name, const char *value)
{
	if (setenv(name, value, 1) < 0)
		exit(EXIT_FAILURE);
}
void	Job::_set_environment_variable(const char *name, std::string &value)
{
	this->_set_environment_variable(name, value.c_str());
}
void	Job::_set_environment_variable(const char *name, std::string value)
{
	this->_set_environment_variable(name, value.c_str());
}
int Job::generate_autoindex(Job *job, std::string &uri, std::string &body)
{
	body = "<html>\r\n<head>\r\n<title>Index of " + job->get_request().get_unedited_uri() + "</title>\r\n</head>\r\n<body>\r\n<h1>Index of " + job->get_request().get_unedited_uri() + "</h1>\r\n<hr>\r\n<pre>\r\n";
	std::string		endBody = "\r\n</pre>\r\n<hr>\r\n</body>\r\n</html>\r\n";

	DIR *dir;
	struct dirent *diread;
	std::string name;
	struct stat sb;

	job->get_response().set_status_code(200);
	job->get_response().set_default_headers("html");
	if ((dir = opendir(job->get_request()._uri.c_str())) != NULL)
	{
		while ((diread = readdir(dir)) != NULL)
		{
			name.append(uri);
			name.append(diread->d_name);

			if (lstat(name.c_str(), &sb) == -1)
			{
				perror("lstat");
				exit(EXIT_FAILURE);
			}
			body += create_autoindex_line(job->get_request().get_unedited_uri() + diread->d_name, diread->d_name, sb.st_ctime, diread->d_reclen, S_ISREG(sb.st_mode)) + "<br>";
			name.clear();
		}
		closedir(dir);
		body.append(endBody);
		return (0);
	}
	closedir(dir);
	body.clear();
	return (1);
}

const char *Job::job_type_to_char()
{
	return this->job_type_to_char((JOB_TYPE)this->type);
}

const char *Job::job_type_to_char(JOB_TYPE type)
{
	switch (type)
	{
		case WAIT_FOR_CONNECTION: return "SERVER: WAIT_FOR_CONNECTION";
		case WAIT_FOR_READING: return "CLIENT: WAIT_FOR_READING";
		case WAIT_FOR_WRITING: return "CLIENT: WAIT_FOR_WRITING";
		case WAIT_FOR_DELETING: return "CLIENT: WAIT_FOR_DELETING";
		case WAIT_FOR_CGIING: return "CLIENT: WAIT_FOR_CGIING";
		case READY_TO_READ: return "CLIENT: READY_TO_READ";
		case READY_TO_WRITE: return "CLIENT: READY_TO_WRITE";
		case READY_TO_DELETE: return "CLIENT: READY_TO_DELETE";
		case READY_TO_CGI: return "CLIENT: READY_TO_CGI";
		case READING: return "TASK: READING";
		case WRITING: return "TASK: WRITING";
		case DELETING: return "TASK: DELETING";
		case CGIING: return "TASK: CGIING";
		case CLIENT_REMOVE: return "CLIENT: CLIENT_REMOVE";
		case TASK_REMOVE: return "TASK: TASK_REMOVE";
		case CLIENT_RESPONSE: return "CLIENT: CLIENT_RESPONSE";
		case CLIENT_READ: return "CLIENT: CLIENT_READ";
		case FILE_WRITE: return "TASK: FILE_WRITE";
		case FILE_READ: return "TASK: FILE_READ";
		case CGI_WRITE: return "TASK: CGI_WRITE";
		case CGI_READ: return "TASK: CGI_READ";
	}
	return "UNKNOWN";
}

bool Job::isClient() const 
{
	return (!this->isTask() && !this->isServer());
}

bool Job::isTask() const 
{
	return (
	this->type == Job::READING ||
	this->type == Job::WRITING ||
	this->type == Job::DELETING ||
	this->type == Job::CGIING
);
}

bool Job::isServer() const 
{
	return this->type == Job::WAIT_FOR_CONNECTION;
}

Request &Job::_getRequest()
{
	if (this->isClient()) // If it is the user then return main otherwise return reference
		return this->request;
	if (this->client)
		return this->client->get_request();
	throw std::runtime_error("Job::_getRequest() - No request found");
}

Response		&Job::_getResponse()
{
	if (this->isClient()) // If it is the user then return main otherwise return reference
		return this->response;
	if (this->client)
		return this->client->get_response();
	throw std::runtime_error("Job::_getResponse() - No request found");
}

int Job::fileReader(int fd)
{
	Response &res = this->_getResponse();
	char *pointer = NULL;
	char buf[4096 + 1];
	int ret, pos = 0;
	
	bzero(buf, 4096 + 1);

	ret = read(fd, buf, 4096);
	if (ret < 0)
	{
		this->bytes_sent = 0;
		return (-1);
	}
	if (ret == 0)
	{
		this->bytes_sent = 0;
		return (1);
	}
	this->bytes_sent += ret;
		pointer = ft_strnstr(buf, "\r\n\r\n", ret);
		if (pointer != NULL)
			pos = ft_strnstr(buf, "\r\n\r\n", ret) - buf;
		else
			pos = 0;
		if (pointer != NULL && pos > 0)
			res.set_header(std::string(buf, pos));
	res.set_body(buf, ret);
	return (0);
}

// std::ostream&	operator<<(std::ostream& out, const Job &c)
// {
// 	if (c.type == 0)
// 		out << "Job:\nType: " << "WAIT_FOR_CONNECTION" << "\nFD: " << c.fd << std::endl << std::endl;
// 	else if (c.type == 1)
// 		out << "Job:\nType: " << "CLIENT_RESPONSE" << "\nFD: " << c.fd << std::endl << std::endl;
// 	else if (c.type == 2)
// 		out << "Job:\nType: " << "CLIENT_READ" << "\nFD: " << c.fd << std::endl << std::endl;
// 	else if (c.type == 3)
// 		out << "Job:\nType: " << "FILE_WRITE" << "\nFD: " << c.fd << std::endl << std::endl;
// 	else if (c.type == 4)
// 		out << "Job:\nType: " << "FILE_READ" << "\nFD: " << c.fd << std::endl << std::endl;
// 	else if (c.type == 5)
// 		out << "Job:\nType: " << "CGI_WRITE" << "\nFD: " << c.fd << std::endl << std::endl;
// 	else if (c.type == 6)
// 		out << "Job:\nType: " << "CGI_READ" << "\nFD: " << c.fd << std::endl << std::endl;
// 	else
// 		out << "Job:\nType: " << "UNKNOWN" << "\nFD: " << c.fd << std::endl << std::endl;
// 	out << "Server:\n" << c.server << std::endl;
// 	out << "User:\n" << c.user << std::endl;
// 	return out;
// }

// std::ostream&	operator<<(std::ostream& out, const Job *c)
// {
// 	if (c->type == 0)
// 		out << "Job:\nType: " << "WAIT_FOR_CONNECTION" << "\nFD: " << c->fd << std::endl << std::endl;
// 	else if (c->type == 1)
// 		out << "Job:\nType: " << "CLIENT_RESPONSE" << "\nFD: " << c->fd << std::endl << std::endl;
// 	else if (c->type == 2)
// 		out << "Job:\nType: " << "CLIENT_READ" << "\nFD: " << c->fd << std::endl << std::endl;
// 	else if (c->type == 3)
// 		out << "Job:\nType: " << "FILE_WRITE" << "\nFD: " << c->fd << std::endl << std::endl;
// 	else if (c->type == 4)
// 		out << "Job:\nType: " << "FILE_READ" << "\nFD: " << c->fd << std::endl << std::endl;
// 	else if (c->type == 5)
// 		out << "Job:\nType: " << "CGI_WRITE" << "\nFD: " << c->fd << std::endl << std::endl;
// 	else if (c->type == 6)
// 		out << "Job:\nType: " << "CGI_READ" << "\nFD: " << c->fd << std::endl << std::endl;
// 	else
// 		out << "Job:\nType: " << "UNKNOWN" << "\nFD: " << c->fd << std::endl << std::endl;
// 	// out << "Server:\n\t" << c->server << std::endl;
// 	out << "User:\n" << c->user << std::endl;
// 	out << "request:\n" << c->request << std::endl;
// 	return out;
// }