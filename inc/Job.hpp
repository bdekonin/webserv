/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Job.hpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/31 16:44:20 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/09/21 19:16:18 by bdekonin      ########   odam.nl         */
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
				VARR(this->request._uri);
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
					this->get_response().set_405_response(config); // TESTING
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