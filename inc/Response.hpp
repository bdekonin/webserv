/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Response.hpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/09/05 15:07:07 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/10/10 15:04:25 by bdekonin      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <vector>
# include <map>
# include <string>
# include <cstring>

#include "utils.hpp"

# include "Configuration.hpp"



class Response
{
	public:
		/* Constructor  */
		Response()
		: _status_code(0), _is_build(false)
		{
			ft::RespondCodes codes = ft::RespondCodes();
			this->_response_codes = codes.get_respond_codes();
		}

		/* Destructor */
		virtual ~Response()
		{
		}

		/* Copy constructor */
		Response(const Response &src)
		{
			*this = src;
		}

		/* Operation overload = */
		Response& operator = (const Response& e)
		{
			this->_status_code = e._status_code;
			this->_headers = e._headers;
			this->_is_build = e._is_build;

			this->_response_codes = e._response_codes;
			this->_response = e._response;
			this->_body = e._body;
			return *this;
		}

		void set_status_code(int status_code)
		{
			this->_status_code = status_code;
		}
		void set_header(std::string const &header)
		{
			this->_headers += header + "\r\n";
		}
		void set_body(const char *body, size_t length, size_t start = 0)
		{
			if (start > length)
				return ;
			this->_body.insert(this->_body.end(), body + start, body + length);
		}
		int get_status_code() const
		{
			return this->_status_code;
		}
		const char *get_status_code_str() const
		{
			std::string temp;
			temp = std::to_string(this->_status_code);
			temp += " " + this->_response_codes.at(this->_status_code);
			return temp.c_str();
		}
		const std::string &get_headers() const
		{
			return this->_headers;
		}
		bool is_body_empty() const
		{
			return this->_body.empty();
		}
		void clear()
		{
			this->_status_code = 0;
			this->_headers.clear();
			this->_is_build = false;
			this->_response.clear();
			this->_body.clear();
		}

		void build_response_text()
		{
			std::string res = "";

			if (this->_is_build == true)
				;
			
			res += "HTTP/1.1 " + std::to_string(this->_status_code) + " " + this->_response_codes[this->_status_code] + "\r\n";
			res += this->_headers + "\r\n";

			if (this->_headers.find("Content-Length") == std::string::npos)
			{
				res.pop_back();
				res.pop_back();
				res += "Content-Length: " + std::to_string(this->_body.size()) + "\r\n\r\n";
			}

			this->_response.insert(this->_response.end(), res.c_str(), res.c_str() + res.length());
			this->_response.insert(this->_response.end(), this->_body.begin(), this->_body.end());
			this->_is_build = true;
		}
		std::vector <unsigned char> &get_response()
		{
			return this->_response;
		}
		void set_default_headers(std::string extension = "")
		{
			this->set_header("Server: Webserv (Bob Luke Rowan) 1.0");
			this->set_header("Content-Type: " + ft::MimeTypes().getMimeType(extension));
		}

	private:
		int	_status_code;
		std::string _headers;
		bool _is_build;

		std::map<int, std::string> _response_codes;
		std::vector<unsigned char> _response;

	public:
		std::vector<char> _body;
		void set_3xx_response(Configuration &config, std::string uri) /* Redirections */
		{
			std::map<size_t, std::string> map = config.get_return();
			std::map<size_t, std::string>::iterator it = map.begin();

			this->set_status_code(it->first);
			this->set_header("Location: " + it->second);

			std::stringstream ss;
			ss << CLRS_YEL;
			ss << "INTERNAL REDIRECT : [target : " << it->second << "]";
			ss << CLRS_reset;
			std::cerr << ss.str() << std::endl;

			this->set_header("Server: Webserv (Bob Luke Rowan) 1.0");
		}
		void set_405_response(Configuration &config) /* Method Not Allowed */ // Allow: GET, POST, DELETE
		{
			std::string allowed_methods;
			
			std::vector<std::string> methods;
			if (config.get_methods(0) == true)
				methods.push_back("GET");
			if (config.get_methods(1) == true)
				methods.push_back("POST");
			if (config.get_methods(2) == true)
				methods.push_back("DELETE");
			for (size_t i = 0; i < methods.size(); i++)
			{
				allowed_methods += methods[i];
				if (i != methods.size() - 1)
					allowed_methods += ", ";
			}
			allowed_methods.insert(0, "Allow: ");

			this->set_header(allowed_methods.c_str());

			this->set_xxx_response(config, 405);
		}
		void set_xxx_response(Configuration &config, int code)
		{
			std::string content;
			char buffer[4096 + 1];
			int fd = 0;
			int ret = 0;

			this->set_status_code(code);
			this->set_header("Server: Webserv (Bob Luke Rowan) 1.0");
			this->set_header("Content-Type: text/html");

			bzero(buffer, 4096 + 1);
			if (config.get_error_page()[code] != "")
			{
				std::string path = config.get_error_page()[code];
				fd = open(config.get_error_page()[code].c_str(), O_RDONLY);
				if (fd > 0)
					ret = read(fd, buffer, 4096);

				if (fd > 0 && ret > 0)
					content = buffer;
				else
					content = std::to_string(code) + " " + this->_response_codes[code] + "<br>Using default error page because the error page file couldnt be read or openen";
				if (fd > 0 && ret >= 0)
					close(fd);
			}
			else
				content = std::to_string(code) + " " + this->_response_codes[code];
			this->set_body(content.c_str(), content.size());
		}
	private:
		void set_500_response() /* Internal Server Error CLOSE READ ERROR*/
		{
			this->set_status_code(500);
			this->set_header("Server: Webserv (Bob Luke Rowan) 1.0");
			this->set_header("Content-Type: text/html");
			std::string error_page_content = "500 " + this->_response_codes[500];
			this->set_body(error_page_content.c_str(), error_page_content.size());
		}
};

#endif // RESPONSE_HPP