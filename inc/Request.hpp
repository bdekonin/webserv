/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Request.hpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/23 12:38:22 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/10/05 18:10:28 by bdekonin      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <string>
# include <vector>
# include <map>
#include <string.h>

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
# include <algorithm>
# include <string>

#define getString(n) #n
#define VARRR(var) std::cerr << std::boolalpha << __FILE__ ":"<< __LINE__ << ":\t" << getString(var) << " = [" <<  (var) << "]" << std::noboolalpha << std::endl;
#define PRINT(var) std::cout << var << std::endl

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
			DONE // DONE WITH READING
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
		Request()
		: _method(Method::UNSET), _uri(""), _version(""), _body(std::vector<char>()), _hostname(""), _type(Type::NOT_SET), _request_total_buffer(std::vector<char>())
		{
			this->_headers_map = std::map<std::string, std::string>();
		}

		// Setup
		void setup()
		{
			char *request = reinterpret_cast<char*> (&this->_request_total_buffer[0]);

			this->parse_request(request);
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
			this->_headers_map = e->_headers_map;
			this->_method = e->_method;
			this->_uri = e->_uri;
			this->_version = e->_version;
			this->_body = e->_body;
			this->_hostname = e->_hostname;
			this->_type = e->_type;
			this->_unedited_uri = e->_unedited_uri;
			return *this;
		}
		// Methods
		void add_incoming_data(char *incoming_buffer)
		{
			if (this->_request_total_buffer.size() == 0)
				this->_insert_chars_to_vector(this->_request_total_buffer, incoming_buffer);
			else
			{
				size_t vector_size;

				vector_size = this->_request_total_buffer.size();
				
				if (this->_request_total_buffer.back() == '\0')
					this->_request_total_buffer.pop_back();
				this->_insert_chars_to_vector(this->_request_total_buffer, incoming_buffer);
			}
			this->_request_total_buffer.push_back('\0');
		}





		bool is_complete()
		{
			if (this->_request_total_buffer.size() == 0)
				return false;
			const char *request = reinterpret_cast<const char*> (&this->_request_total_buffer[0]);
			char *body = (char *)strstr(request, "\r\n\r\n");
			
			if (!body)
				return false;
			body += 4;
			if (ft_strnstr(request, "chunked", strlen(request) - strlen(body)))
			{
				if (strstr(body, "\r\n\r\n"))
					return true;
				return false;
			}
			else if (ft_strnstr(request, "Content-Length", strlen(request) - strlen(body)))
			{
				// if (strstr(body, "\r\n\r\n"))
				// 	return true;
				char *start = ft_strnstr(request, "Content-Length: ", strlen(request) - strlen(body)) + 16;
				char *end = strstr(start, "\r\n");
				char *len = strndup(start, end - start);
				free(len);
				int len_i = atoi(len);
				if ((size_t)len_i <= strlen(body))
					return true;
				return false;
			}
			else if (ft_strnstr(request, "boundary=", strlen(request) - strlen(body)))
			{
				if (strstr(body, "\r\n\r\n"))
					return true;
				return false;
			}
			return true;


			// if (this->_type == this->NO_BODY)
			// 	return false;

			return this->_type == this->DONE;
			// std::map<std::string, std::string>::const_iterator it;

			// it = this->_headers_map.find("content-length");

			// if (it == this->_headers_map.end())
			// 	return true;
			// else
			// {
			// 	if (this->_body.size() == atoi(it->second.c_str()))
			// 		return true;
			// 	else
			// 		return false;
			// }
		}






		void clear()
		{
			this->_headers_map.clear();
			this->_method = Method::UNSET;
			this->_uri.clear();
			this->_version.clear();
			this->_body.clear();
			this->_hostname.clear();
			this->_unedited_uri.clear();
			this->_type = Type::NOT_SET;
		}
		bool is_empty() const
		{
			if (this->_method == Method::UNSET && this->_uri == "" && this->_version == "" && this->_headers_map.empty())
				return true;
			else
				return false;
		}
		bool is_method_get() const
		{
			return this->_method == Method::GET;
		}
		bool is_method_post() const
		{
			return this->_method == Method::POST;
		}
		bool is_method_delete() const
		{
			return this->_method == Method::DELETE;
		}
		bool is_method_unset() const
		{
			return this->_method == Method::UNSET;
		}
		bool is_method_unsupported() const
		{
			return this->_method == Method::UNSUPPORTED;
		}
		const std::string &get_unedited_uri() const
		{
			return this->_unedited_uri;
		}

		bool is_bad_request()
		{
			if (this->_type == Type::ERROR)
				return true;

			if (this->_method == Method::UNSUPPORTED)
				return true;

			if (this->get_header("host") == "")
				return true;

			if (this->_uri == "")
				return true;

			if (this->_version == "")
				return true;
			
			if (this->_method == Method::POST)
			{
				if (this->get_header("content-length") == "")
					return true; 
				
				if (this->_type == Type::NO_BODY)
					return true;

				if (this->_type == Type::UNMATCHED_CONTENT_LENGTH)
					return true;
			}

			return false;
		}
		const char 			*type__i_to_s(const int i) const
		{
			if (i == 0)
				return "NOT_SET";
			else if (i == 1)
				return "NO_BODY";
			else if (i == 2)
				return "UNMATCHED_CONTENT_LENGTH";
			else if (i == 3)
				return "ENCODING_CHUNKED";
			else if (i == 5)
				return "DONE";
			return "ERROR";
		}
		const char 			*type__enum_to_s(Type type) const
		{
			if (type == Type::NOT_SET)
				return "NOT_SET";
			else if (type == Type::NO_BODY)
				return "NO_BODY";
			else if (type == Type::UNMATCHED_CONTENT_LENGTH)
				return "UNMATCHED_CONTENT_LENGTH";
			else if (type == Type::ENCODING_CHUNKED)
				return "ENCODING_CHUNKED";
			else if (type == Type::DONE)
				return "DONE";
			return "ERROR";
		}
		const char 			*method_i_to_s(const int i) const
		{
			if (i == 0)
				return "UNSET";
			else if (i == 1)
				return "GET";
			else if (i == 2)
				return "POST";
			else if (i == 3)
				return "DELETE";
			return "UNSUPPORTED";
		}
		const char 			*method_enum_to_s(Method method) const
		{
			if (method == Method::UNSET)
				return "UNSET";
			else if (method == Method::GET)
				return "GET";
			else if (method == Method::POST)
				return "POST";
			else if (method == Method::DELETE)
				return "DELETE";
			return "UNSUPPORTED";
		}
		const Method 		get_method() const
		{
			return this->_method;
		}
		std::string	&get_header(const std::string &header)
		{
			return this->_headers_map[header];
		}
	
	public:
		std::map<std::string, std::string>		_headers_map;

		Method									_method;
		std::string								_uri;
		std::string								_version;

		std::vector<char> _body;
		std::string _hostname;
		short		_type;
		std::vector<char> _request_total_buffer;
	private:
		std::string _unedited_uri; // uri unedited
		int _insert_chars_to_vector(std::vector<char> &vector, std::string string)
		{
			size_t size = string.size();

			for (size_t i = 0; i < size; i++)
				vector.push_back(string[i]);
			return size;
			
		}
		int _insert_chars_to_vector(std::vector<char> &vector, const char *string)
		{
			size_t size = strlen(string);
			for (size_t i = 0; i < size; i++) // TODO strlen allowed???
				vector.push_back(string[i]);
			return size;
		}
		void parse_request(const char *raw)
		{
			// Method
			size_t meth_len = strcspn(raw, " ");
			if (memcmp(raw, "GET", strlen("GET")) == 0)
				this->_method = "GET";
			else if (memcmp(raw, "POST", strlen("POST")) == 0)
				this->_method = "POST";
			else if (memcmp(raw, "DELETE", strlen("DELETE")) == 0)
				this->_method = "DELETE";
			else
				this->_method = "UNSUPPORTED";
			raw += meth_len + 1; // move past <SP>

			// Request-URI
			{
				size_t url_len = strcspn(raw, " ");
				char url[1000];
				memcpy(url, raw, url_len);
				url[url_len] = '\0';
				this->_uri = url;
				this->_unedited_uri = url;
				raw += url_len + 1; // move past <SP>
			}

			// HTTP-Version
			{
				size_t ver_len = strcspn(raw, "\r\n");
				char version[1000];
				memcpy(version, raw, ver_len);
				version[ver_len] = '\0';
				this->_version = version;
				raw += ver_len + 2; // move past <CR><LF>
			}

			std::string name, value;
			char cname[1000], cvalue[1000];
			while (raw[0]!='\r' || raw[1]!='\n')
			{
				bzero(cname, 1000);
				bzero(cvalue, 1000);
				// name
				size_t name_len = strcspn(raw, ":");
				memcpy(cname, raw, name_len);
				cname[name_len] = '\0';
				name = cname;
				raw += name_len + 1; // move past :
				while (*raw == ' ')
					raw++;

				// value
				size_t value_len = strcspn(raw, "\r\n");
				memcpy(cvalue, raw, value_len);
				cvalue[value_len] = '\0';
				value = cvalue;
				raw += value_len + 2; // move past <CR><LF>

				for (size_t i = 0; i < name.length(); i++)
					name[i] = std::tolower(name[i]);
				
				this->_headers_map[name] = value;
			}
			raw += 2; // move past <CR><LF>

			size_t body_len = strlen(raw);
			char cbody[body_len + 1];
			memcpy(cbody, raw, body_len);
			cbody[body_len] = '\0';
			this->_insert_chars_to_vector(this->_body, cbody);

			VARRR(this->_body.size());
			if (this->_method == "POST")
			{
				if (this->get_header("content-length") == "")
					this->_type == this->ERROR;

				if (this->_body.size() == 0)
					this->_type = this->NO_BODY;
				else if (this->_body.size() != atoi(this->get_header("content-length").c_str()))
					this->_type = this->UNMATCHED_CONTENT_LENGTH;
				else
					this->_type = this->DONE;
			}
			else
				this->_type = this->DONE;
		}
};

std::ostream&	operator<<(std::ostream& out, const Request &c)
{
	out << c._method << " " << c._uri << " " << c._version << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = c._headers_map.begin(); it != c._headers_map.end(); it++)
	{
		out << "[" << it->first << "]" << std::endl << "[" << it->second << "]" << std::endl << std::endl;
	}
	out << "Body:\n"<< c._body.size() << std::endl;
	return out;
}
std::ostream&	operator<<(std::ostream& out, const Request *c)
{
	out << c->_method << " " << c->_uri << " " << c->_version << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = c->_headers_map.begin(); it != c->_headers_map.end(); it++)
	{
		out << "[" << it->first << "]" << std::endl << "[" << it->second << "]" << std::endl << std::endl;
	}
	out << "Body:\n" << c->_body.size() << std::endl;
	return out;
}

#endif // REQUEST_HPP