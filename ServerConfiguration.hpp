/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ServerConfiguration.hpp                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/20 21:18:36 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/08/20 22:04:02 by bdekonin      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIGURATION_HPP
# define SERVERCONFIGURATION_HPP

# include <string>
# include <vector>
# include <map>

class ServerConfiguration
{
	public:
		/* Constructor  */
		ServerConfiguration();

		/* Destructor */
		virtual ~ServerConfiguration();

		/* Copy constructor */
		ServerConfiguration(const ServerConfiguration&);

		/* Operation overload = */
		ServerConfiguration& operator = (const ServerConfiguration& e);

		// Methods
	protected:
		std::map<std::string, size_t>		_ports; // <host(ip), port>
		std::vector<std::string>			_names; // server names
		// std::map<std::string, locationConfiguration> _locations; // location path, location config
};

#endif // SERVERCONFIGURATION_HPP