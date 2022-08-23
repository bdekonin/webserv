/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Request.hpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/23 12:38:22 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/08/23 17:44:03 by bdekonin      ########   odam.nl         */
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

class Request
{
	public:
		/* Constructor  */
		Request();

		/* Destructor */
		virtual ~Request();

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
			
			value = header.substr(pos, + 2);
			this->_headers_map[identifier] = value;
			std::cout << "Header: " << identifier << " Value: " << value << std::endl;
		}
	private:
	// List of Headers in vector
		std::vector<std::string, std::string>	_headers;
		std::map<std::string, std::string> _headers_map;

		
	// Request-Line
		std::string _method; // GET POST DELETE
		std::string _request_uri; // /index.html
		std::string _http_version; // HTTP/1.1

	// Request-Header
		std::string _host; // The domain name of the server
		std::string _accept; // The media type/types acceptable
};

#endif // REQUEST_HPP