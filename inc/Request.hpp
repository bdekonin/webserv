/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Request.hpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/23 12:38:22 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/10/08 11:35:28 by bdekonin      ########   odam.nl         */
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
		: _method(Method::UNSET), _uri(""), _version(""), _body(std::vector<unsigned char>()), _type(Type::NOT_SET), _incoming_data(std::vector<unsigned char>()), _content_length(0), was_chunked(false)
		{
			this->_headers_map = std::map<std::string, std::string>();
		}

		// Setup
		void setup(char *request)
		{
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
			this->_type = e->_type;
			this->_unedited_uri = e->_unedited_uri;
			return *this;
		}

		// Methods
		void add_incoming_data(char *incoming_buffer, size_t len)
		{
			this->_incoming_data.insert(this->_incoming_data.end(), incoming_buffer, incoming_buffer + len);
			if (this->_type == Type::NOT_SET)
			{
				std::string request(reinterpret_cast<char *>(&this->_incoming_data[0]), this->_incoming_data.size());

				size_t start_body = request.find("\r\n\r\n");
				if (start_body == std::string::npos) // keep reading headers
					return ;

				std::string		headers = request.substr(0, start_body + 4);
				size_t			headers_size = headers.size();

				this->_request_line(headers);
				this->_headers(headers);
				this->_reading_mode();

				std::vector<unsigned char>::iterator it = this->_incoming_data.begin();

				// Incoming data is now only the body
				this->_incoming_data.erase(it, it + headers_size);

				if (this->_type == Type::UNMATCHED_CONTENT_LENGTH)
					this->add_body();
				else if (this->_type == Type::ENCODING_CHUNKED)
					_parseChunk();
				else if (this->_incoming_data.size() > 0)
				{
					this->_type = Type::DONE;
					this->_type = Type::ERROR;
				}
				else
					this->_type = Type::DONE;
			}
			else if (this->_type == Type::UNMATCHED_CONTENT_LENGTH)
				this->add_body();
			else if (this->_type == Type::ENCODING_CHUNKED)
				_parseChunk();
			else
				this->_type == Type::ERROR;
		}

		void						_parseChunk()
		{
			size_t	totalChunkLength = 0;
			std::vector<unsigned char>::iterator		it;
			while (true)
			{

				it = std::search(this->_incoming_data.begin(), this->_incoming_data.end(),
								"\r\n", "\r\n" + 2);
				if (it == this->_incoming_data.end())
					break;

				std::stringstream	conversionStream;
				size_t				chunkLength;
				size_t				firstLineEnd = it - this->_incoming_data.begin();
				std::string			hexString(reinterpret_cast<char*>(&this->_incoming_data[0]), firstLineEnd);
				conversionStream << std::hex << hexString;
				conversionStream >> chunkLength;
				totalChunkLength += chunkLength;

				size_t		chunkStartPos = firstLineEnd + 2;
				size_t		chunkEndPos = chunkStartPos + chunkLength;
				if (this->_incoming_data.size() < chunkEndPos + 2)
					break;
				if (this->_incoming_data[chunkEndPos] != '\r' ||
						this->_incoming_data[chunkEndPos + 1] != '\n')
				{
					this->_type = Type::ERROR;
					throw std::runtime_error("Invalid chunked encoding");
					break;
				}
				if (chunkLength == 0)
				{
					this->_type = Type::DONE;
					this->_body.push_back('\0');

					this->_headers_map["Content-Length"] = std::to_string(this->_body.size());
					this->_content_length = this->_body.size();
					this->was_chunked = true;
					break;
				}
				it = this->_incoming_data.begin() + chunkStartPos;
				this->setBody(this->_incoming_data.begin() + chunkStartPos, chunkLength);
				this->_incoming_data.erase(this->_incoming_data.begin(), this->_incoming_data.begin() + chunkEndPos + 2);
			}
		}

		bool is_complete()
		{
			if (this->_type == Type::DONE)
				return true;
			return false;
		}

		void clear()
		{
			this->_headers_map.clear();
			this->_method = Method::UNSET;
			this->_uri.clear();
			this->_version.clear();
			this->_body.clear();
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
		const char 			*type_i_to_s(const int i) const
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
		const char 			*type_enum_to_s(Type type) const
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
		const char 			*type_to_s() const
		{
			if (this->_type == Type::NOT_SET)
				return "NOT_SET";
			else if (this->_type == Type::NO_BODY)
				return "NO_BODY";
			else if (this->_type == Type::UNMATCHED_CONTENT_LENGTH)
				return "UNMATCHED_CONTENT_LENGTH";
			else if (this->_type == Type::ENCODING_CHUNKED)
				return "ENCODING_CHUNKED";
			else if (this->_type == Type::DONE)
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
		const char 			*method_to_s() const
		{
			if (this->_method == Method::UNSET)
				return "UNSET";
			else if (this->_method == Method::GET)
				return "GET";
			else if (this->_method == Method::POST)
				return "POST";
			else if (this->_method == Method::DELETE)
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
	
		void set_method(const Method method)
		{
			this->_method = method;
		}
		void set_method(std::string &method)
		{
			if (method == "GET")
				this->_method = Method::GET;
			else if (method == "POST")
				this->_method = Method::POST;
			else if (method == "DELETE")
				this->_method = Method::DELETE;
			else
				this->_method = Method::UNSUPPORTED;
		}
		

	public: 
		std::map<std::string, std::string>		_headers_map;

		Method									_method;
		std::string								_uri;
		std::string								_version;

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
		void _request_line(std::string &line)
		{
			std::string method;
			std::string uri;
			std::string version;

			size_t space[2];
			size_t end;

			space[0] = line.find(" ");
			space[1] = line.find(" ", space[0] + 1);
			end = line.find("\r\n");
			
			if (space[0] < space[1] && space[1] < end && end != std::string::npos)
			{
				method = line.substr(0, space[0]);
				uri = line.substr(space[0] + 1, space[1] - space[0] - 1);
				version = line.substr(space[1] + 1, end - space[1] - 1);
				
				this->set_method(method);
				this->_uri = uri;
				this->_unedited_uri = uri;
				this->_version = version;
				line = line.substr(end + 2);
			}
			else
				this->_type = Type::ERROR;
		}
		void _headers(std::string &line)
		{
			size_t end;
			size_t colon;
			std::string header;
			std::string value;

			while ((end = line.find("\r\n")) != std::string::npos)
			{
				colon = line.find(":");
				if (colon < end)
				{
					header = line.substr(0, colon);
					value = line.substr(colon + 2, end - colon - 2);

					for (size_t i = 0; i < header.length(); i++)
						header[i] = std::tolower(header[i]);

					this->_headers_map[header] = value;
				}
				line = line.substr(end + 2);
			}
		}
		void _reading_mode()
		{
			std::string value;

			if ((value =this->get_header("content-length")) != "")
			{
				this->_content_length = atoi(value.c_str());
				this->_type = Type::UNMATCHED_CONTENT_LENGTH;
			}
			else if (this->get_header("transfer-encoding") == "chunked")
				this->_type = Type::ENCODING_CHUNKED;
			else
				this->_type = Type::NO_BODY;
		}


		void add_body()
		{
			this->setBody(this->_incoming_data.begin(), this->_incoming_data.size());
			this->_incoming_data.clear();
			if (this->_body.size() >= this->_content_length)
			{
				this->_type = Type::DONE;
				if (this->_body.size() != this->_content_length)
					this->_type = Type::ERROR;
			}
		}
		void	setBody(std::vector<unsigned char>::iterator it, size_t length)
		{
			this->_body.insert(this->_body.end(), it, it + length);
		}














		void parse_request(const char *raw, int bytesread = 0)
		{
			// Method
			size_t meth_len = strcspn(raw, " ");
			if (memcmp(raw, "GET", strlen("GET")) == 0)
				this->_method = Method::GET;
			else if (memcmp(raw, "POST", strlen("POST")) == 0)
				this->_method = Method::POST;
			else if (memcmp(raw, "DELETE", strlen("DELETE")) == 0)
				this->_method = Method::DELETE;
			else
				this->_method = Method::UNSUPPORTED;
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
			this->_insert_chars_to_vector(this->_body, cbody, body_len);

			if (this->_method == Method::POST)
			{
				if (this->get_header("content-length") == "")
					this->_type == this->ERROR;

				size_t body_size = this->_body.size();
				size_t content_length = atoi(this->get_header("content-length").c_str());

				if (body_size == 0)
					this->_type = this->NO_BODY;
				else if (body_size != content_length)
					this->_type = this->UNMATCHED_CONTENT_LENGTH;
				else
					this->_type = this->DONE;
			}
			else if (this->get_header("encoding") == "chunked")
				this->_type = Type::ENCODING_CHUNKED;
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