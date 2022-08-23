/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Webserv.hpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/22 22:57:55 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/08/22 23:00:54 by bdekonin      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include "Configuration.hpp" // Base Class
# include "ServerConfiguration.hpp" // Derived from Configuration
# include "LocationConfiguration.hpp" // Derived from Configuration

# include "Parser.hpp"
class Webserv
{
	public:
		/* Constructor  */
		Webserv();

		/* Destructor */
		virtual ~Webserv();

		/* Copy constructor */
		Webserv(const Webserv&);

		/* Operation overload = */
		Webserv& operator = (const Webserv& e);

		// Methods
		// ...
	private:
		// ...
};

#endif // WEBSERV_HPP