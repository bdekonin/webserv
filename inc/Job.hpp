/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Job.hpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/31 16:44:20 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/08/31 20:48:17 by bdekonin      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef JOB_HPP
# define JOB_HPP

# include "Server.hpp"

# define WAIT_FOR_CONNECTION 0 // READ | EVEN

# define CLIENT_RESPONSE 1 // WRITE | ODD
# define CLIENT_READ 2 // READ | EVEN

# define FILE_WRITE 3 // WRITE | ODD
# define FILE_READ 4 // READ | EVEN

# define CGI_WRITE 5 // WRITE | ODD
# define CGI_READ 6 // READ | EVEN


class Job
{
	public:
		/* Constructor  */
		Job()
		{
		}
		Job(int type, int fd, Server *server, void *client)
		: type(type), fd(fd), server(server), client(client), cgi(NULL)
		{
		}

		/* Destructor */
		virtual ~Job()
		{
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
			this->client = e.client;
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
		void			*client; // TODO: change to client class
		void			*cgi;// TODO change to CGI Object
};

#endif // JOB_HPP