#include "Request.hpp"		// Request
#include <string>			// std::string
#include <map>				// std::map
#include <vector>			// std::vector
#include "StringUtils.hpp"	// StringUtils
#include <sstream>			// std::stringstream
#include <algorithm>		// std::search
#include <iostream>

/*
 * Default constructor.
 */
Request::Request() : _contentLength(0), _badRequest(false), _doneReading(false)
{
	this->_readMode = Request::NOT_SET;
}

/*
 * Copy constructor.
 */
Request::Request(Request const &src)
{
	*this = src;
}

/*
 * Destructor.
 */
Request::~Request()
{
}

/*
 * Assignment operator.
 */
Request						&Request::operator=(Request const &src)
{
	this->_method = src._method;
	this->_URI = src._URI;
	this->_queryString = src._queryString;
	this->_protocol = src._protocol;
	this->_headers = src._headers;
	this->_body = src._body;
	this->_matchedServerName = src._matchedServerName;
	this->_port = src._port;
	this->_incomingMessage = src._incomingMessage;
	this->_readMode = src._readMode;
	this->_contentLength = src._contentLength;
	this->_badRequest = src._badRequest;
	this->_doneReading = src._doneReading;
	return *this;
}

/*
 * This function will reset a request to its basic settings.
 */
void						Request::reset()
{
	this->_method.clear();
	this->_URI.clear();
	this->_queryString.clear();
	this->_protocol.clear();
	this->_headers.clear();
	this->_body.clear();
	this->_matchedServerName.clear();
	this->_port.clear();
	this->_incomingMessage.clear();
	this->_readMode = Request::NOT_SET;
	this->_contentLength = 0;
	this->_badRequest = false;
	this->_doneReading = false;
}

/*
 * This function returns the method as a RequestMethod enum.
 */
std::string					Request::getMethod() const
{
	return this->_method;
}

/*
 * This function returns the URI of the request.
 */
std::string					Request::getURI() const
{
	return this->_URI;
}

/*
 * This function returns the query string of the request.
 */
std::string					Request::getQueryString() const
{
	return this->_queryString;
}

/*
 * This function returns the protocol of the request.
 */
std::string					Request::getProtocol() const
{
	return this->_protocol;
}

/*
 * This function returns the value belonging to the requested header. Will be
 * an empty string if the header was not set.
 */
std::string					Request::getHeader(std::string const &headerName)
{
	std::string		headerValue;
	std::string		lowerCaseHeaderName;

	for (std::string::size_type i = 0; i < headerName.size(); i++)
		lowerCaseHeaderName += std::tolower(headerName[i]);
	if (this->_headers.count(lowerCaseHeaderName))
		headerValue = this->_headers[lowerCaseHeaderName];
	else
		headerValue = std::string("");
	return headerValue;
}

/*
 * This function returns the body of the request.
 */
std::vector<unsigned char>	&Request::getBody()
{
	return this->_body;
}

/*
 * This funtion returns the matched server name of the request.
 */
std::string					Request::getMatchedServerName() const
{
	return this->_matchedServerName;
}

/*
 * This functions returns the port of the request.
 */
std::string					Request::getPort()
{
	return this->_port;
}

/*
 * This function returns the length of the requested body.
 */
size_t						Request::getBodyLength() const
{
	return this->_body.size();
}

/*
 * This function returns a string version of the body length.
 */
std::string					Request::getBodyLengthString() const
{
	std::stringstream	conversionStream;
	std::string			length;

	conversionStream << this->_body.size();
	conversionStream >> length;
	return length;
}

/*
 * This function returns the message that has been read so far.
 */
std::vector<unsigned char>	&Request::getIncomingMessage()
{
	return this->_incomingMessage;
}

/*
 * This function sets the method of the request.
 */
void						Request::setMethod(std::string const &method)
{
	this->_method = method;
}

/*
 * This function sets the URI of the request.
 */
void						Request::setURI(std::string const &URI)
{
	size_t			questionMarkPosition;

	questionMarkPosition = URI.find("?");
	if (questionMarkPosition != std::string::npos)
		this->setQueryString(URI.substr(questionMarkPosition + 1));
	this->_URI = URI.substr(0, questionMarkPosition);
}

/*
 * This function sets the query string of the request.
 */
void						Request::setQueryString(std::string const &queryString)
{
	this->_queryString = queryString;
}

/*
 * This function sets the protocol of the request.
 */
void						Request::setProtocol(std::string const &protocol)
{
	this->_protocol = protocol;
}

/*
 * This function sets a header in the request.
 */
void						Request::setHeader(std::string const &header)
{
	size_t			colonPosition;
	std::string		headerName;
	std::string		lowerCaseHeaderName;
	std::string		headerValue;

	colonPosition = header.find(":");
	if (colonPosition == std::string::npos)
		return ;
	headerName = header.substr(0, colonPosition);
	for (std::string::size_type i = 0; i < headerName.size(); i++)
		lowerCaseHeaderName += std::tolower(headerName[i]);
	headerValue = header.substr(colonPosition + 2);
	this->_headers[lowerCaseHeaderName] = headerValue;
	std::cout << "Header: " << headerName << " Value: " << headerValue << std::endl;
}

/*
 * This function sets the body of the request.
 */
void						Request::setBody(std::vector<unsigned char>::iterator it, size_t length)
{
	this->_body.insert(this->_body.end(), it, it + length);
}

/*
 * This function sets the matched server name of the request.
 */
void						Request::setMatchedServerName(std::string const &matchedServerName)
{
	this->_matchedServerName = matchedServerName;
}

/*
 * This function sets the port of this request.
 */
void						Request::setPort(int port)
{
	std::stringstream		conversionStream;

	conversionStream << port;
	conversionStream >> this->_port;
}

/*
 * This function adds content to the incoming message, and parses it
 * appropriately.
 */
void						Request::setIncomingMessage(const char *message, int length)
{
	this->_incomingMessage.insert(this->_incomingMessage.end(), message, message + length);
	// Do we still need to read the header?
	if (this->_readMode == Request::NOT_SET)
	{
		char			*charMessage = reinterpret_cast<char*>(&this->_incomingMessage[0]);
		std::string		strMessage(charMessage, this->_incomingMessage.size());
		size_t	headerEndPosition = strMessage.find("\r\n\r\n");
		if (headerEndPosition == std::string::npos)
			// We're not done reading the header.
			return ;

		// Separate the header and parse it.
		std::string		header = strMessage.substr(0, headerEndPosition + 4);
		int		headerSize = header.size();
		this->_parseRequestLine(header);
		this->_parseHeaders(header);
		this->_findReadMode();

		// Remove the characters for the header from the incoming message.
		std::vector<unsigned char>::iterator	it = this->_incomingMessage.begin();
		this->_incomingMessage.erase(it, it + headerSize);

		// Decide what to do with the rest of the body, if there is more.
		if (this->_readMode == Request::CONTENT_LENGTH)
			// misschien niet goed genoeg.
			this->_addContentLength();
		else if (this->_readMode == Request::CHUNKED)
			this->_parseChunk();
		else if (this->_incomingMessage.size() > 0)
		{
			this->_badRequest = true;
			this->_doneReading = true;
		}
		else
			this->_doneReading = true;
	}
	else if (this->_readMode == Request::CONTENT_LENGTH)
		this->_addContentLength();
	else if (this->_readMode == Request::CHUNKED)
		this->_parseChunk();
	else
	{
		// ERROR ?
	}
}

/*
 * This function returns true if we're done reading the request.
 */
bool						Request::isComplete()
{
	return this->_doneReading;
}

/*
 * This function will return true if a mistake has been found while
 * parsing the request.
 */
bool						Request::isBadRequest()
{
	return this->_badRequest;
}

/*
 * This function will return true if the request is chunked.
 */
bool						Request::isChunked()
{
	return (this->_readMode == CHUNKED);
}

/*
 * This function will parse the request line and the headers of the
 * incoming request.
 */
void						Request::_parseHeaders(std::string &allHeaders)
{
	size_t			lineEnd;
	std::string		header;

	while (true)
	{
		lineEnd = 0;
		header = StringUtils::getString(&lineEnd, allHeaders, "\r\n");
		if (lineEnd == 0 || header == "")
			break;
		else
		{
			this->setHeader(header);
			allHeaders = allHeaders.substr(lineEnd);
		}
	}
}

/*
 * This function will parse the request line and return the rest of the
 * argument given back.
 */
void						Request::_parseRequestLine(std::string &line)
{
	size_t			firstSpace;
	size_t			secondSpace;
	size_t			endLine;
	std::string		method;
	std::string		URI;
	std::string		protocol;

	// First, validate if the first line is correctly formatted.
	firstSpace = line.find(" ");
	secondSpace = line.find(" ", firstSpace + 1);
	endLine = line.find("\r\n");

	if (firstSpace < secondSpace && secondSpace < endLine && endLine != std::string::npos)
	{
		// Now separate the line into the three strings.
		method = line.substr(0, firstSpace);
		URI = line.substr(firstSpace + 1, secondSpace - firstSpace - 1);
		protocol = line.substr(secondSpace + 1, endLine - secondSpace - 1);
		this->setMethod(method);
		this->setURI(URI);
		this->setProtocol(protocol);
		line = line.substr(endLine + 2);
	}
	else
		this->_badRequest = true;
}

/*
 * This function finds the read mode of the request after the headers have
 * been parsed.
 */
void						Request::_findReadMode()
{
	if (this->getHeader("Content-Length") != "")
	{
		// Convert the value of the Content-Length header and save it.
		std::string			contentLengthString = this->getHeader("Content-Length");
		std::stringstream	conversionStream;

		conversionStream << contentLengthString;
		conversionStream >> this->_contentLength;
		this->_readMode = Request::CONTENT_LENGTH;
	}
	else if (this->getHeader("Transfer-Encoding") == "chunked")
		this->_readMode = Request::CHUNKED;
	else
		this->_readMode = Request::NO_BODY;
}

/*
 * This function parses data received as part of a HTTP request body.
 */
void						Request::_addContentLength()
{
	this->setBody(this->_incomingMessage.begin(), this->_incomingMessage.size());
	this->_incomingMessage.clear();
	if (this->_body.size() >= this->_contentLength)
	{
		this->_doneReading = true;
		if (this->_body.size() != this->_contentLength)
			this->_badRequest = true;
	}
}

/*
 * This function parses a chunk of a chunked HTTP request.
 */
void						Request::_parseChunk()
{
	size_t	totalChunkLength = 0;
	std::vector<unsigned char>::iterator		it;
	const char									clrf[] = "\r\n";
	while (true)
	{
		// Let's see if we have a complete chunk.
		// First, find the length of the chunk.
		it = std::search(this->_incomingMessage.begin(), this->_incomingMessage.end(),
						clrf, clrf + 2);
		if (it == this->_incomingMessage.end())
			return ;
		// Convert the given number from hex to an int.
		std::stringstream	conversionStream;
		size_t				chunkLength;
		size_t				firstLineEnd = it - this->_incomingMessage.begin();
		std::string			hexString(reinterpret_cast<char*>(&this->_incomingMessage[0]), firstLineEnd);
		conversionStream << std::hex << hexString;
		conversionStream >> chunkLength;
		totalChunkLength += chunkLength;

		// Now check to see if the chunk is long enough, and then if the two
		// characters after that are \r and \n.
		size_t		chunkStartPos = firstLineEnd + 2;
		size_t		chunkEndPos = chunkStartPos + chunkLength;
		if (this->_incomingMessage.size() < chunkEndPos + 2)
			return ;
		if (this->_incomingMessage[chunkEndPos] != '\r' ||
				this->_incomingMessage[chunkEndPos + 1] != '\n')
		{
			this->_badRequest = true;
			this->_doneReading = true;
			return ;
		}
		if (chunkLength == 0)
		{
			this->_doneReading = true;
			return ;
		}
		// Set the body with the content of the chunk, then remove the chunk
		// from the incomingMessage variable.
		it = this->_incomingMessage.begin() + chunkStartPos;
		this->setBody(this->_incomingMessage.begin() + chunkStartPos, chunkLength);
		this->_incomingMessage.erase(this->_incomingMessage.begin(), this->_incomingMessage.begin() + chunkEndPos + 2);
	}
}
