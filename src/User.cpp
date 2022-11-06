/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   User.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: rkieboom <rkieboom@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/11/05 15:01:07 by rkieboom      #+#    #+#                 */
/*   Updated: 2022/11/05 15:01:56 by rkieboom      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/User.hpp"

User::User()
: _fd(-1), _address("")
{
}
User::User(int socketFD, struct sockaddr_in *address_info)
: _fd(socketFD), _address_info(address_info)
{
	char buffer[1024];
	bzero(buffer, 1024);
	inet_ntop(AF_INET, &address_info->sin_addr, buffer, 1024);
	this->_address.insert(0, buffer);
}

/* Destructor */
User::~User()
{
	delete this->_address_info;
}

/* Copy constructor */
User::User(const User &src)
{
	*this = src;
}

/* Operation overload = */
User& User::operator = (const User& e)
{
	this->_fd = e._fd;
	this->_address_info = e._address_info;
	this->_address = e._address;
	return *this;
}

// Methods
void User::set_fd(int fd)
{
	this->_fd = fd;
}
void User::set_address_info(struct sockaddr_in *address_info)
{
	this->_address_info = address_info;
}
std::string			&User::get_address()
{
	return this->_address;
}

int User::get_fd()
{
	return this->_fd;
}
struct sockaddr_in *User::get_address_info()
{
	return this->_address_info;
}
