/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: rkieboom <rkieboom@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/11/05 14:49:10 by rkieboom      #+#    #+#                 */
/*   Updated: 2022/11/05 14:50:42 by rkieboom      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Server.hpp"

Server::Server()
: _socket(0), _hostname(NULL), _port(0)
{
}
Server::Server(int socket, char *hostname, in_port_t port)
: _socket(socket), _hostname(hostname), _port(port)
{
	this->_configuration = std::vector<ServerConfiguration>();
}
Server::Server(int socket, char *hostname, in_port_t port, ServerConfiguration const &config)
: _socket(socket), _hostname(hostname), _port(port)
{
	this->_configuration = std::vector<ServerConfiguration>();
	this->push_back(config);
	// this->_configuration.push_back(config);
	(void)config;
}

void Server::set(int socket, char *hostname, in_port_t port, ServerConfiguration const &config)
{
	this->_socket = socket;
	this->_hostname = hostname;
	this->_port = port;
	this->_configuration.push_back(config);
}

/* Destructor */
Server::~Server()
{
	// free (this->_hostname);
	// this->_configuration.clear();
	// close(this->_socket); // TODO CHECK IF NESSECARY
	// TODO FREE CONFIGATION???
}

/* Copy constructor */
Server::Server(const Server &src)
{
	*this = src;
}

/* Operation overload = */
Server& Server::operator = (const Server& e)
{
	this->_socket = e.get_socket();
	this->_port = e.get_port();
	this->_hostname = e.get_hostname();
	this->_configuration = e.get_configurations();
	return *this;
}

// Getters
int Server::get_socket() const
{
	return this->_socket;
}
int Server::get_port() const
{
	return this->_port;
}
char *Server::get_hostname() const
{
	return this->_hostname;
}
std::vector<ServerConfiguration> &Server::get_configurations()
{
	return this->_configuration;
}
const std::vector<ServerConfiguration> &Server::get_configurations() const
{
	return this->_configuration;
}

// Setters
void Server::set_socket(int socket)
{
	this->_socket = socket;
}
void Server::set_port(int port)
{
	this->_port = port;
}
void Server::set_hostname(const char *hostname)
{
	this->_hostname = (char*)hostname;
}
void Server::set_configurations(const std::vector<ServerConfiguration> configs)
{
	this->_configuration = configs;
}

// Methods
Server &Server::push_back (const ServerConfiguration& val)
{
	this->_configuration.push_back(ServerConfiguration(val)); // TODO OR JUST VAL?
	return *this;
}

ServerConfiguration &Server::operator[](int index)
{
	return this->_configuration[index];
}
