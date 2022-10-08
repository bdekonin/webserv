/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Response.hpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/09/05 15:07:07 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/10/08 09:16:37 by bdekonin      ########   odam.nl         */
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
			this->set_header("Server: Webserv (Luke & Bob) 1.0");
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

			this->set_status_code(map.begin()->first);
			this->set_header("Location: " + map.begin()->second);
			this->set_header("Server: Webserv (Bob Luke Rowan) 1.0");
		}
		void set_403_response(Configuration &config) /* Forbidden */
		{
			this->set_status_code(403);
			this->set_header("Server: Webserv (Luke & Bob) 1.0");
			this->set_header("Content-Type: text/html");
			if (config.get_error_page()[500] != "")
			{
				std::ifstream error_page_file(config.get_error_page()[500]);
				std::string error_page_content((std::istreambuf_iterator<char>(error_page_file)), std::istreambuf_iterator<char>());
				this->set_body(error_page_content.c_str(), error_page_content.size());
			}
			else
			{
				std::string error_page_content = "403 " + this->_response_codes[403];
				this->set_body(error_page_content.c_str(), error_page_content.size());
			}
		}
		void set_404_response(Configuration &config) /* Not Found */
		{
			this->set_status_code(404);
			this->set_header("Server: Webserv (Luke & Bob) 1.0");
			this->set_header("Content-Type: text/html");
			if (config.get_error_page()[404] != "")
			{
				std::ifstream error_page_file(config.get_error_page()[405]);
				std::string error_page_content((std::istreambuf_iterator<char>(error_page_file)), std::istreambuf_iterator<char>());
				this->set_body(error_page_content.c_str(), error_page_content.size());
			}
			else
			{
				std::string error_page_content = "404 " + this->_response_codes[404];
				this->set_body(error_page_content.c_str(), error_page_content.size());
			}
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
			this->set_status_code(405);
			allowed_methods.insert(0, "Allow: ");

			this->set_header(allowed_methods.c_str());
			this->set_header("Server: Webserv (Luke & Bob) 1.0");
			this->set_header("Content-Type: text/html");

			if (config.get_error_page()[405] != "")
			{
				std::ifstream error_page_file(config.get_error_page()[405]);
				std::string error_page_content((std::istreambuf_iterator<char>(error_page_file)), std::istreambuf_iterator<char>());
				this->set_body(error_page_content.c_str(), error_page_content.size());
			}
			else
			{
				std::string error_page_content = "405 " + this->_response_codes[405];
				this->set_body(error_page_content.c_str(), error_page_content.size());
			}
		}
		void set_500_response(Configuration &config) /* Internal Server Error */
		{
			this->set_status_code(500);
			this->set_header("Server: Webserv (Luke & Bob) 1.0");
			this->set_header("Content-Type: text/html");
			if (config.get_error_page()[500] != "")
			{
				std::ifstream error_page_file(config.get_error_page()[500]);
				std::string error_page_content((std::istreambuf_iterator<char>(error_page_file)), std::istreambuf_iterator<char>());
				this->set_body(error_page_content.c_str(), error_page_content.size());
			}
			else
			{
				std::string error_page_content = "500 " + this->_response_codes[500];
				this->set_body(error_page_content.c_str(), error_page_content.size());
			}
		}
};

#endif // RESPONSE_HPP