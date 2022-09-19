/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Request.hpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/23 12:38:22 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/09/19 13:21:35 by bdekonin      ########   odam.nl         */
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
# include "Job.hpp"

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
		Request(const char *buffer)
		{
			std::string str(buffer);
			this->setup(str);
		}

		// Setup

		void setup(std::string &buffer)
		{
			std::vector<std::string> lines;
			split(buffer, "\r\n", lines);

			for (size_t i = 1; i < lines.size(); i++)
				this->set_header(lines[i]);

			std::vector<std::string> request_line;
			split(lines[0], " ", request_line);\

			to_upper(request_line[0]);

			this->_method = request_line[0];
			this->_uri = request_line[1];

			this->_unedited_uri = this->_uri;

			this->_version = request_line[2];
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
			this->_method = e->_method;
			this->_uri = e->_uri;
			this->_version = e->_version;

			this->_unedited_uri = e->get_unedited_uri();

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

			identifier = ft_strtrim(identifier, whitespaces); // TODO WHITESPACES
			value = ft_strtrim(value, whitespaces);

			// std::cout << "[" << identifier << "]\t[" << value << "]\n";
			this->_headers_map[identifier] = value;
		}
		void clear()
		{
			this->_method = "";
			this->_uri = "";
			this->_version = "";
			this->_headers_map.clear();

			this->_unedited_uri = "";
		}
		const std::string &get_unedited_uri() const
		{
			return this->_unedited_uri;
		}
	public:
		std::map<std::string, std::string> _headers_map;

		std::string _method;
		std::string _uri;
		std::string _version;

	private:
		std::string _unedited_uri; // uri unedited
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
	for (std::map<std::string, std::string>::const_iterator it = c->_headers_map.begin(); it != c->_headers_map.end(); it++)
	{
		out << "[" << it->first << "]" << std::endl << "[" << it->second << "]" << std::endl << std::endl;
	}
	return out;
}

#endif // REQUEST_HPP