/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Request.hpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/23 12:38:22 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/09/08 21:51:59 by bdekonin      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <string>
# include <vector>
# include <map>

# include <iostream>

/* Sources
** https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Transfer-Encoding
**
**	When Transfer-Encoding: chunked (\r\n)
**
**
*/

# include "utils.hpp"

class Request
{
	public:
		/* Constructor  */
		Request()
		: _method(""), _uri(""), _version("")
		{
			this->_headers_map = std::map<std::string, std::string>();
		}
		Request(std::string &buffer)
		{
			this->setup(buffer);
		}

		// Setup

		void setup(std::string &buffer)
		{
			std::vector<std::string> lines;
			split(buffer, "\r\n", lines);

						
			for (int i = 1; i < lines.size(); i++)
				this->set_header(lines[i]);

			// for (auto it = this->_headers_map.begin(); it != this->_headers_map.end(); it++)
			// {
			// 	std::cout << "[" << it->first << "]" << std::endl << "[" << it->second << "]" << std::endl << std::endl;
			// }

			std::vector<std::string> request_line;
			split(lines[0], " ", request_line);
			this->_method = request_line[0];
			this->_uri = request_line[1];
			this->_version = request_line[2];

			// std::cout << "lines[0]: " << lines[0] <<std::endl;
			// std::cout << "Method: " << this->_method << std::endl;
			// std::cout << "URI: " << this->_uri << std::endl;
			// std::cout << "Version: " << this->_version << std::endl;
		}

		/* Destructor */
		virtual ~Request()
		{
		}

		/* Copy constructor */
		Request(const Request *e)
		{
			*this = e;
		}

		/* Operation overload = */
		Request& operator = (const Request *e)
		{
			// std::cout << "Request operator = called" << std::endl;
			// std::cout << "e._headers_map.size(): " << e._headers_map.size() << std::endl;
			// this->_headers_map.clear();
			// this->_headers_map.insert(e._headers_map.begin(), e._headers_map.end());
			this->_method = e->_method;
			this->_uri = e->_uri;
			this->_version = e->_version;

			return *this;
		}

		// Methods
		void set_header(std::string const &header)
		{
			size_t pos;

			std::string identifier;
			std::string value;

			pos = header.find(":");
			if (pos == std::string::npos)
				return ; //  bad request?
			
			identifier = header.substr(0, pos);
			for (size_t i = 0; i < identifier.length(); i++)
				identifier[i] = std::tolower(identifier[i]);

			
			value = header.substr(pos + 1);
			
			identifier = ft_strtrim(identifier, whitespaces);
			value = ft_strtrim(value, whitespaces);

			// std::cout << "[" << identifier << "]\t[" << value << "]\n";
			this->_headers_map[identifier] = value;
		}
	public:
		std::map<std::string, std::string> _headers_map;

		std::string _method;
		std::string _uri;
		std::string _version;
};

std::ostream&	operator<<(std::ostream& out, const Request &c)
{
	out << c._method << " " << c._uri << " " << c._version << std::endl;
	// for (auto it = c._headers_map.begin(); it != c._headers_map.end(); it++)
	// {
	// 	out << "[" << it->first << "]" << std::endl << "[" << it->second << "]" << std::endl << std::endl;
	// }
	return out;
}
std::ostream&	operator<<(std::ostream& out, const Request *c)
{
	out << c->_method << " " << c->_uri << " " << c->_version << std::endl;
	for (auto it = c->_headers_map.begin(); it != c->_headers_map.end(); it++)
	{
		out << "[" << it->first << "]" << std::endl << "[" << it->second << "]" << std::endl << std::endl;
	}
	return out;
}

#endif // REQUEST_HPP