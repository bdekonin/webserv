/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   User.hpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/09/05 14:46:37 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/11/04 11:59:39 by bdekonin      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef USER_HPP
# define USER_HPP

#include "Request.hpp"
#include <arpa/inet.h>

class User
{
	public:
		/* Constructor  */
		User()
		: _fd(-1), _address("")
		{
		}
		User(int socketFD, struct sockaddr_in *address_info)
		: _fd(socketFD), _address_info(address_info)
		{
			char buffer[1024];
			bzero(buffer, 1024);
			inet_ntop(AF_INET, &address_info->sin_addr, buffer, 1024);
			this->_address.insert(0, buffer);
		}

		/* Destructor */
		virtual ~User()
		{
			delete this->_address_info;
		}

		/* Copy constructor */
		User(const User &src)
		{
			*this = src;
		}

		/* Operation overload = */
		User& operator = (const User& e)
		{
			this->_fd = e._fd;
			this->_address_info = e._address_info;
			this->_address = e._address;
			return *this;
		}

		// Methods
		void set_fd(int fd)
		{
			this->_fd = fd;
		}
		void set_address_info(struct sockaddr_in *address_info)
		{
			this->_address_info = address_info;
		}
		std::string			&get_address()
		{
			return this->_address;
		}
		
		int get_fd()
		{
			return this->_fd;
		}
		struct sockaddr_in *get_address_info()
		{
			return this->_address_info;
		}

	private:
		int		_fd;

		struct sockaddr_in *_address_info;
		std::string _address;
};

// std::ostream&	operator<<(std::ostream& out, User &c)
// {
// 	out << "Fd:\n" << c.get_fd() << std::endl;
// 	out << "Address:\n" << c.get_address() << std::endl;

// 	return out;
// }
// std::ostream&	operator<<(std::ostream& out, User *c)
// {
// 	out << "Fd:\n" << c->get_fd() << std::endl;
// 	out << "Address:\n" << c->get_address() << std::endl;
// 	out << "&Address:\n" << c->get_address_info() << std::endl;
	
// 	return out;
// }

#endif // USER_HPP