/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Job.hpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/31 16:44:20 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/10/06 14:38:21 by bdekonin      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef JOB_HPP
# define JOB_HPP

# include "Server.hpp"
# include "Request.hpp"
# include "Response.hpp"
# include "User.hpp"


# include <dirent.h>

# include <sys/stat.h> // stat

# define WAIT_FOR_CONNECTION 0 // READ | EVEN

# define CLIENT_RESPONSE 1 // WRITE | ODD
# define CLIENT_READ 2 // READ | EVEN

# define FILE_WRITE 3 // WRITE | ODD
# define FILE_READ 4 // READ | EVEN

# define CGI_WRITE 5 // WRITE | ODD
# define CGI_READ 6 // READ | EVEN




#define getString(n) #n
#define VARR(var) std::cerr << std::boolalpha << __LINE__ << ":\t" << getString(var) << " = [" <<  (var) << "]" << std::noboolalpha << std::endl;
#define PRINT(var) std::cout << var << std::endl


class Job
{
	public:
		/* Constructor  */
		Job()
		: request(Request()), response(Response()), correct_config(Configuration())
		{
		}
		Job(int type, int fd, Server *server, User *user)
		: type(type), fd(fd), server(server), user(user), request(Request()), response(Response()), cgi(NULL), correct_config(Configuration())
		{
		}

		/* Destructor */
		virtual ~Job()
		{
			// if (this->request)
			// 	delete request;
		}

		/* Copy constructor */
		Job(const Job &src)
		{
			*this = src;
		}

		/* Operation overload = */
		Job& operator = (const Job& e)
		{
			this->type = e.type;
			this->fd = e.fd;
			this->server = e.server;
			this->user = e.user;
			this->request = e.request;
			this->response = e.response;
			this->cgi = e.cgi;
			this->correct_config = e.correct_config;
			return *this;
		}

		bool is_read()
		{
			return (this->type % 2 == 0);
		}
		bool is_write()
		{
			return (this->type % 2 == 1);
		}
		bool is_server()
		{
			return (this->type == WAIT_FOR_CONNECTION);
		}
		bool is_client()
		{
			return (!this->is_server());
		}

		Response &get_response()
		{
			return this->response;
		}
		Request &get_request()
		{
			return this->request;
		}

		void clear() // clear the request and response
		{
			this->request.clear();
			this->response.clear();
			this->correct_config = Configuration();
			// delete this->user; // TODO MUST? DO???? CHECK WEBSERV.hpp 353
		}

		void parse_request(std::string &ConfigToChange_path)
		{
			// VAR(this->request._uri);
			// VAR(ConfigToChange_path);

			if (this->correct_config.get_return().size() != 0)
			{
				// this->set_3xx_response(this->correct_config);
				this->get_response().set_status_code(this->correct_config.get_return().begin()->first);
			}
			else
			{
				std::string path = this->request._uri;
				path.replace(path.find(ConfigToChange_path), ConfigToChange_path.size(), ConfigToChange_path);
				this->request._uri = this->correct_config.get_root() + path;
			}
		}

		void handle_file(fd_set *copy_writefds, Configuration &config)
		{
			std::string &uri = this->get_request()._uri;
			std::string extension = uri.substr(uri.find_last_of(".") + 1);

			if (extension == uri)
				extension = "";
			
			if (extension != "") // file has extension
			{
				if (config.get_cgi().find(extension) != config.get_cgi().end())
				{
					// DO CGI STUFF
					this->get_response().set_500_response(config);
					this->set_client_response(copy_writefds);
					return ;
				}
			}
			std::string path = this->get_request()._uri;
			std::ifstream in(path, std::ios::in);
			if (!in)
				this->get_response().set_500_response(config); // TODO check which error


			this->get_response().set_status_code(200);
			this->get_response().set_default_headers(extension);

			std::stringstream contents;
			contents << in.rdbuf();
			in.close();
			this->get_response().set_body(contents.str());

			this->get_response().set_content_length();
			// job->set_client_response(copy_writefds);
		}
	
		void generate_autoindex_add_respone(Configuration &config)
		{
			std::string temp;
			if (this->generate_autoindex(this, this->request._uri, temp) == 0)
			{
				this->get_response().set_body(temp);
				this->get_response().set_content_length();
			}
			else
				this->set_500_response(config);
		}
	
		const char *num_to_define_name(const int num)
		{
			if (num == 0)
				return ("WAIT_FOR_CONNECTION");
			if (num == 1)
				return ("CLIENT_RESPONSE");
			if (num == 2)
				return ("CLIENT_READ");
			if (num == 3)
				return ("FILE_WRITE");
			if (num == 4)
				return ("FILE_READ");
			if (num == 5)
				return ("CGI_WRITE");
			if (num == 6)
				return ("CGI_READ");
			return ("UNSUPPORTED");
		}
	
		void set_environment_variables()
		{
			char buffer[1024];
			getcwd(buffer, 1024);

			std::string				cwd = std::string(buffer) + '/';
			
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
			this->_set_environment_variable("QUERY_STRING", ""); // NO QUERY STRING
			this->_set_environment_variable("REDIRECT_STATUS", "true");
			this->_set_environment_variable("REMOTE_ADDR", this->user->get_address().c_str());
			this->_set_environment_variable("REQUEST_METHOD", r.method_to_s());
			this->_set_environment_variable("SCRIPT_FILENAME", cwd + r._uri);

			this->_set_environment_variable("SCRIPT_NAME", r._uri.c_str());
			this->_set_environment_variable("SERVER_PORT", std::to_string(this->server->get_port()).c_str());
			this->_set_environment_variable("SERVER_NAME", this->server->get_hostname());
			this->_set_environment_variable("SERVER_PROTOCOL", "HTTP/1.1");

			this->_set_environment_variable("SERVER_SOFTWARE", "Webserver Codam 1.0");

			
		}
	public:
		int				type;
		int				fd;
		Server			*server;
		User			*user; // TODO: change to client class | SAME AS CLIENT
		Request			request;
		Response		response;
		void			*cgi;// TODO change to CGI Object

		Configuration correct_config;

		void set_3xx_response(Configuration &config)
		{
			this->response.set_3xx_response(config, this->get_request()._uri);
		}
		void set_405_response(Configuration &config)
		{
			this->response.set_405_response(config);
		}
		void set_404_response(Configuration &config)
		{
			this->response.set_404_response(config);
		}
		void set_500_response(Configuration &config)
		{
			this->response.set_500_response(config);
		}

		void set_client_response(fd_set *copy_writefds) // fd_sets to write fd_set. and set client_response
		{
			this->type = CLIENT_RESPONSE;
			FD_SET(this->fd, copy_writefds);
		}

		char get_path_options(std::string uri) // TODO FOR DEBUG OPEN IN CHROME https://github.com/bdekonin/minishell/blob/master/src/execve.c#:~:text=stat.h%3E-,int%09%09%09validate_file(char%20*filepath),%7D,-void%09%09signal_exec(
		{
			std::string &path = uri;

			struct stat	sb;
			int			ret;
			int			returnstat;

			ret = stat(path.c_str(), &sb);
			if (ret < 0)
				return '0'; // NOT FOUND
			if (S_ISDIR(sb.st_mode) && uri[uri.size() - 1] == '/')
			{
				returnstat = sb.st_mode & S_IXUSR;
				if (returnstat == 0)
					return ('X'); // NO PERMISSIONS
				else
					return 'D'; // DIRECTORY
			}
			else if (ret == 0)
			{
				returnstat = sb.st_mode & S_IXUSR;
				if (returnstat == 0)
					return ('X'); // NO PERMISSIONS
				else if (S_ISDIR(sb.st_mode) == false)
					return ('F'); // FILE FOUND
				else
					return '0';
			}
			else // should never go here but if its error
				return '0'; // NOT FOUND
		}
		char get_path_options()
		{
			return this->get_path_options(this->request._uri);
		}

	private:
		void	_set_environment_variable(const char *name, const char *value)
		{
			// std::cout << "setenv: " << name << " = " << value << std::endl;
			if (setenv(name, value, 1) < 0)
				exit(EXIT_FAILURE);
		}
		void	_set_environment_variable(const char *name, std::string &value)
		{
			this->_set_environment_variable(name, value.c_str());
		}
		void	_set_environment_variable(const char *name, std::string value)
		{
			this->_set_environment_variable(name, value.c_str());
		}
		int generate_autoindex(Job *job, std::string &uri, std::string &body)
		{
			body = "<html>\r\n<head>\r\n<title>Index of " + job->get_request().get_unedited_uri() + "</title>\r\n</head>\r\n<body>\r\n<h1>Index of " + job->get_request().get_unedited_uri() + "</h1>\r\n<hr>\r\n<pre>\r\n";
			std::string		endBody = "\r\n</pre>\r\n<hr>\r\n</body>\r\n</html>\r\n";

			DIR *dir;
			struct dirent *diread;
			std::string name;
			struct stat sb;

			if ((dir = opendir(job->get_request()._uri.c_str())) != nullptr)
			{
				while ((diread = readdir(dir)) != nullptr)
				{
					name.append(job->get_request()._uri);
					name.append(diread->d_name);

					if (lstat(name.c_str(), &sb) == -1)
					{
						perror("lstat");
						exit(EXIT_FAILURE);
					}
					job->get_response().set_status_code(200);
					job->get_response().set_default_headers("html");
					// if (__APPLE__)
						body += create_autoindex_line(job->get_request().get_unedited_uri() + diread->d_name, diread->d_name, sb.st_ctim, diread->d_reclen, S_ISREG(sb.st_mode)) + "<br>";
					name.clear();
				}
				closedir(dir);
				body.append(endBody);
				return (0);
			}
			else
			{
				// TODO ERROR
				body.clear();
				return (1);
			}
		}
};

std::ostream&	operator<<(std::ostream& out, const Job &c)
{
	if (c.type == 0)
		out << "Job:\nType: " << "WAIT_FOR_CONNECTION" << "\nFD: " << c.fd << std::endl << std::endl;
	else if (c.type == 1)
		out << "Job:\nType: " << "CLIENT_RESPONSE" << "\nFD: " << c.fd << std::endl << std::endl;
	else if (c.type == 2)
		out << "Job:\nType: " << "CLIENT_READ" << "\nFD: " << c.fd << std::endl << std::endl;
	else if (c.type == 3)
		out << "Job:\nType: " << "FILE_WRITE" << "\nFD: " << c.fd << std::endl << std::endl;
	else if (c.type == 4)
		out << "Job:\nType: " << "FILE_READ" << "\nFD: " << c.fd << std::endl << std::endl;
	else if (c.type == 5)
		out << "Job:\nType: " << "CGI_WRITE" << "\nFD: " << c.fd << std::endl << std::endl;
	else if (c.type == 6)
		out << "Job:\nType: " << "CGI_READ" << "\nFD: " << c.fd << std::endl << std::endl;
	else
		out << "Job:\nType: " << "UNKNOWN" << "\nFD: " << c.fd << std::endl << std::endl;

	out << "Server:\n" << c.server << std::endl;
	out << "User:\n" << c.user << std::endl;

	return out;
}
std::ostream&	operator<<(std::ostream& out, const Job *c)
{
	if (c->type == 0)
		out << "Job:\nType: " << "WAIT_FOR_CONNECTION" << "\nFD: " << c->fd << std::endl << std::endl;
	else if (c->type == 1)
		out << "Job:\nType: " << "CLIENT_RESPONSE" << "\nFD: " << c->fd << std::endl << std::endl;
	else if (c->type == 2)
		out << "Job:\nType: " << "CLIENT_READ" << "\nFD: " << c->fd << std::endl << std::endl;
	else if (c->type == 3)
		out << "Job:\nType: " << "FILE_WRITE" << "\nFD: " << c->fd << std::endl << std::endl;
	else if (c->type == 4)
		out << "Job:\nType: " << "FILE_READ" << "\nFD: " << c->fd << std::endl << std::endl;
	else if (c->type == 5)
		out << "Job:\nType: " << "CGI_WRITE" << "\nFD: " << c->fd << std::endl << std::endl;
	else if (c->type == 6)
		out << "Job:\nType: " << "CGI_READ" << "\nFD: " << c->fd << std::endl << std::endl;
	else
		out << "Job:\nType: " << "UNKNOWN" << "\nFD: " << c->fd << std::endl << std::endl;
		
	// out << "Server:\n\t" << c->server << std::endl;
	out << "User:\n" << c->user << std::endl;
	out << "request:\n" << c->request << std::endl;
	

	return out;
}

#endif // JOB_HPP