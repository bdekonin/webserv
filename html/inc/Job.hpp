/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Job.hpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/31 16:44:20 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/09/14 15:13:48 by bdekonin      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef JOB_HPP
# define JOB_HPP

# include "Server.hpp"
# include "Request.hpp"
# include "Response.hpp"
# include "User.hpp"

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
		: request(Request()), response(Response())
		{
		}
		Job(int type, int fd, Server *server, User *user)
		: type(type), fd(fd), server(server), user(user), request(Request()), response(Response()), cgi(NULL)
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
			this->cgi = e.cgi;
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
		}

	public:
		int				type;
		int				fd;
		Server			*server;
		User			*user; // TODO: change to client class | SAME AS CLIENT
		Request			request;
		Response		response;
		void			*cgi;// TODO change to CGI Object

		void set_3xx_response(Configuration &config)
		{
			this->response.set_3xx_response(config);
		}
		void set_405_response(Configuration &config)
		{
			this->response.set_405_response(config);
		}
		void set_404_response(Configuration &config)
		{
			this->response.set_404_response(config);
		}

		void set_client_response(fd_set *copy_writefds) // fd_sets to write fd_set. and set client_response
		{
			this->type = CLIENT_RESPONSE;
			FD_SET(this->fd, copy_writefds);
		}

		char get_path_options(std::string uri, std::string root) // TODO FOR DEBUG OPEN IN CHROME https://github.com/bdekonin/minishell/blob/master/src/execve.c#:~:text=stat.h%3E-,int%09%09%09validate_file(char%20*filepath),%7D,-void%09%09signal_exec(
		{
			std::string &path = uri;

			if (root.size() > 0)
				path = root + path;

			struct stat	sb;
			int			ret;
			int			returnstat;

			VARR(path);

			ret = stat(path.c_str(), &sb);
			if (ret < 0)
				return '0'; // NOT FOUND
			if (S_ISDIR(sb.st_mode))
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
				else
					return ('F'); // FILE FOUND
			}
			else // should never go here but if its error
				return '0'; // NOT FOUND
		}
		char get_path_options(std::string root)
		{
			return this->get_path_options(this->request._uri, root);
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