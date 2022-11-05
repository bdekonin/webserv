/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Request.cpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: rkieboom <rkieboom@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/11/05 11:35:09 by rkieboom      #+#    #+#                 */
/*   Updated: 2022/11/05 12:09:17 by rkieboom      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Request.hpp"

Request::Request()
: _method(this->UNSET), _uri(""), _version(""), _body(std::vector<unsigned char>()), _type(this->NOT_SET), _incoming_data(std::vector<unsigned char>()), _content_length(0), was_chunked(false)
{
	this->_headers_map = std::map<std::string, std::string>();
}

/* Destructor */
Request::~Request()
{
}

/* Copy constructor */
Request::Request(const Request *e)
{
	*this = e;
}

/* Operation overload = */
Request& Request::operator = (const Request *e)
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
void Request::add_incoming_data(char *incoming_buffer, size_t len)
{
	this->_incoming_data.insert(this->_incoming_data.end(), incoming_buffer, incoming_buffer + len);
	if (this->_type == this->NOT_SET)
	{
		std::string request(reinterpret_cast<char *>(&this->_incoming_data[0]), this->_incoming_data.size());

		size_t start_body = request.find("\r\n\r\n");
		if (start_body == std::string::npos) // keep reading headers
		{
			if (this->_incoming_data.size() > MAX_ENTITIY_SIZE)
				this->_type = this->MAX_ENTITY;

			if (request.find("\v") != std::string::npos) //  Moet dit?
			{
				this->_type = this->ERROR;
				return ;
			}
			if (request.find("\t") != std::string::npos) // Moet dit??
			{
				this->_type = this->ERROR;
				return ;
			}
			return ;
		}

		std::string		headers = request.substr(0, start_body + 4);
		size_t			headers_size = headers.size();

		this->_request_line(headers);
		this->_headers(headers);
		this->_reading_mode();
		
		if (this->_type == this->ERROR || this->_type == this->MAX_ENTITY)
			return ;

		std::vector<unsigned char>::iterator it = this->_incoming_data.begin();

		// Incoming data is now only the body
		this->_incoming_data.erase(it, it + headers_size);

		if (this->_type == this->UNMATCHED_CONTENT_LENGTH)
			this->add_body();
		else if (this->_type == this->ENCODING_CHUNKED)
			_parseChunk();
		else if (this->_incoming_data.size() > 0)
		{
			this->_type = this->DONE;
			this->_type = this->ERROR;
		}
		else
			this->_type = this->DONE;
	}
	else if (this->_type == this->UNMATCHED_CONTENT_LENGTH)
		this->add_body();
	else if (this->_type == this->ENCODING_CHUNKED)
		_parseChunk();
	else
		this->_type = this->ERROR;

	if (this->_content_length > 0)
		this->_headers_map["content-length"] = std::to_string(this->_content_length);
}

void						Request::_parseChunk()
{
	size_t	totalChunkLength = 0;
	(void) totalChunkLength;
	std::vector<unsigned char>::iterator		it;
	while (true)
	{

		it = std::search(this->_incoming_data.begin(), this->_incoming_data.end(),
						"\r\n", &"\r\n"[2]);
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
			this->_type = this->ERROR;
			break;
		}
		if (chunkLength == 0)
		{
			this->_type = this->DONE;
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

bool Request::is_complete()
{
	if (this->_type == this->DONE)
		return true;
	return false;
}

void Request::clear()
{
	this->_headers_map.clear();
	this->_method = this->UNSET;
	this->_uri.clear();
	this->_version.clear();
	this->_body.clear();
	this->_unedited_uri.clear();
	this->_type = this->NOT_SET;
	this->_query_string.clear();
}

bool Request::is_empty() const
{
	if (this->_method == this->UNSET && this->_uri == "" && this->_version == "" && this->_headers_map.empty())
		return true;
	else
		return false;
}

bool Request::is_method_get() const
{
	return this->_method == this->GET;
}
bool Request::is_method_post() const
{
	return this->_method == this->POST;
}
bool Request::is_method_delete() const
{
	return this->_method == this->DELETE;
}
bool Request::is_method_unset() const
{
	return this->_method == this->UNSET;
}
bool Request::is_method_unsupported() const
{
	return this->_method == this->UNSUPPORTED;
}
const std::string &Request::get_unedited_uri() const
{
	return this->_unedited_uri;
}

bool				Request::is_http_supported()
{
	return this->_version == "HTTP/1.1";
}
bool				Request::is_bad_request()
{
	if (this->_type == this->ERROR)
		return true;

	if (this->_method == this->UNSUPPORTED)
		return true;

	if (this->get_header("host") == "")
		return true;

	if (this->_uri == "")
		return true;

	if (this->_version == "")
		return true;
	
	if (this->_method == this->POST)
	{
		if (this->get_header("content-length") == "")
			return true; 

		if (this->_type != this->DONE)
			return true;
		
		// Should not go here

		if (this->_type == this->NO_BODY) 
			return true;

		if (this->_type == this->UNMATCHED_CONTENT_LENGTH)
			return true;
	}

	return false;
}
const char 			*Request::type_i_to_s(const int i) const
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
const char 			*Request::type_enum_to_s(Type type) const
{
	if (type == this->NOT_SET)
		return "NOT_SET";
	else if (type == this->NO_BODY)
		return "NO_BODY";
	else if (type == this->UNMATCHED_CONTENT_LENGTH)
		return "UNMATCHED_CONTENT_LENGTH";
	else if (type == this->ENCODING_CHUNKED)
		return "ENCODING_CHUNKED";
	else if (type == this->DONE)
		return "DONE";
	return "ERROR";
}
const char 			*Request::type_to_s() const
{
	if (this->_type == this->NOT_SET)
		return "NOT_SET";
	else if (this->_type == this->NO_BODY)
		return "NO_BODY";
	else if (this->_type == this->UNMATCHED_CONTENT_LENGTH)
		return "UNMATCHED_CONTENT_LENGTH";
	else if (this->_type == this->ENCODING_CHUNKED)
		return "ENCODING_CHUNKED";
	else if (this->_type == this->DONE)
		return "DONE";
	return "ERROR";
}
Request::Type			Request::get_type() const
{
	return this->_type;
}

const char 			*Request::method_i_to_s(const int i) const
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
const char 			*Request::method_enum_to_s(Method method) const
{
	if (method == this->UNSET)
		return "UNSET";
	else if (method == this->GET)
		return "GET";
	else if (method == this->POST)
		return "POST";
	else if (method == this->DELETE)
		return "DELETE";
	return "UNSUPPORTED";
}
const char 			*Request::method_to_s() const
{
	if (this->_method == this->UNSET)
		return "UNSET";
	else if (this->_method == this->GET)
		return "GET";
	else if (this->_method == this->POST)
		return "POST";
	else if (this->_method == this->DELETE)
		return "DELETE";
	return "UNSUPPORTED";
}

Request::Method 		Request::get_method() const
{
	return this->_method;
}
std::string	&Request::get_header(const std::string &header)
{
	return this->_headers_map[header];
}

void Request::set_method(const Method method)
{
	this->_method = method;
}
void Request::set_method(std::string &method)
{
	if (method == "GET")
		this->_method = this->GET;
	else if (method == "POST")
		this->_method = this->POST;
	else if (method == "DELETE")
		this->_method = this->DELETE;
	else
		this->_method = this->UNSUPPORTED;
}

void Request::_request_line(std::string &line)
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

		{
			this->_query_string = uri.find("?") != std::string::npos ? uri.substr(uri.find("?") + 1) : "";
			this->_uri = uri.find("?") != std::string::npos ? uri.substr(0, uri.find("?")) : uri;
			while(this->_uri.find("%20") != std::string::npos)
				this->_uri.replace(this->_uri.find("%20"), 3, " ");
			this->_unedited_uri = this->_uri;
		}
		this->_version = version;
		line = line.substr(end + 2);
	}
	else
		this->_type = this->ERROR;
}

void Request::_headers(std::string &line)
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
			if (line[colon + 1] == ' ')
				value = line.substr(colon + 2, end - colon - 2);
			else
				value = line.substr(colon + 1, end - colon - 1);
			// value = line.substr(colon + 2, end - colon - 2);

			for (size_t i = 0; i < header.length(); i++)
				header[i] = std::tolower(header[i]);

			if (header.find(' ') != std::string::npos)
				this->_type = this->ERROR;
			else
			{
				if (header.empty() || value.empty())
					this->_type = this->ERROR;
				else
				{
					if (this->_headers_map[header] != "")
						this->_type = this->ERROR;
					else
					{
						if (header == "content-length")
							this->check_content_length(value);
						else if (header == "host")
							this->check_host(value);

						if (this->_type != this->ERROR)
							this->_headers_map[header] = value;
					}
				}
			}
		}
		line = line.substr(end + 2);
	}
}
void Request::_reading_mode()
{
	std::string value;

	if (this->_type == this->ERROR)
		return ;
	if ((value =this->get_header("content-length")) != "")
	{
		this->_content_length = atoi(value.c_str());
		if (this->get_header("transfer-encoding") == "chunked")
			this->_type = this->ERROR;
		else if (this->_content_length > MAX_ENTITIY_SIZE)
			this->_type = this->MAX_ENTITY;
		else
			this->_type = this->UNMATCHED_CONTENT_LENGTH;
	}
	else if (this->get_header("transfer-encoding") == "chunked")
	{
		if (this->get_header("content-length") != "")
			this->_type = this->ERROR;
		else
			this->_type = this->ENCODING_CHUNKED;
	}
	else
		this->_type = this->NO_BODY;
}


void Request::add_body()
{
	size_t bodySize;

	this->setBody(this->_incoming_data.begin(), this->_incoming_data.size());
	this->_incoming_data.clear();
	bodySize = this->_body.size();
	if (bodySize >= this->_content_length)
	{
		this->_type = this->DONE;
		if (bodySize != this->_content_length)
			this->_type = this->ERROR;
	}
}
void	Request::setBody(std::vector<unsigned char>::iterator it, size_t length)
{
	this->_body.insert(this->_body.end(), it, it + length);
}

void Request::check_content_length(std::string &value)
{
	for (size_t i = 0; i < value.length(); i++)
		if (!isdigit(value[i]))
			this->_type = this->ERROR;
}

void Request::check_host(std::string &value)
{
	for (size_t i = 0; i < value.length(); i++)
	{
		if (value[i] == ':')
		{
			i++;
			for (; i < value.length(); i++)
			{
				if (!isdigit(value[i]))
				{
					this->_type = this->ERROR;
					return ;
				}
			}
			if (i == value.length())
				return ;
		}
		if (isalnum(value[i]) || value[i] == '.')
			continue ;
		else
			this->_type = this->ERROR;
	}
}