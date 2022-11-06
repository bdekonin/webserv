/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/26 16:15:16 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/11/06 12:10:56 by rkieboom      ########   odam.nl         */
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
		Server();
		Server(int socket, char *hostname, in_port_t port);
		Server(int socket, char *hostname, in_port_t port, ServerConfiguration const &config);

		void set(int socket, char *hostname, in_port_t port, ServerConfiguration const &config);
		
		/* Destructor */
		virtual ~Server();

		/* Copy constructor */
		Server(const Server &src);

		/* Operation overload = */
		Server& operator = (const Server& e);

		// Getters
		int get_socket() const;
		int get_port() const;
		char *get_hostname() const;
		std::vector<ServerConfiguration> &get_configurations();
		const std::vector<ServerConfiguration> &get_configurations() const;

		// Setters
		void set_socket(int socket);
		void set_port(int port);
		void set_hostname(const char *hostname);
		void set_configurations(const std::vector<ServerConfiguration> configs);

		// Methods
		Server &push_back (const ServerConfiguration& val);
		ServerConfiguration &operator[](int index);
		
	private:
		int									_socket; // Server socket
		char *								_hostname; // Hostname to listen on
		in_port_t							_port; // Port to listen on
		std::vector<ServerConfiguration>	_configuration; // Configurations. Used to check if a client is allowed to connect. using the host request header. 0 is always first used.
};

// std::ostream&	operator<<(std::ostream& out, const Server& c)
// {
// 	out << "Socket: " << c.get_socket() << "\t(hostname:port): [" << c.get_hostname() << ":" << c.get_port() << "]" << std::endl;
// 	out << "Locations: " << c.get_configurations().size() << std::endl;
// 	for (size_t i = 0; i < c.get_configurations().size(); i++)
// 	{
// 		for (size_t loc_i = 0; loc_i < c.get_configurations()[i].get_locations().size(); loc_i++)
// 		{
// 			out << loc_i + 1 << " - " << c.get_configurations()[i].get_locations()[loc_i] << std::endl << std::endl;
// 		}
// 	}
// 	return out;
// }

// std::ostream&	operator<<(std::ostream& out, const Server *c)
// {
// 	// out << c->get_socket() << "\t" << c->get_hostname() << ":" << c->get_port() << std::endl;
// 	out << "Socket: " << c->get_socket() << "\t(hostname:port): [" << c->get_hostname() << ":" << c->get_port() << "]" << std::endl;
// 	out << "Locations: " << c->get_configurations().size() << std::endl;
// 	for (size_t i = 0; i < c->get_configurations().size(); i++)
// 	{
// 		for (size_t loc_i = 0; loc_i < c->get_configurations()[i].get_locations().size(); loc_i++)
// 		{
// 			out << loc_i + 1 << " - " << c->get_configurations()[i].get_locations()[loc_i] << std::endl << std::endl;
// 		}
// 	}
// 	return out;
// }

#endif // SERVER_HPP