/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Configuration.hpp                                  :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/20 22:03:45 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/08/23 11:50:03 by bdekonin      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGURATION_HPP
# define CONFIGURATION_HPP

# include "utils.hpp"

# include <iostream>
# include <string>
# include <vector>
# include <map>

# define whitespaces " \v\t\n"
# define forbidden_characters "\'\"|&;<>`$(){}[]*?#~"

void	split(const std::string& str, const char* delims, std::vector<std::string>& out);

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
			this->clear();
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

			// std::cout << "s: " << s << std::endl;

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
		
		// Getters
		std::map<size_t, std::string>				&get_error_page()
		{
			return this->_error_page;
		}
		const std::map<size_t, std::string> 		&get_error_page() const
		{
			return this->_error_page;
		}

		size_t										get_client_max_body_size() 
		{
			return this->_client_max_body_size;
		}
		const size_t								get_client_max_body_size() const
		{
			return this->_client_max_body_size;
		}

		bool										get_methods(size_t request)
		{
			return this->_methods[request];
		}
		const bool									get_methods(size_t request) const
		{
			return this->_methods[request];
		}

		std::map<size_t, std::string> 				&get_return() 
		{
			return this->_return;
		}
		const std::map<size_t, std::string> 		&get_return() const
		{
			return this->_return;
		}

		std::string 								&get_root()
		{
			return this->_root;
		}
		const std::string 							&get_root() const
		{
			return this->_root;
		}

		bool										get_autoindex()
		{
			return this->_autoindex;
		}
		const bool									get_autoindex() const
		{
			return this->_autoindex;
		}

		std::vector<std::string> 					&get_index()
		{
			return this->_index;
		}
		const std::vector<std::string> 				&get_index() const
		{
			return this->_index;
		}

		std::map<std::string, std::string> 			&get_cgi() 
		{
			return this->_cgi;
		}
		const std::map<std::string, std::string>	&get_cgi() const
		{
			return this->_cgi;
		}
	protected:
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

inline std::ostream&	operator<<(std::ostream& out, const Configuration& c)
{
	{
		out << "Error page: " << std::endl;
		for (std::map<size_t, std::string>::const_iterator it = c.get_error_page().begin(); it != c.get_error_page().end(); it++)
			out << "\t" << it->first << " " << it->second << std::endl;
		if (c.get_error_page().size() == 0)
			out << "\t" << "None" << std::endl;
	}
	{
		out << "Client max body size:\n\t" << c.get_client_max_body_size() << std::endl;
	}
	{
		out << "Methods:\n";
		if (c.get_methods(0) == true)
			out << "\tGET\n";
		if (c.get_methods(1) == true)
			out << "\tPOST\n";
		if (c.get_methods(2) == true)
			out << "\tDELETE\n";
		if (c.get_methods(0) == false && c.get_methods(1) == false && c.get_methods(2) == false)
			out << "\tNone\n";
	}
	{
		out << "Return:" << std::endl;
		for (std::map<size_t, std::string>::const_iterator it = c.get_return().begin(); it != c.get_return().end(); it++)
			out << "\t" << it->first << " " << it->second << std::endl;
		if (c.get_return().size() == 0)
			out << "\t" << "None" << std::endl;
	}
	{
		if (c.get_root() == "")
			out << "Root:\n\tNone" << std::endl;
		else
			out << "Root:\n\t" << c.get_root() << std::endl;
	}
	{
		if (c.get_autoindex() == true)
			out << "Autoindex:\n\tTrue" << std::endl;
		else
			out << "Autoindex:\n\tFalse" << std::endl;
	}
	{
		out << "Index:" << std::endl;
		for (size_t i = 0; i < c.get_index().size(); i++)
		{
			if (i == 0)
				out << "\t" << c.get_index()[i];
			else
				out <<  " -> " << c.get_index()[i];
		}
		if (c.get_index().size() == 0)
			out << "\t" << "None";
		out << std::endl;
	}
	{
		out << "Cgi:" << std::endl;
		for (std::map<std::string, std::string>::const_iterator it = c.get_cgi().begin(); it != c.get_cgi().end(); it++)
			out << "\t" << it->first << " " << it->second << std::endl;
		if (c.get_cgi().size() == 0)
			out << "\t" << "None" << std::endl;
	}
	return out;
}

#endif // CONFIGURATION_HPP