/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/26 16:15:16 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/09/07 17:32:03 by bdekonin      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include "ServerConfiguration.hpp"
# include <netinet/in.h> // in_port_t

class Server
{
	public:
		/* Constructor  */
		Server()
		: _socket(0), _hostname(NULL), _port(0)
		{
		}
		Server(int socket, char *hostname, in_port_t port)
		: _socket(socket), _hostname(hostname), _port(port)
		{
			this->_configuration = std::vector<ServerConfiguration>();
		}
		Server(int socket, char *hostname, in_port_t port, ServerConfiguration const &config)
		: _socket(socket), _hostname(hostname), _port(port)
		{
			this->_configuration = std::vector<ServerConfiguration>();
			this->push_back(config);
			// this->_configuration.push_back(config);
			(void)config;
		}
		
		/* Destructor */
		virtual ~Server()
		{
			// free (this->_hostname);
			// this->_configuration.clear();
			// close(this->_socket); // TODO CHECK IF NESSECARY
			// TODO FREE CONFIGATION???
		}

		/* Copy constructor */
		Server(const Server &src)
		{
			*this = src;
		}

		/* Operation overload = */
		Server& operator = (const Server& e)
		{
			this->_socket = e.get_socket();
			this->_port = e.get_port();
			this->_hostname = e.get_hostname();
			this->_configuration = e.get_configurations();
			return *this;
		}

		// Getters
		int get_socket() const
		{
			return this->_socket;
		}
		int get_port() const
		{
			return this->_port;
		}
		char *get_hostname() const
		{
			return this->_hostname;
		}
		std::vector<ServerConfiguration> &get_configurations()
		{
			return this->_configuration;
		}
		const std::vector<ServerConfiguration> &get_configurations() const
		{
			return this->_configuration;
		}

		// Setters
		void set_socket(int socket)
		{
			this->_socket = socket;
		}
		void set_port(int port)
		{
			this->_port = port;
		}
		void set_hostname(const char *hostname)
		{
			this->_hostname = (char*)hostname;
		}
		void set_configurations(const std::vector<ServerConfiguration> configs)
		{
			this->_configuration = configs;
		}

		// Methods
		Server &push_back (const ServerConfiguration& val)
		{
			this->_configuration.push_back(ServerConfiguration(val)); // TODO OR JUST VAL?
			return *this;
		}
		ServerConfiguration &operator[](int index)
		{
			return this->_configuration[index];
		}
		
	private:
		int									_socket; // Server socket
		char *								_hostname; // Hostname to listen on
		in_port_t							_port; // Port to listen on
		std::vector<ServerConfiguration>	_configuration; // Configurations. Used to check if a client is allowed to connect. using the host request header. 0 is always first used.
};

std::ostream&	operator<<(std::ostream& out, const Server& c)
{
	out << "Socket: " << c.get_socket() << "\t(hostname:port): [" << c.get_hostname() << ":" << c.get_port() << "]" << std::endl;
	out << "Locations: " << c.get_configurations().size() << std::endl;
	for (size_t i = 0; i < c.get_configurations().size(); i++)
	{
		for (size_t loc_i = 0; loc_i < c.get_configurations()[i].get_locations().size(); loc_i++)
		{
			out << loc_i + 1 << " - " << c.get_configurations()[i].get_locations()[loc_i] << std::endl << std::endl;
		}
	}
	return out;
}

std::ostream&	operator<<(std::ostream& out, const Server *c)
{
	// out << c->get_socket() << "\t" << c->get_hostname() << ":" << c->get_port() << std::endl;
	out << "Socket: " << c->get_socket() << "\t(hostname:port): [" << c->get_hostname() << ":" << c->get_port() << "]" << std::endl;
	out << "Locations: " << c->get_configurations().size() << std::endl;
	for (size_t i = 0; i < c->get_configurations().size(); i++)
	{
		for (size_t loc_i = 0; loc_i < c->get_configurations()[i].get_locations().size(); loc_i++)
		{
			out << loc_i + 1 << " - " << c->get_configurations()[i].get_locations()[loc_i] << std::endl << std::endl;
		}
	}
	return out;
}

#endif // SERVER_HPP