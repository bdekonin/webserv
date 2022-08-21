/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ServerConfiguration.hpp                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/20 21:18:36 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/08/21 23:01:14 by bdekonin      ########   odam.nl         */
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
	private:
		typedef std::pair<std::string, size_t>		host_port_pair;
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
		void set_listen(std::string &s)
		{
			std::vector<std::string> v; // vector to store the split string
			std::vector<std::string> h_and_p; // vector to store the host and port
			this->remove_semicolen(s);
			this->has_forbidden_charachters(s);
			
			split(s, whitespaces, v);
			

			if (v.size() == 0)
				throw std::runtime_error("Error: port number is not valid");
			for (size_t i = 0; i < v.size(); i++)
			{
				split(v[i], ":", h_and_p);
				// TODO check if the port is valid and not already used
				if (h_and_p.size() == 1 && h_and_p[0].find(".") != std::string::npos) // There is a lone ip address and will listen to default port 80
				{
					host_port_pair pair = std::make_pair(h_and_p[0], 0);
					this->_ports.push_back(pair);
				}
				else if (h_and_p.size() == 1)
				{
					host_port_pair pair = std::make_pair(std::string(""), std::stoi(h_and_p[0]));
					// this->_ports[std::stoi(h_and_p[0])] = "";
					this->_ports.push_back(pair);
				}
				else if (h_and_p.size() == 2)
				{
					host_port_pair pair = std::make_pair(h_and_p[0], std::stoi(h_and_p[1]));
					// this->_ports[std::stoi(h_and_p[1])] = h_and_p[0];
					this->_ports.push_back(pair);
				}
				else
					throw std::runtime_error("Error: port number is not valid");
				h_and_p.clear();
			}
		}
		void set_server_names(std::string &s)
		{
			std::vector<std::string> v;

			this->remove_semicolen(s);
			this->has_forbidden_charachters(s);

			split(s, whitespaces, v);

			if (v.size() == 0)
				throw std::runtime_error("Error: missing server name");

			for (size_t i = 0; i < v.size(); i++)
			{
				this->_names.push_back(v[i]);
			}
		}
		
		// Getters
		std::vector<std::pair<std::string, size_t> > get_listen()
		{
			return this->_ports;
		}
		const std::vector<std::pair<std::string, size_t> > get_listen() const
		{
			return this->_ports;
		}
		std::vector<std::string> get_server_names()
		{
			return this->_names;
		}
		const std::vector<std::string> get_server_names() const
		{
			return this->_names;
		}
		std::vector<LocationConfiguration> get_locations()
		{
			return this->_locations;
		}
		const std::vector<LocationConfiguration> get_locations() const
		{
			return this->_locations;
		}
	public:
		std::vector<std::pair<std::string, size_t> > _ports;
		std::vector<std::string>			_names; // server names
		std::vector<LocationConfiguration> 	_locations; // location path, location config
};

std::ostream&	operator<<(std::ostream& out, const ServerConfiguration& c)
{
	{
		out << "port:<host(ip):" << std::endl;
		// for (std::map<size_t, std::string>::const_iterator it = c._ports.begin(); it != c._ports.end(); it++)
		// 	out  << "\t" << it->second << ":" << it->first << std::endl;
		for (size_t i = 0; i < c._ports.size(); i++)
			out  << "\t" << c._ports[i].first << ":" << c._ports[i].second << std::endl;
	}
	{
		out << "server names:" << std::endl;
		for (size_t i = 0; i < c._names.size(); i++)
			out << "\t" <<  c._names[i] << std::endl;
	}
	{
		out << static_cast<const Configuration&>(c);
	}
	{
		if (c._locations.size() > 0)
		{
			out << "\n                    locations:                    " << std::endl;
			for (size_t i = 0; i < c._locations.size(); i++)
			{
				out << "__________________________________________________" << std::endl;
				out << c._locations[i] << std::endl;
				out << "__________________________________________________" << std::endl;
			}
		}
		else
			out << "locations:\n\tNone" << std::endl;
	}

	
	return (out);
}

#endif // SERVERCONFIGURATION_HPP