/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Response.cpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: rkieboom <rkieboom@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/11/05 13:31:44 by rkieboom      #+#    #+#                 */
/*   Updated: 2022/11/07 21:37:19 by bdekonin      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Response.hpp"

Response::Response()
: _status_code(0), _is_build(false)
{
	ft::RespondCodes codes = ft::RespondCodes();
	this->_response_codes = codes.get_respond_codes();
}

/* Destructor */
Response::~Response()
{
}

/* Copy constructor */
Response::Response(const Response &src)
{
	*this = src;
}

/* Operation overload = */
Response& Response::operator = (const Response& e)
{
	this->_status_code = e._status_code;
	this->_headers = e._headers;
	this->_is_build = e._is_build;

	this->_response_codes = e._response_codes;
	this->_response = e._response;
	this->_body = e._body;
	return *this;
}

void Response::set_status_code(int status_code)
{
	this->_status_code = status_code;
}
void Response::set_header(std::string const &header)
{
	this->_headers += header + "\r\n";
}
void Response::set_body(const char *body, size_t length, size_t start)
{
	if (start > length)
		return ;
	this->_body.insert(this->_body.end(), body + start, body + length);
}
void Response::set_body(const char *body, size_t length)
{
	this->_body.insert(this->_body.end(), body, body + length);
	// this->_body.insert(this->_body.end(), body, length);
}
int Response::get_status_code() const
{
	return this->_status_code;
}
const std::string &Response::get_headers() const
{
	return this->_headers;
}
bool Response::is_body_empty() const
{
	return this->_body.empty();
}
void Response::clear()
{
	this->_status_code = 0;
	this->_headers.clear();
	this->_is_build = false;
	this->_response.clear();
	this->_body.clear();
}

void Response::build_response_text()
{
	std::string res = "";

	if (this->_is_build == true)
	{
		return ;
	}
	
	res += "HTTP/1.1 " + std::to_string(this->_status_code) + " " + this->_response_codes[this->_status_code] + "\r\n";
	res += this->_headers + "\r\n";

	if (this->_headers.find("Content-Length") == std::string::npos)
	{
		// res.pop_back();
		res.erase(res.size() - 1);
		// res.pop_back();
		res.erase(res.size() - 1);
		res += "Content-Length: " + std::to_string(this->_body.size()) + "\r\n\r\n";
	}

	this->_response.insert(this->_response.end(), res.c_str(), res.c_str() + res.length());
	this->_response.insert(this->_response.end(), this->_body.begin(), this->_body.end());
	this->_is_build = true;
}
std::vector <unsigned char> &Response::get_response()
{
	return this->_response;
}
void Response::set_default_headers(std::string extension = "")
{
	this->set_header("Server: Webserv (Bob Luke Rowan) 1.0");
	this->set_header("Content-Type: " + ft::MimeTypes().getMimeType(extension));
}

void Response::set_3xx_response(Configuration &config) /* Redirections */
{
	std::map<size_t, std::string> map = config.get_return();
	std::map<size_t, std::string>::iterator it = map.begin();

	this->set_status_code(it->first);
	this->set_header("Location: " + it->second);

	if (DEBUG == 1)
	{
		std::stringstream ss;
		ss << CLRS_YEL;
		ss << "INTERNAL REDIRECT : [target : " << it->second << "]";
		ss << CLRS_reset;
		std::cerr << ss.str() << std::endl;
	}

	this->set_header("Server: Webserv (Bob Luke Rowan) 1.0");
}
void Response::set_405_response(Configuration &config) /* Method Not Allowed */ // Allow: GET, POST, DELETE
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
void Response::set_xxx_response(Configuration &config, int code)
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
	this->set_body(content.c_str(), content.size(), 0);
}

void Response::set_500_response() /* Internal Server Error CLOSE READ ERROR*/
{
	this->set_status_code(500);
	this->set_header("Server: Webserv (Bob Luke Rowan) 1.0");
	this->set_header("Content-Type: text/html");
	std::string error_page_content = "500 " + this->_response_codes[500];
	this->set_body(error_page_content.c_str(), error_page_content.size(), 0);
}
