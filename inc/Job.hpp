/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Job.hpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/31 16:44:20 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/10/28 20:50:08 by bdekonin      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef JOB_HPP
# define JOB_HPP

# include "Server.hpp"
# include "Request.hpp"
# include "Response.hpp"
# include "User.hpp"
# include <string>


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
		enum PATH_TYPE
		{
			NOT_FOUND = '0',
			NO_PERMISSIONS = 'X',
			DIRECTORY = 'D',
			FILE_FOUND = 'F',
		};

		/* Constructor  */
		Job()
		: request(Request()), response(Response()), correct_config(Configuration())
		{
		}
		Job(int type, int fd, Server *server, User *user)
		: type(type), fd(fd), server(server), user(user), request(Request()), response(Response()), correct_config(Configuration())
		{
		}

		/* Destructor */
		virtual ~Job()
		{
			// if (this->is_client() == true)
			// 	delete this->user;
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
			this->correct_config = e.correct_config;
			return *this;
		}

		/// @brief this function checks if this job is ready to read
		/// @return this returns true if the job is ready to read, false if not.
		bool is_read()
		{
			return (this->type % 2 == 0);
		}
		/// @brief this function checks if this job is ready to write
		/// @return this returns true if the job is ready to write, false if not.
		bool is_write()
		{
			return (this->type % 2 == 1);
		}
		/// @brief This function is used to check if the job is a server. a Server type is always 'WAIT_FOR_CONNECTION'.
		/// @return this returns true if the job is a server, false if not.
		bool is_server()
		{
			return (this->type == WAIT_FOR_CONNECTION);
		}
		/// @brief This function is used to check if the job is a client job. a Job type is never 'WAIT_FOR_CONNECTION'.
		/// @return this returns true if the job is a client job, false if not.
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

		/// @brief This clears the request and response of the job.
		void clear()
		{
			this->request.clear();
			this->response.clear();
			this->correct_config = Configuration();
		}

		/// @brief This function makes sure that when you access the dir 'post' and you have a root of 'www'. It will return 'www/post'.
		/// @param ConfigToChange_path 
		void parse_request(std::string &ConfigToChange_path)
		{
			if (this->correct_config.get_return().size() != 0)
				this->get_response().set_status_code(this->correct_config.get_return().begin()->first);
			else
			{
				std::string path = this->request._uri;
				path.replace(path.find(ConfigToChange_path), ConfigToChange_path.size(), ConfigToChange_path);
				this->request._uri = this->correct_config.get_root() + path;
			}
		}

		/// @brief This function reads from the file descriptor and saves the header in the request. and the body in the body.
		/// @param fd The file descriptor to read from.
		void handle_file(int fd)
		{
			std::string &uri = this->get_request()._uri;

			this->get_response().set_status_code(200);
			this->get_response().set_default_headers(uri.substr(uri.find_last_of(".") + 1));

			int ret, pos = 0;
			char *pointer = NULL;
			char buf[4096 + 1];
			
			bzero(buf, 4096 + 1);
			while ((ret = read(fd, buf, 4096)) > 0)
			{
				pointer = ft_strnstr(buf, "\r\n\r\n", ret);
				if (pointer != NULL)
					pos = ft_strnstr(buf, "\r\n\r\n", ret) - buf;
				else
					pos = 0;
				if (pointer != NULL && pos > 0)
					this->get_response().set_header(std::string(buf, pos));
				this->get_response().set_body(buf, ret, pos);
				if (ret < 4096)
					break;
				bzero(buf, 4096);
				pos = 0;
				pointer = NULL;
			}
		}

		/// @brief This function creates a list of all the files in the directory. 'autoindex' and stores it into the response body.
		/// @param config this is the config object. mainly used if there was something wrong in the config. e.g. malloc, read, etc
		void generate_autoindex_add_respone(Configuration &config)
		{
			std::string temp;
			if (this->generate_autoindex(this, this->request._uri, temp) == 0)
				this->get_response().set_body(temp.c_str(), temp.size());
			else
				this->set_xxx_response(config, 500);
		}
	
		/// @brief This function environment veriables for the CGI. This is alwasy called inside a fork otherwise it will overwrite the parent's environment. so also the shell that it got called from
		void set_environment_variables()
		{
			char buffer[1024];
			getcwd(buffer, 1024);

			std::string				cwd = std::string(buffer) + '/';
			
			Request &r = this->get_request();
			this->_set_environment_variable("CONTENT_TYPE", r.get_header("content-type").c_str());
			this->_set_environment_variable("CONTENT_LENGTH", SSTR( r._content_length ).c_str());
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
			this->_set_environment_variable("REMOTE_ADDR", this->user->get_address().c_str());
			this->_set_environment_variable("REQUEST_METHOD", r.method_to_s());
			this->_set_environment_variable("SCRIPT_FILENAME", cwd + r._uri);
			this->_set_environment_variable("SCRIPT_NAME", r._uri.c_str());
			this->_set_environment_variable("SERVER_PORT", SSTR(this->server->get_port()).c_str());

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
		Configuration correct_config;

		void set_3xx_response(Configuration &config)
		{
			this->response.set_3xx_response(config);
		}
		void set_405_response(Configuration &config)
		{
			this->response.set_405_response(config);
		}
		void set_500_response(Configuration &config)
		{
			this->set_xxx_response(config, 500);
		}

		/* Sets a response to @code with no special headers */
		void set_xxx_response(Configuration &config, int code)
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

		/// @brief This function gets called after you are done with it. It will FD_SET the fd to write and makes sure it will be writen
		/// @param copy_writefds this is the fd_set that will be used loop over with FD_ISSET
		void set_client_response(fd_set *copy_writefds) // fd_sets to write fd_set. and set client_response
		{
			this->type = CLIENT_RESPONSE;
			FD_SET(this->fd, copy_writefds);
		}


		/*
		** Function that returns information about the file. See PATH_TYPE for more information.
		*/
		/// @brief This function returns information about the file. See PATH_TYPE for more information. Used bdekonin minishell code for this.
		/// https://github.com/bdekonin/minishell/blob/master/src/execve.c#:~:text=stat.h%3E-,int%09%09%09validate_file(char%20*filepath),%7D,-void%09%09signal_exec(
		/// @param uri location of the file or directory
		/// @return PATH_TYPE
		PATH_TYPE get_path_options(std::string &uri)
		{
			std::string &path = uri;

			struct stat	sb;
			int			ret;
			int			returnstat;

			ret = stat(path.c_str(), &sb);
			if (ret < 0)
				return this->NOT_FOUND; // NOT FOUND
			if (S_ISDIR(sb.st_mode) && uri[uri.size() - 1] == '/')
			{
				returnstat = sb.st_mode & S_IXUSR;
				if (returnstat == 0)
					return this->NO_PERMISSIONS; // NO PERMISSIONS
				else
					return this->DIRECTORY; // DIRECTORY
			}
			else if (ret == 0)
			{
				returnstat = sb.st_mode & S_IXUSR;
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
		PATH_TYPE get_path_options()
		{
			return this->get_path_options(this->request._uri);
		}

	private:
		void	_set_environment_variable(const char *name, const char *value)
		{
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
			body.clear();
			return (1);
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