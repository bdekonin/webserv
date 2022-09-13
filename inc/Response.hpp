/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Response.hpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/09/05 15:07:07 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/09/13 11:40:25 by bdekonin      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <vector>
# include <map>
# include <string>
# include <cstring>

# include "Configuration.hpp"

# define HTTP_VERSION "HTTP/1.1"


class RespondCodes
{
	public:
		/* Constructor  */
		RespondCodes()
		{
			response_codes[100] = "Continue";
			response_codes[101] = "Switching protocols";
			response_codes[102] = "Processing";
			response_codes[103] = "Early Hints";

			response_codes[200] = "OK";
			response_codes[201] = "Created";
			response_codes[202] = "Accepted";
			response_codes[204] = "No Content";
			response_codes[205] = "Reset Content";
			response_codes[206] = "Partial Content";
			response_codes[207] = "Multi-Status";
			response_codes[208] = "Already Reported";
			response_codes[226] = "IM Used";

			response_codes[300] = "Multiple Choices";
			response_codes[301] = "Moved Permanently";
			response_codes[302] = "Found (Previously \"Moved Temporarily\")";
			response_codes[303] = "See Other";
			response_codes[304] = "Not Modified";
			response_codes[305] = "Use Proxy";
			response_codes[306] = "Switch Proxy";
			response_codes[307] = "Temporary Redirect";
			response_codes[308] = "Permanent Redirect";

			response_codes[400] = "Bad Request";
			response_codes[401] = "Unauthorized";
			response_codes[402] = "Payment Required";
			response_codes[403] = "Forbidden";
			response_codes[404] = "Not Found";
			response_codes[405] = "Method Not Allowed";
			response_codes[406] = "Not Acceptable";
			response_codes[407] = "Proxy Authentication Required";
			response_codes[408] = "Request Timeout";
			response_codes[409] = "Conflict";
			response_codes[410] = "Gone";
			response_codes[411] = "Length Required";
			response_codes[412] = "Precondition Failed";
			response_codes[413] = "Request Entity Too Large";
			response_codes[414] = "URI Too Long";
			response_codes[415] = "Unsupported Media Type";
			response_codes[416] = "Range Not Satisfiable";
			response_codes[417] = "Expectation Failed";
			response_codes[418] = "I'm a Teapot";
			response_codes[421] = "Misdirected Request";
			response_codes[422] = "Unprocessable Entity";
			response_codes[423] = "Locked";
			response_codes[424] = "Failed Dependency";
			response_codes[425] = "Too Early";
			response_codes[426] = "Upgrade Required";
			response_codes[428] = "Precondition Required";
			response_codes[429] = "Too Many Requests";
			response_codes[431] = "Request Header Fields Too Large";
			response_codes[451] = "Unavailable For Legal Reasons";

			response_codes[500] = "Internal Server Error";
			response_codes[501] = "Not Implemented";
			response_codes[502] = "Bad Gateway";
			response_codes[503] = "Service Unavailable";
			response_codes[504] = "Gateway Timeout";
			response_codes[505] = "HTTP Version Not Supported";
			response_codes[506] = "Variant Also Negotiates";
			response_codes[507] = "Insufficient Storage";
			response_codes[508] = "Loop Detected";
			response_codes[510] = "Not Extended";
			response_codes[511] = "Network Authentication Required";
		}

		/* Destructor */
		virtual ~RespondCodes()
		{
		}

		std::map<int, std::string> get_respond_codes()
		{
			return response_codes;
		}

		std::string get_error_page(int error)
		{
			std::string page("<!DOCTYPE html>\n<html>\n<head>\n<title>Webserv Bob & Luke</title>\n</head>\n<body>\n<h1>CODE STRING</h1>\n</body>\n</html>");

			page.replace(page.find("CODE"), 4, std::to_string(error));
			page.replace(page.find("STRING"), 6, response_codes[error]);

			return page;
		}

		std::map<int, std::string> response_codes;
};

class Response
{
	public:
		/* Constructor  */
		Response()
		: _status_code(0)
		{
			RespondCodes codes = RespondCodes();
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

		// Methods
		/*
			set_statuscode // bad request forbidden etc
			build_status_line 
			set_headers
			set_body
			get_respone_in_string
			build_response
		*/

		void set_status_code(int status_code)
		{
			this->_status_code = status_code;
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
		
		std::vector<char> _status_line;
		std::vector<char> _headers;
		std::vector<char> _body;
		int	_status_code;

	public:

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

	public:
		std::vector<char> build_405(Configuration &config)
		{
			this->set_status_code(405);
			this->build_status_line();

			// Setting Allow Header
			{
				std::vector<std::string> allowed_methods;

				if (config.get_methods(0) == false) // not allowed
					allowed_methods.push_back("GET");
				if (config.get_methods(1) == false) // not allowed
					allowed_methods.push_back("HEAD");
				if (config.get_methods(2) == false) // not allowed
					allowed_methods.push_back("POST");

				std::string allowed_methods_string;
				for (size_t i = 0; i < allowed_methods.size(); i++)
				{
					allowed_methods_string += allowed_methods[i];
					if (i != allowed_methods.size() - 1)
						allowed_methods_string += ", ";
				}

				this->set_header("Allow", allowed_methods_string);
			}
			this->set_header("Content-Type", "text/html");
			this->set_header("Server", "Webserv (Luke & Bob) 1.0");
			{
				if (config.get_error_page()[405] != "")
				{
					std::ifstream error_page_file(config.get_error_page()[405]);
					std::string error_page_content((std::istreambuf_iterator<char>(error_page_file)), std::istreambuf_iterator<char>());
					this->set_body(error_page_content);
				}
				else
				{
					this->set_body("<html><head><title>405 Method Not Allowed</title></head><body><h1>405 Method Not Allowed</h1></body></html>");
				}
			}

			this->set_content_length();
			return this->build_response();
		}
};

#endif // RESPONSE_HPP