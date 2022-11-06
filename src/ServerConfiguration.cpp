/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ServerConfiguration.cpp                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: rkieboom <rkieboom@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/11/05 14:53:23 by rkieboom      #+#    #+#                 */
/*   Updated: 2022/11/05 14:56:17 by rkieboom      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ServerConfiguration.hpp"

ServerConfiguration::ServerConfiguration()
: Configuration()
{
	
}

/* Destructor */
ServerConfiguration::~ServerConfiguration()
{
	this->clean();
}

/* Copy constructor */
ServerConfiguration::ServerConfiguration(const ServerConfiguration &src)
: Configuration()
{
	*this = src;
}

/* Operation overload = */
ServerConfiguration& ServerConfiguration::operator = (const ServerConfiguration &src)
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
void ServerConfiguration::clean()
{
	Configuration::clear();
	this->_ports.clear();
	this->_names.clear();
	this->_locations.clear();
}

// Setters
void ServerConfiguration::set_listen(std::string &s)
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
			host_port_pair pair = std::make_pair(std::string(""), ft_stoi(h_and_p[0]));
			// this->_ports[ft_stoi(h_and_p[0])] = "";
			this->_ports.push_back(pair);
		}
		else if (h_and_p.size() == 2)
		{
			host_port_pair pair = std::make_pair(h_and_p[0], ft_stoi(h_and_p[1]));
			// this->_ports[ft_stoi(h_and_p[1])] = h_and_p[0];
			this->_ports.push_back(pair);
		}
		else
			throw std::runtime_error("Error: port number is not valid");
		h_and_p.clear();
	}
}
void ServerConfiguration::set_server_names(std::string &s)
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

size_t	ServerConfiguration::getMatchLength(const std::string& URI, const std::string& locationPath)
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

LocationConfiguration *ServerConfiguration::get_location_by_uri(std::string &uri)
{
	std::string locations_path;
	LocationConfiguration *best_match = NULL;
	size_t best_match_length = 0;
	size_t matchlength;

	for (size_t i = 0; i < this->_locations.size(); i++)
	{
		locations_path = this->_locations[i].get_path();

		// size_t matchlength = getMatchLength(uri, locations_path);
		matchlength = this->match_length_with_without_backslash(uri, locations_path);
		if (matchlength > best_match_length)
		{
			best_match = &this->_locations[i];
			best_match_length = matchlength;
		}
	}
	return best_match;
}

size_t ServerConfiguration::match_length_with_without_backslash(std::string const &uri, std::string &l_path)
{
	// size_t one;
	// size_t two;

	// std::string l_path_bslash = l_path;
	// // if (*l_path.begin() != '/')
	// // 	l_path_bslash.insert(l_path_bslash.begin(), '/');
	// // if (*l_path.rbegin() != '/')
	// // 	l_path_bslash.push_back('/');

	// one = getMatchLength(uri, l_path);
	// two = getMatchLength(uri, l_path_bslash);

	// return (one > two ? one : two);
	return getMatchLength(uri, l_path);
}

// Getters
std::vector<std::pair<std::string, size_t> >		&ServerConfiguration::get_listen()
{
	return this->_ports;
}
const std::vector<std::pair<std::string, size_t> >	&ServerConfiguration::get_listen() const
{
	return this->_ports;
}

std::vector<std::string>							&ServerConfiguration::get_server_names()
{
	return this->_names;
}
const std::vector<std::string>						&ServerConfiguration::get_server_names() const
{
	return this->_names;
}

std::vector<LocationConfiguration>					&ServerConfiguration::get_locations()
{
	return this->_locations;
}
const std::vector<LocationConfiguration>			&ServerConfiguration::get_locations() const
{
	return this->_locations;
}
