/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Request.hpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/23 12:38:22 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/10/04 16:22:40 by bdekonin      ########   odam.nl         */
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

#define getString(n) #n
#define VARRR(var) std::cerr << std::boolalpha << __FILE__ ":"<< __LINE__ << ":\t" << getString(var) << " = [" <<  (var) << "]" << std::noboolalpha << std::endl;
#define PRINT(var) std::cout << var << std::endl


class Request
{
	public:
		/* Constructor  */
		Request()
		: _method(""), _uri(""), _version(""), _body(std::vector<char>()), _hostname("")
		{
			this->_headers_map = std::map<std::string, std::string>();
		}
		Request(const char *buffer)
		: _hostname("")
		{
			this->setup(buffer);
		}

		// Setup

		void setup(const char *buffer)
		{
			this->parse_request(buffer);
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
		int add_incoming_data(char *incoming_buffer)
		{
			static std::string buffer;

			buffer.append(incoming_buffer);
			
			VARRR(buffer);
			VARRR(this->is_complete());
			if (this->is_complete())
			{
				this->setup(buffer.c_str());
				buffer.clear();
				return 1;
			}
			return 0;
		}
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
		bool is_complete() const
		{
			std::map<std::string, std::string>::const_iterator it;

			it = this->_headers_map.find("content-length");

			if (it == this->_headers_map.end())
				return true;
			else
			{
				if (this->_body.size() == atoi(it->second.c_str()))
					return true;
				else
					return false;
			}
		}
		bool is_empty() const
		{
			if (this->_method == "" && this->_uri == "" && this->_version == "" && this->_headers_map.empty())
				return true;
			else
				return false;
		}
	public:
		std::map<std::string, std::string> _headers_map;

		std::string _method;
		std::string _uri;
		std::string _version;

		std::vector<char> _body;
		std::string _hostname;
	private:
		std::string _unedited_uri; // uri unedited
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

			// std::cout << "URI\n" << std::endl;

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
			// std::cout << "VERSION\n" << std::endl;

			// HTTP-Version
			{
				size_t ver_len = strcspn(raw, "\r\n");
				char version[1000];
				memcpy(version, raw, ver_len);
				version[ver_len] = '\0';
				this->_version = version;
				raw += ver_len + 2; // move past <CR><LF>
			}
			
			// std::cout << "HEADERS\n" << std::endl;

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

			// std::cout << "BODY\n" << std::endl;

			size_t body_len = strlen(raw);
			char cbody[body_len + 1];
			memcpy(cbody, raw, body_len);
			cbody[body_len] = '\0';
			this->_insert_chars_to_vector(this->_body, cbody);

			// std::cout << "DONE " << body_len << "\n" << std::endl;
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