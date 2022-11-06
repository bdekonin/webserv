/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Configuration.cpp                                  :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/11/03 21:00:19 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/11/06 10:47:16 by bdekonin      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Configuration.hpp"

/* Constructors */
Configuration::Configuration()
: _error_page(), _client_max_body_size(1 * CLIENT_MAX_BODY_SIZE_MULTIPLIER), _return(), _root(""), _index(), _cgi()
{
	
	this->_methods[0] = true; // ??
	this->_methods[1] = true; // ??
	this->_methods[2] = true; // ??
	this->_autoindex = false;
}
Configuration::Configuration(const Configuration &src)
{
	*this = src;
}

/* Destructor */
Configuration::~Configuration()
{
	this->clear();
}

/* Operator overloads */
Configuration& Configuration::operator = (const Configuration& src)
{
	this->_error_page = src._error_page;
	this->_client_max_body_size = src._client_max_body_size;
	this->_methods[0] = src._methods[0];
	this->_methods[1] = src._methods[1];
	this->_methods[2] = src._methods[2];
	this->_return = src._return;
	this->_root = src._root;
	this->_autoindex = src._autoindex;
	this->_index = src._index;
	this->_cgi = src._cgi;

	this->_isSet = src._isSet;
	return *this;
}

/* Public Member Functions */
void Configuration::clear() // clear all data
{
	this->_isSet.clear();
	this->_error_page.clear();
	this->_client_max_body_size = 1 * CLIENT_MAX_BODY_SIZE_MULTIPLIER;
	this->_methods[0] = false; // GET
	this->_methods[1] = false; // POST
	this->_methods[2] = false; // DELETE
	this->_return.clear();
	this->_root.clear();
	this->_autoindex = false;
	this->_index.clear();
	this->_cgi.clear();
}
void Configuration::combine_two_locations(Configuration &src) // copying <src> to *this | src is previous config level
{
	if (this->_error_page.empty())
		this->_error_page = src._error_page;
	if (this->_root.empty())
		this->_root = src._root;
}
bool Configuration::is_method_allowed(std::string const &method) const 
{
	return this->is_method_allowed(method.c_str());
}
#include <cstring>
bool Configuration::is_method_allowed(const char *method) const
{
	if (strcmp(method, "GET") == 0)
		return (this->_methods[0]);
	else if (strcmp(method, "POST") == 0)
		return (this->_methods[1]);
	else if (strcmp(method, "DELETE") == 0)
		return (this->_methods[2]);
	else
		return false; // only GET POST DELETE
}

/* Setters */
void Configuration::set_error_page(std::string &s)
{
	std::vector<std::string> v;

	this->remove_semicolen(s);

	split(s, whitespaces, v);
	
	if (v.size() != 2)
		throw std::runtime_error("config: error_page has invalid number of arguments");

	this->has_forbidden_charachters(v[0]);
	this->has_forbidden_charachters(v[1]);

	this->_error_page[ft_stoi(v[0])] = v[1];
	this->_isSet["error_page"] = true;
}
void Configuration::set_client_max_body_size(std::string &s)
{
	std::vector<std::string> v;

	this->remove_semicolen(s);
	split(s, whitespaces, v);

	if (v.size() != 1)
		throw std::runtime_error("config: client_max_body_size has invalid number of arguments");

	if (*v[0].rbegin() != 'm')
		throw std::runtime_error("config: client_max_body_size doesnt specify a size in megabytes (m)");

	// v[0].pop_back();
	v[0].erase(v[0].size() - 1);

	this->has_forbidden_charachters(v[0]);
	
	this->_client_max_body_size = ft_stoi(v[0]);
	this->_client_max_body_size *= CLIENT_MAX_BODY_SIZE_MULTIPLIER;
	this->_isSet["client_max_body_size"] = true;
}
void Configuration::set_methods(std::string &s)
{
	if (this->_isSet["methods"] == false)
	{
		this->_methods[0] = false; // GET
		this->_methods[1] = false; // POST
		this->_methods[2] = false; // DELETE
	}

	std::vector<std::string> v;

	this->remove_semicolen(s);
	split(s, whitespaces, v);
	if (v.size() == 0)
		throw std::runtime_error("config: methods has invalid number of arguments");
	
	for (size_t i = 0; i < v.size(); i++)
	{
		this->has_forbidden_charachters(v[i]);

		for (size_t j  = 0; j < v[i].size(); j++)
			v[i][j] = std::toupper(v[i][j]);

		if (v[i] == "GET")
			this->_methods[0] = true;
		else if (v[i] == "POST")
			this->_methods[1] = true;
		else if (v[i] == "DELETE")
			this->_methods[2] = true;
		else
			throw std::runtime_error("config: methods has a forbidden argument");
	}
	this->_isSet["methods"] = true;
}
void Configuration::set_methods(size_t i, bool b)
{
	this->_methods[i] = b;
}
void Configuration::set_return(std::string &s)
{
	std::vector<std::string> v;
	std::string path;

	this->remove_semicolen(s);

	split(s, whitespaces, v);
	if (v.size() != 2)
		throw std::runtime_error("config: return has invalid number of arguments");

	this->has_forbidden_charachters(v[0]);
	this->has_forbidden_charachters(v[1]);
	
	this->_return[ft_stoi(v[0])] = v[1];
	this->_isSet["return"] = true;
}
void Configuration::set_root(std::string &s)
{
	std::vector<std::string> v;
	this->remove_semicolen(s);
	
	split(s, whitespaces, v);
	if (v.size() != 1)
		throw std::runtime_error("config: root has invalid number of arguments");
	
	this->has_forbidden_charachters(v[0]);

	this->_root = v[0];
	this->_isSet["root"] = true;

	int ret = get_root_options(this->_root.c_str());

	if (ret == -1)
		throw std::runtime_error("config: root doesnt exist: " + this->_root);
	else if (ret == 1)
		throw std::runtime_error("config: root is not a directory: " + this->_root);
	else
		return ;
}
void Configuration::set_autoindex(std::string &s)
{
	std::vector<std::string> v;
	this->remove_semicolen(s);
	
	split(s, whitespaces, v);
	if (v.size() != 1)
		throw std::runtime_error("config: autoindex has invalid number of arguments");
	
	this->has_forbidden_charachters(v[0]);

	if (v[0] == "on")
		this->_autoindex = true;
	else if (v[0] == "off")
		this->_autoindex = false;
	else
		throw std::runtime_error("config: autoindex has invalid argument");
	this->_isSet["autoindex"] = true;
}
void Configuration::set_index(std::string &s)
{
	std::vector<std::string> v;
	this->remove_semicolen(s);

	split(s, whitespaces, v);
	if (v.size() == 0)
		throw std::runtime_error("config: index has invalid number of arguments");

	for (size_t i = 0; i < v.size(); i++)
	{
		this->has_forbidden_charachters(v[i]);
		this->_index.push_back(v[i]);
	}
	this->_isSet["index"] = true;
}
void Configuration::set_cgi(std::string &s)
{
	std::vector<std::string> v;
	std::string path;

	this->remove_semicolen(s);

	split(s, whitespaces, v);
	if (v.size() != 2)
		throw std::runtime_error("config: cgi has invalid number of arguments");

	this->has_forbidden_charachters(v[0]);
	this->has_forbidden_charachters(v[1]);

	this->_cgi[v[0]] = v[1];
	this->_isSet["cgi"] = true;
}

/* Getters */
std::map<size_t, std::string>				&Configuration::get_error_page()
{
	return this->_error_page;
}
const std::map<size_t, std::string> 		&Configuration::get_error_page() const
{
	return this->_error_page;
}
size_t										Configuration::get_client_max_body_size() 
{
	return this->_client_max_body_size;
}
size_t										Configuration::get_client_max_body_size() const
{
	return this->_client_max_body_size;
}
bool										Configuration::get_methods(size_t request)
{
	return this->_methods[request];
}
bool										Configuration::get_methods(size_t request) const
{
	return this->_methods[request];
}
std::map<size_t, std::string> 				&Configuration::get_return() 
{
	return this->_return;
}
const std::map<size_t, std::string> 		&Configuration::get_return() const
{
	return this->_return;
}
std::string 								&Configuration::get_root()
{
	return this->_root;
}
const std::string 							&Configuration::get_root() const
{
	return this->_root;
}
bool										Configuration::get_autoindex()
{
	return this->_autoindex;
}
bool										Configuration::get_autoindex() const
{
	return this->_autoindex;
}
std::vector<std::string> 					&Configuration::get_index()
{
	return this->_index;
}
const std::vector<std::string> 				&Configuration::get_index() const
{
	return this->_index;
}
std::map<std::string, std::string> 			&Configuration::get_cgi() 
{
	return this->_cgi;
}
const std::map<std::string, std::string>	&Configuration::get_cgi() const
{
	return this->_cgi;
}


/* Private Member Functions */
void Configuration::remove_semicolen(std::string &s) // removes the semicolen at the end if it is still there
{
	if (s[s.length() - 1] == ';')
		s.erase(s.length() - 1);
}
void Configuration::has_forbidden_charachters(std::string &s)
{
	if (s.find_first_of(forbidden_characters) != std::string::npos)
		throw std::runtime_error("config: forbidden characters in string");
}