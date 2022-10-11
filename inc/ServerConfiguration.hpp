/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ServerConfiguration.hpp                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/20 21:18:36 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/10/11 13:31:13 by bdekonin      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIGURATION_HPP
# define SERVERCONFIGURATION_HPP

# include "Configuration.hpp"
# include "LocationConfiguration.hpp"
# include <iostream>

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
			this->clean();
		}

		/* Copy constructor */
		ServerConfiguration(const ServerConfiguration &src)
		: Configuration()
		{
			*this = src;
		}

		/* Operation overload = */
		ServerConfiguration& operator = (const ServerConfiguration &src)
		{
			this->_ports = src.get_listen();
			this->_names = src.get_server_names();
			this->_locations = src.get_locations();

			this->_error_page = src.get_error_page();
			this->_client_max_body_size = src.get_client_max_body_size();
			this->_methods[0] = src.get_methods(0);
			this->_methods[1] = src.get_methods(1);
			this->_methods[2] = src.get_methods(2);
			this->_return = src.get_return();
			this->_root = src.get_root();
			this->_autoindex = src.get_autoindex();
			this->_index = src.get_index();
			this->_cgi = src.get_cgi();
			return *this;
		}

		// Methods
		void clean()
		{
			Configuration::clear();
			this->_ports.clear();
			this->_names.clear();
			this->_locations.clear();
		}
		
		// Setters
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

		size_t	getMatchLength(const std::string& URI, const std::string& locationPath)
		{
			size_t	matchLength = 0;

			if (locationPath.size() > URI.size())
				return 0;
			for (size_t i = 0; i < locationPath.size(); i++)
			{
				if (locationPath[i] != URI[i])
					return 0;
				matchLength++;
			}
			return matchLength;
		}
		
		LocationConfiguration *get_location_by_uri(std::string &uri)
		{
			std::string locations_path;
			LocationConfiguration *best_match = nullptr;
			size_t best_match_length = 0;

			for (size_t i = 0; i < this->_locations.size(); i++)
			{
				locations_path = this->_locations[i].get_path();

				size_t matchlength = getMatchLength(uri, locations_path);
				if (matchlength > best_match_length)
				{
					best_match = &this->_locations[i];
					best_match_length = matchlength;
				}
			}
			return best_match;
		}

		// Getters
		std::vector<std::pair<std::string, size_t> >		&get_listen()
		{
			return this->_ports;
		}
		const std::vector<std::pair<std::string, size_t> >	&get_listen() const
		{
			return this->_ports;
		}

		std::vector<std::string>							&get_server_names()
		{
			return this->_names;
		}
		const std::vector<std::string>						&get_server_names() const
		{
			return this->_names;
		}

		std::vector<LocationConfiguration>					&get_locations()
		{
			return this->_locations;
		}
		const std::vector<LocationConfiguration>			&get_locations() const
		{
			return this->_locations;
		}
	private:
		std::vector<std::pair<std::string, size_t> > 	_ports;
		std::vector<std::string>						_names; // server names
		std::vector<LocationConfiguration> 				_locations; // location path, location config
};

std::ostream&	operator<<(std::ostream& out, const ServerConfiguration& c)
{
	{
		out << "port:<host(ip):" << std::endl;
		// for (std::map<size_t, std::string>::const_iterator it = c._ports.begin(); it != c._ports.end(); it++)
		// 	out  << "\t" << it->second << ":" << it->first << std::endl;
		for (size_t i = 0; i < c.get_listen().size(); i++)
			out  << "\t" << c.get_listen().at(i).first << ":" << c.get_listen().at(i).second << std::endl;
	}
	{
		out << "server names:" << std::endl;
		for (size_t i = 0; i < c.get_server_names().size(); i++)
			out << "\t" << c.get_server_names().at(i) << std::endl;
	}
	{
		out << static_cast<const Configuration&>(c);
	}
	{
		if (c.get_locations().size() > 0)
		{
			out << "                    locations:                    " << std::endl;
			out << "--------------------------------------------------" << std::endl;
			for (size_t i = 0; i < c.get_locations().size(); i++)
			{
				out << c.get_locations()[i];
				out << "--------------------------------------------------" << std::endl;
			}
		}
		else
			out << "locations:\n\tNone" << std::endl;
	}
	return (out);
}

#endif // SERVERCONFIGURATION_HPP