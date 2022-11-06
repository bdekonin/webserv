/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   User.hpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/09/05 14:46:37 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/11/05 15:02:31 by rkieboom      ########   odam.nl         */
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
		User();
		User(int socketFD, struct sockaddr_in *address_info);

		/* Destructor */
		virtual ~User();

		/* Copy constructor */
		User(const User &src);

		/* Operation overload = */
		User& operator = (const User& e);

		// Methods
		void set_fd(int fd);
		void set_address_info(struct sockaddr_in *address_info);
		std::string			&get_address();
		
		int get_fd();
		struct sockaddr_in *get_address_info();

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