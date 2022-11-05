/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   LocationConfiguration.cpp                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: rkieboom <rkieboom@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/11/04 21:45:26 by rkieboom      #+#    #+#                 */
/*   Updated: 2022/11/04 21:50:17 by rkieboom      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/LocationConfiguration.hpp"

LocationConfiguration::LocationConfiguration()
{
	
}

LocationConfiguration::LocationConfiguration(std::string &s)
: _path(s)
{
	
}

LocationConfiguration::~LocationConfiguration()
{
	this->clear();
}

LocationConfiguration::LocationConfiguration(const LocationConfiguration *src)
{
	*this = *src;
}


LocationConfiguration::LocationConfiguration(const LocationConfiguration &src)
{
	*this = src;
}

LocationConfiguration& LocationConfiguration::operator = (const LocationConfiguration& src)
{
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
	this->_path = src.get_path(); // TODO check if this is correct
	return *this;
}

// Methods
void LocationConfiguration::clean()
{
	Configuration::clear();
	this->_path.clear();
}

// Setters
void LocationConfiguration::set_path(std::string &s)
{
	this->_path = s;
}
void LocationConfiguration::set_path(std::string const &s)
{
	this->_path = s;
}

// Getters
std::string			&LocationConfiguration::get_path()
{
	return (this->_path);
}
const std::string	&LocationConfiguration::get_path() const
{
	return (this->_path);
}