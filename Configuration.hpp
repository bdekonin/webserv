/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Configuration.hpp                                  :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/20 22:03:45 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/08/21 01:08:03 by bdekonin      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGURATION_HPP
# define CONFIGURATION_HPP

#include <iostream>

# include <string>
# include <vector>
# include <map>

#define whitespaces " \v\t\n"
#define forbidden_characters "\'\"|&;<>`$(){}[]*?#~"

void	split(const std::string& str, const char* delims, std::vector<std::string>& out);
size_t		count(std::string str, char c);

class Configuration
{
	public:
		/* Constructor  */
		Configuration()
		: _error_page(), _client_max_body_size(0), _return(), _root(""), _index(), _cgi()
		{
			
			this->_methods[0] = false; // ??
			this->_methods[1] = false; // ??
			this->_methods[2] = false; // ??
			this->_autoindex = false;
		}

		/* Destructor */
		virtual ~Configuration()
		{
			// for (std::map<size_t, std::string>::iterator it = _error_page.begin(); it != _error_page.end(); it++)
			// 	std::cout << it->first << " " << it->second << std::endl;


			// if (this->_methods[0] == true || this->_methods[1] == true || this->_methods[2] == true)
			// {
			// 	std::cout << "GET: " << this->_methods[0] << std::endl;
			// 	std::cout << "POST: " << this->_methods[1] << std::endl;
			// 	std::cout << "DELETE: " << this->_methods[2] << std::endl;
			// }

			
			// if (this->_return.size() > 0)
			// 	std::cout << "Return: " << this->_return.begin()->first << " -> " << this->_return.begin()->second << std::endl;
			
			// if (this->_autoindex == true)
			// {
			// 	std::cout << "autoindex: " << this->_autoindex << std::endl;
			// }
			// if (this->_index.size() > 0)
			// {
			// 	std::cout << "index: ";
			// 	for (size_t i = 0; i < this->_index.size(); i++)
			// 		std::cout << this->_index[i] << " ";
			// 	std::cout << std::endl;
			// }

			// if (this->_cgi.size() > 0)
			// {
			// 	for (std::map<std::string, std::string>::iterator it = _cgi.begin(); it != _cgi.end(); it++)
			// 		std::cout << it->first << " " << it->second << std::endl;
			// }
		}

		/* Copy constructor */
		Configuration(const Configuration &src)
		{
			*this = src;
		}

		/* Operation overload = */
		Configuration& operator = (const Configuration& src)
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
			return *this;
		}

		// Methods
		void clear() // clear all data
		{
			this->_error_page.clear();
			this->_client_max_body_size = 0;
			this->_methods[0] = false; // GET
			this->_methods[1] = false; // POST
			this->_methods[2] = false; // DELETE
			this->_return.clear();
			this->_root.clear();
			this->_autoindex = false;
			this->_index.clear();
			this->_cgi.clear();
		}

		// Setters
		void set_error_page(std::string &s)
		{
			std::vector<std::string> v;

			this->remove_semicolen(s);

			split(s, whitespaces, v);
			if (v.size() != 2)
				throw std::runtime_error("config: error_page has invalid number of arguments");

			this->has_forbidden_charachters(v[0]);
			this->has_forbidden_charachters(v[1]);

			this->_error_page[std::stoi(v[0])] = v[1];
		}
		void set_client_max_body_size(std::string &s)
		{
			
		}
		void set_methods(std::string &s)
		{
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
		}
		void set_return(std::string &s)
		{
			std::vector<std::string> v;
			std::string path;

			this->remove_semicolen(s);

			split(s, whitespaces, v);
			if (v.size() != 2)
				throw std::runtime_error("config: return has invalid number of arguments");

			this->has_forbidden_charachters(v[0]);
			this->has_forbidden_charachters(v[1]);
			
			this->_return[std::stoi(v[0])] = v[1];
		}
		void set_root(std::string &s)
		{
			std::vector<std::string> v;
			this->remove_semicolen(s);
			
			split(s, whitespaces, v);
			if (v.size() != 1)
				throw std::runtime_error("config: root has invalid number of arguments");
			
			this->has_forbidden_charachters(v[0]);

			this->_root = v[0];
		}
		void set_autoindex(std::string &s)
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
		}
		void set_index(std::string &s)
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
		}
		void set_cgi(std::string &s)
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
		}
	public:
		std::map<size_t, std::string>		_error_page; // <error code, path>
		size_t								_client_max_body_size; // max size of body
		bool								_methods[3]; // GET, POST, DELETE
		std::map<size_t, std::string>		_return; // Redirection to ...
		std::string							_root; // root path
		bool								_autoindex; // defaults to false
		std::vector<std::string>			_index; // order of index files
		std::map<std::string, std::string>	_cgi; // path to cgi

		void remove_semicolen(std::string &s) // removes the semicolen at the end if it is still there
		{
			if (s[s.length() - 1] == ';')
				s.erase(s.length() - 1);
		}
		void has_forbidden_charachters(std::string &s)
		{
			if (s.find_first_of(forbidden_characters) != std::string::npos)
				throw std::runtime_error("config: forbidden characters in string");
		}
};

#endif // CONFIGURATION_HPP