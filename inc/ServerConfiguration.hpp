/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ServerConfiguration.hpp                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/20 21:18:36 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/11/06 12:14:16 by rkieboom      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIGURATION_HPP
# define SERVERCONFIGURATION_HPP

# include "Configuration.hpp"
# include "LocationConfiguration.hpp"
# include <iostream>


/// @brief This class is used to store everything in a 'server' block.
/// It inherits from the Configuration class. The difference between.
/// LocationConfiguration and ServerConfiguration is that the ServerConfiguration class has a vector of LocationConfiguration objects.
/// Everything inside a Configuration object can also exist in a ServerConfiguration object.
class ServerConfiguration : public Configuration
{
	private:
		typedef std::pair<std::string, size_t>		host_port_pair;
	public:
		/* Constructor  */
		ServerConfiguration();

		/* Destructor */
		virtual ~ServerConfiguration();

		/* Copy constructor */
		ServerConfiguration(const ServerConfiguration &src);

		/* Operation overload = */
		ServerConfiguration& operator = (const ServerConfiguration &src);

		// Methods
		void clean();
		
		// Setters
		void set_listen(std::string &s);
		void set_server_names(std::string &s);
		void set_server_names(std::string &s);
		size_t	getMatchLength(const std::string& URI, const std::string& locationPath);
		
		LocationConfiguration *get_location_by_uri(std::string &uri);

		size_t match_length_with_without_backslash(std::string const &uri, std::string &l_path);

		// Getters
		std::vector<std::pair<std::string, size_t> >		&get_listen();
		const std::vector<std::pair<std::string, size_t> >	&get_listen() const;

		std::vector<std::string>							&get_server_names();
		const std::vector<std::string>						&get_server_names() const;

		std::vector<LocationConfiguration>					&get_locations();
		const std::vector<LocationConfiguration>			&get_locations() const;
	private:
		std::vector<std::pair<std::string, size_t> > 	_ports;
		std::vector<std::string>						_names; // server names
		std::vector<LocationConfiguration> 				_locations; // location path, location config
};

// std::ostream&	operator<<(std::ostream& out, const ServerConfiguration& c)
// {
// 	{
// 		out << "port:<host(ip):" << std::endl;
// 		// for (std::map<size_t, std::string>::const_iterator it = c._ports.begin(); it != c._ports.end(); it++)
// 		// 	out  << "\t" << it->second << ":" << it->first << std::endl;
// 		for (size_t i = 0; i < c.get_listen().size(); i++)
// 			out  << "\t" << c.get_listen().at(i).first << ":" << c.get_listen().at(i).second << std::endl;
// 	}
// 	{
// 		out << "server names:" << std::endl;
// 		for (size_t i = 0; i < c.get_server_names().size(); i++)
// 			out << "\t" << c.get_server_names().at(i) << std::endl;
// 	}
// 	{
// 		out << static_cast<const Configuration&>(c);
// 	}
// 	{
// 		if (c.get_locations().size() > 0)
// 		{
// 			out << "                    locations:                    " << std::endl;
// 			out << "--------------------------------------------------" << std::endl;
// 			for (size_t i = 0; i < c.get_locations().size(); i++)
// 			{
// 				out << c.get_locations()[i];
// 				out << "--------------------------------------------------" << std::endl;
// 			}
// 		}
// 		else
// 			out << "locations:\n\tNone" << std::endl;
// 	}
// 	return (out);
// }

#endif // SERVERCONFIGURATION_HPP