/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Request.hpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: rkieboom <rkieboom@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/23 12:38:22 by rkieboom      #+#    #+#                 */
/*   Updated: 2022/11/07 20:57:02 by bdekonin      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <string>
# include <vector>
# include <map>
# include <string.h>

# include <iostream>
# include <sstream>

/* Sources
** https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Transfer-Encoding
**
**	When Transfer-Encoding: chunked (\r\n)
**
**
*/

# include "Utils.hpp"
// # include "Job.hpp"
# include <algorithm>
# include <string>
# define MAX_ENTITIY_SIZE 16000000

class Request
{
	public:
		enum	Type
		{
			NOT_SET, // Not set
			NO_BODY, // POST missing body
			UNMATCHED_CONTENT_LENGTH, // POST CONTENT LENGTH NOT MATCHED (KEEP READING)
			ENCODING_CHUNKED, // ENCODING IS CHUNKED KEEP READING
			ERROR, // ERROR
			DONE, // DONE WITH READING
			MAX_ENTITY
		};
		enum Method
		{
			UNSET,
			GET,
			POST,
			DELETE,
			UNSUPPORTED
		};

		/* Constructor  */
		Request();

		/* Destructor */
		virtual ~Request();

		/* Copy constructor */
		Request(const Request *e);

		/* Operation overload = */
		Request& operator = (const Request *e);

		// Methods
		Request::Type add_incoming_data(char *incoming_buffer, size_t len);

		void						_parseChunk();

		bool is_complete();

		void clear();

		bool is_empty() const;

		bool is_method_get() const;
		bool is_method_post() const;
		bool is_method_delete() const;
		bool is_method_unset() const;
		bool is_method_unsupported() const;
		const std::string &get_unedited_uri() const;

		bool				is_http_supported();
		bool				is_bad_request();
		const char 			*type_i_to_s(const int i) const;
		const char 			*type_enum_to_s(Type type) const;
		const char 			*type_to_s() const;
		Type				get_type() const;

		const char 			*method_i_to_s(const int i) const;
		const char 			*method_enum_to_s(Method method) const;
		const char 			*method_to_s() const;
	
		Method 		get_method() const;
		std::string	&get_header(const std::string &header);
	
		void set_method(const Method method);
		void set_method(std::string &method);
		

	public: 
		std::map<std::string, std::string>		_headers_map;

		Method									_method;
		std::string								_uri;
		std::string								_version;
		std::string								_query_string;

		std::vector<unsigned char> _body;
		Type		_type;
		std::vector<unsigned char> _incoming_data;

		size_t _bytes_read;
		size_t _content_length;
		bool was_chunked;
	private:
		std::string _unedited_uri; // uri unedited

		template<typename T>
		int _insert_chars_to_vector(T &vector, std::string string, size_t size = 0)
		{
			return (this->_insert_chars_to_vector(vector, string.c_str(), size));
		}
		template<typename T>
		int _insert_chars_to_vector(T &vector, const char *string, size_t size)
		{
			for (size_t i = 0; i < size; i++) // TODO strlen allowed???
				vector.push_back(string[i]);
			return size;
		}
		void _request_line(std::string &line);
		void _headers(std::string &line);
		void _reading_mode();


		void add_body();
		void	setBody(std::vector<unsigned char>::iterator it, size_t length);
		void check_content_length(std::string &value);
		void check_host(std::string &value);
};

// std::ostream&	operator<<(std::ostream& out, const Request &c)
// {
// 	out << c._method << " " << c._uri << " " << c._version << std::endl;
// 	for (std::map<std::string, std::string>::const_iterator it = c._headers_map.begin(); it != c._headers_map.end(); it++)
// 	{
// 		out << "[" << it->first << "]" << std::endl << "[" << it->second << "]" << std::endl << std::endl;
// 	}
// 	out << "Body:\n"<< c._body.size() << std::endl;
// 	return out;
// }
// std::ostream&	operator<<(std::ostream& out, const Request *c)
// {
// 	out << c->_method << " " << c->_uri << " " << c->_version << std::endl;
// 	for (std::map<std::string, std::string>::const_iterator it = c->_headers_map.begin(); it != c->_headers_map.end(); it++)
// 	{
// 		out << "[" << it->first << "]" << std::endl << "[" << it->second << "]" << std::endl << std::endl;
// 	}
// 	out << "Body:\n" << c->_body.size() << std::endl;
// 	return out;
// }

#endif // REQUEST_HPP