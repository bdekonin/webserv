/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Response.hpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/09/05 15:07:07 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/09/21 14:13:48 by bdekonin      ########   odam.nl         */
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

# define HTTP_VERSION "HTTP/1.1"




class Response
{
	public:
		/* Constructor  */
		Response()
		: _status_code(0)
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
			this->_status_line = e._status_line;
			this->_headers = e._headers;
			this->_body = e._body;

			this->_headers_map = e._headers_map;
			return *this;
		}

		void set_status_code(int status_code)
		{
			this->_status_code = status_code;
		}
		int get_status_code()
		{
			return this->_status_code;
		}
		void set_header(const std::string &identifier, const std::string &value)
		{
			this->_headers_map[identifier] = value;
		}
		void set_body(const std::string &body)
		{
			this->_insert_chars_to_vector(this->_body, body);
		}
		void set_content_length()
		{
			this->set_header("Content-Length", std::to_string(this->_body.size()));
		}

		std::vector<char> build_response()
		{
			std::vector<char> total_response;

			// Status line
			this->build_status_line();
			total_response.insert(total_response.end(), this->_status_line.begin(), this->_status_line.end());

			// Headers (with \r\n added to the end for empty line CRLF (look at README))
			this->build_headers();
			total_response.insert(total_response.end(), this->_headers.begin(), this->_headers.end());

			// this->_insert_chars_to_vector(total_response, "\r\n");
			total_response.push_back('\r');
			total_response.push_back('\n');
			// Body
			total_response.insert(total_response.end(), this->_body.begin(), this->_body.end());

			return total_response;
		}
		void clear()
		{
			this->_status_code = 0;
			this->_status_line.clear();
			this->_headers.clear();
			this->_body.clear();
			this->_headers_map.clear();
		}
		bool is_body_empty()
		{
			return this->_body.empty();
		}

		void set_default_headers(std::string extension = "")
		{
			this->set_header("Server", "Webserv (Luke & Bob) 1.0");
			this->set_header("Content-Type", ft::MimeTypes().getMimeType(extension));
		}
	private:
		int	_status_code;
		std::vector<char> _status_line;
		std::vector<char> _headers;
		std::vector<char> _body;
		std::map<int, std::string> _response_codes;
		std::map<std::string, std::string> _headers_map;

		void _insert_chars_to_vector(std::vector<char> &vector, std::string string)
		{
			for (size_t i = 0; i < string.size(); i++)
				vector.push_back(string[i]);
		}
		void _insert_chars_to_vector(std::vector<char> &vector, const char *string)
		{
			for (size_t i = 0; i < strlen(string); i++) // TODO strlen allowed???
				vector.push_back(string[i]);
		}
		void build_status_line()
		{
			if (this->_status_code == 0)
				this->_status_code = 500; // Internal Server Error | Something went wrong and no status code was set

			this->_insert_chars_to_vector(this->_status_line, HTTP_VERSION);
			this->_insert_chars_to_vector(this->_status_line, " ");
			this->_insert_chars_to_vector(this->_status_line, std::to_string(this->_status_code));
			this->_insert_chars_to_vector(this->_status_line, " ");
			this->_insert_chars_to_vector(this->_status_line, this->_response_codes[this->_status_code]);
			this->_insert_chars_to_vector(this->_status_line, "\r\n"); // TODO WIll print something weird if you print to cout. because of the \r\n
		}
		void build_headers() // uses headers_map to append to vector
		{
			for (std::map<std::string, std::string>::const_iterator it = this->_headers_map.begin(); it != this->_headers_map.end(); it++)
			{
				this->_insert_chars_to_vector(this->_headers, it->first);
				this->_insert_chars_to_vector(this->_headers, ": ");
				this->_insert_chars_to_vector(this->_headers, it->second);
				if (it == this->_headers_map.end()--)
					this->_insert_chars_to_vector(this->_headers, "\r\n");
				else
					this->_insert_chars_to_vector(this->_headers, "\n");
			}
		}

	public:
		void set_3xx_response(Configuration &config, std::string uri) /* Redirections */
		{
			std::map<size_t, std::string> map = config.get_return();

			this->set_status_code(map.begin()->first);
			this->set_header("Location", map.begin()->second);
			this->set_header("Server", "Webserv (Luke & Bob) 1.0");

			this->set_content_length();
		}
		void set_403_response(Configuration &config) /* Forbidden */
		{
			this->set_status_code(403);
			this->set_header("Server", "Webserv (Luke & Bob) 1.0");
			this->set_header("Content-Type", "text/html");
			if (config.get_error_page()[500] != "")
			{
				std::ifstream error_page_file(config.get_error_page()[500]);
				std::string error_page_content((std::istreambuf_iterator<char>(error_page_file)), std::istreambuf_iterator<char>());
				this->set_body(error_page_content);
			}
			else
				this->set_body("403 " + this->_response_codes[403]);

			this->set_content_length();
		}
		void set_404_response(Configuration &config) /* Not Found */
		{
			this->set_status_code(404);
			this->set_header("Server", "Webserv (Luke & Bob) 1.0");
			this->set_header("Content-Type", "text/html");
			if (config.get_error_page()[404] != "")
			{
				std::ifstream error_page_file(config.get_error_page()[405]);
				std::string error_page_content((std::istreambuf_iterator<char>(error_page_file)), std::istreambuf_iterator<char>());
				this->set_body(error_page_content);
			}
			else
				this->set_body("404 " + this->_response_codes[404]);

			this->set_content_length();
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

			this->set_header("Allow", allowed_methods);
			this->set_header("Server", "Webserv (Luke & Bob) 1.0");
			this->set_header("Content-Type", "text/html");

			if (config.get_error_page()[405] != "")
			{
				std::ifstream error_page_file(config.get_error_page()[405]);
				std::string error_page_content((std::istreambuf_iterator<char>(error_page_file)), std::istreambuf_iterator<char>());
				this->set_body(error_page_content);
			}
			else
				this->set_body("405 " + this->_response_codes[405]);

			this->set_content_length();
		}
		void set_500_response(Configuration &config) /* Internal Server Error */
		{
			this->set_status_code(500);
			this->set_header("Server", "Webserv (Luke & Bob) 1.0");
			this->set_header("Content-Type", "text/html");
			if (config.get_error_page()[500] != "")
			{
				std::ifstream error_page_file(config.get_error_page()[500]);
				std::string error_page_content((std::istreambuf_iterator<char>(error_page_file)), std::istreambuf_iterator<char>());
				this->set_body(error_page_content);
			}
			else
				this->set_body("500 " + this->_response_codes[500]);

			this->set_content_length();
		}
};

#endif // RESPONSE_HPP