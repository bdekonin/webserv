/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Job.hpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/31 16:44:20 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/09/11 20:10:48 by bdekonin      ########   odam.nl         */
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

char *g_define_names[7] { // TODO REMOVE LATER MAYBE
	"WAIT_FOR_CONNECTION",
	"CLIENT_RESPONSE",
	"CLIENT_READ",
	"FILE_WRITE",
	"FILE_READ",
	"CGI_WRITE",
	"CGI_READ"
};

class Job
{
	public:
		/* Constructor  */
		Job()
		{
		}
		Job(int type, int fd, Server *server, User *user)
		: type(type), fd(fd), server(server), user(user), cgi(NULL)
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

	public:
		int				type;
		int				fd;
		Server			*server;
		User			*user; // TODO: change to client class | SAME AS CLIENT
		Request			*request;
		void			*cgi;// TODO change to CGI Object

		Response do_3xx_response(Configuration &config)
		{
			Response response;
			std::map<size_t, std::string> map = config.get_return();

			response.set_status_code(map.begin()->first);
			response.set_header("Location", map.begin()->second);
			response.set_header("Server", "Webserv (Luke & Bob) 1.0");

			return response;
		}
		Response do_405_response(Configuration &config)
		{
			Response response;
			std::string allowed_methods;
			
			std::vector<std::string> methods;
			if (config.get_methods(0) == true)
				methods.push_back("GET");
			if (config.get_methods(1) == true)
				methods.push_back("POST");
			if (config.get_methods(2) == true)
				methods.push_back("DELETE");
			for (size_t i = 0; i < methods.size(); i++)
			{
				allowed_methods += methods[i];
				if (i != methods.size() - 1)
					allowed_methods += ", ";
			}
			response.set_status_code(405);
			response.set_header("Allow", allowed_methods);
			response.set_header("Server", "Webserv (Luke & Bob) 1.0");

			response.set_header("Content-Type", "text/html");

			response.set_body("405 Method Not Allowed");
			response.set_header("Content-Length", std::to_string(response._body.size()));
			return response;
		}
		Response do_404_response(Configuration &config)
		{
			Response response;
			std::string allowed_methods;
			
			std::vector<std::string> methods;
			response.set_status_code(404);

			response.set_header("Server", "Webserv (Luke & Bob) 1.0");
			response.set_header("Content-Type", "text/html");

			response.set_body("404 Method Not Allowed");

			response.set_header("Content-Length", std::to_string(response._body.size()));
			return response;
		}
};

std::ostream&	operator<<(std::ostream& out, const Job &c)
{
	out << "Job: " << g_define_names[c.type] << " | " << c.fd << std::endl;

	out << "Server:\n" << c.server << std::endl;
	out << "User:\n" << c.user << std::endl;

	return out;
}
std::ostream&	operator<<(std::ostream& out, const Job *c)
{
	out << "Job:\nType: " << g_define_names[c->type] << "\nFD: " << c->fd << std::endl << std::endl;

	// out << "Server:\n\t" << c->server << std::endl;
	out << "User:\n" << c->user << std::endl;
	out << "request:\n" << c->request << std::endl;
	

	return out;
}

#endif // JOB_HPP