/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   User.hpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/09/05 14:46:37 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/09/05 18:09:11 by bdekonin      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef USER_HPP
# define USER_HPP

#include "Request.hpp"

class User
{
	public:
		/* Constructor  */
		User()
		: _fd(-1), _request(Request())
		{
		}
		User(int socketFD, struct sockaddr_in *address_info)
		: _fd(socketFD), _address_info(address_info), _request(Request())
		{
			
		}

		/* Destructor */
		virtual ~User()
		{
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
			this->_request = e._request;
			this->_address_info = e._address_info;
			return *this;
		}

		// Methods
		void set_fd(int fd)
		{
			this->_fd = fd;
		}
		void set_request(Request request)
		{
			this->_request = request;
		}
		void set_address_info(struct sockaddr_in *address_info)
		{
			this->_address_info = address_info;
		}
		
		int get_fd()
		{
			return this->_fd;
		}
		Request				&get_request()
		{
			return this->_request;
		}
		struct sockaddr_in *get_address()
		{
			return this->_address_info;
		}

	private:
		int		_fd;
		Request _request;

		struct sockaddr_in *_address_info;
		
};

#endif // USER_HPP