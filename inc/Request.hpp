/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Request.hpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/23 12:38:22 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/09/05 13:34:40 by bdekonin      ########   odam.nl         */
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
		{
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
			
			// std::cout << buffer << std::endl;

						
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
		}

		/* Destructor */
		virtual ~Request()
		{
		}

		/* Copy constructor */
		Request(const Request&);

		/* Operation overload = */
		Request& operator = (const Request& e);

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

#endif // REQUEST_HPP