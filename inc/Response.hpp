/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Response.hpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/09/05 15:07:07 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/09/05 21:52:22 by bdekonin      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <vector>
# include <map>
# include <string>
# include <cstring>

# define HTTP_VERSION "HTTP/1.1"


class RespondCodes
{
	public:
		/* Constructor  */
		RespondCodes()
		{
			response_codes[500] = "Internal Server Error";
			response_codes[200] = "OK";
		}

		/* Destructor */
		virtual ~RespondCodes()
		{
		}

		std::map<int, std::string> get_respond_codes()
		{
			return response_codes;
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
			for (auto it = this->_headers_map.begin(); it != this->_headers_map.end(); it++)
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
};

#endif // RESPONSE_HPP