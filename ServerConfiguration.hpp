/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ServerConfiguration.hpp                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/20 21:18:36 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/08/21 01:04:26 by bdekonin      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIGURATION_HPP
# define SERVERCONFIGURATION_HPP

# include <string>
# include <vector>
# include <map>

# include "Configuration.hpp"
# include "LocationConfiguration.hpp"

class ServerConfiguration : public Configuration
{
	public:
		/* Constructor  */
		ServerConfiguration()
		: Configuration()
		{
			
		}

		/* Destructor */
		virtual ~ServerConfiguration()
		{
			
		}

		/* Copy constructor */
		ServerConfiguration(const ServerConfiguration &src)
		{
			*this = src;
		}

		/* Operation overload = */
		ServerConfiguration& operator = (const ServerConfiguration &src)
		{
			this->_ports = src._ports;
			this->_names = src._names;
			
			// this->_locations = src._locations;
			return *this;
		}

		// Methods
	public:
		std::map<std::string, size_t>		_ports; // <host(ip), port>
		std::vector<std::string>			_names; // server names
		std::map<std::string, LocationConfiguration> _locations; // location path, location config
};

#endif // SERVERCONFIGURATION_HPP