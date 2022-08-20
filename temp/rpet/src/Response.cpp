#include "Response.hpp"	// Response
#include <sstream>		// std::stringstream

/*
 * Default constructor.
 */
Response::Response() : _statusCode(200), _defaultError(false), _responseBuilt(false),
	_bytesSent(0)
{
}

/*
 * Copy constructor.
 */
Response::Response(Response const &src)
{
	*this = src;
}

/*
 * Assignment operator.
 */
Response		&Response::operator=(Response const &src)
{
	this->_body = src._body;
	this->_headers = src._headers;
	this->_statusCode = src._statusCode;
	this->_defaultError = src._defaultError;
	this->_responseBuilt = src._responseBuilt;
	this->_bytesSent = src._bytesSent;
	return *this;
}

/*
 * Destructor.
 */
Response::~Response()
{
}

/*
 * This function will reset a response to its basic settings.
 */
void						Response::reset()
{
	this->_body.clear();
	this->_headers.clear();
	this->_statusCode = 200;
	this->_defaultError = false;
	this->_responseText.clear();
	this->_responseBuilt = false;
	this->_bytesSent = 0;
}


/*
 * This function sets the _body variable.
 */
void						Response::setBody(std::string const &body)
{
	this->_body.insert(this->_body.end(), body.c_str(), body.c_str() + body.length());
}

/*
 * This function sets the _body variable from a vector<unsigned char>.
 */
void						Response::setBody(const char *body, int length)
{
	this->_body.insert(this->_body.end(), body, body + length);
}


/*
 * This function sets a header for the response.
 */
void						Response::setHeader(std::string const &header)
{
	this->_headers += header + "\r\n";
}

/*
 * This function sets the status code for the response.
 */
void						Response::setStatusCode(int code)
{
	this->_statusCode = code;
}

/*
 * This function sets the _defaultError variable to true.
 */
void						Response::setDefaultError()
{
	this->_defaultError = true;
}

/*
 * This function updates the total amount of bytes sent.
 */
void						Response::setBytesSent(size_t bytesSent)
{
	this->_bytesSent += bytesSent;
}

/*
 * This function returns the body of the response.
 */
std::vector<unsigned char>	Response::getBody()
{
	return this->_body;
}

/*
 * This function builds and returns the headers for the response.
 */
std::string					Response::getHeaders()
{
	return this->_headers;
}

/*
 * This function returns the status code for the response.
 */
int							Response::getStatusCode()
{
	return this->_statusCode;
}

/*
 * This function returns the response text.
 */
std::vector<unsigned char>	Response::getResponseText()
{
	return this->_responseText;
}

/*
 * This function returns the amount of bytes that were already sent.
 */
size_t						Response::getBytesSent()
{
	return this->_bytesSent;
}

/*
 * This function returns true if the _defaultError variable has been set.
 */
bool						Response::isDefaultError()
{
	return this->_defaultError;
}

/*
 * A boolean function that returns true when the status code is an error.
 */
bool						Response::isError()
{
	if (this->_statusCode == 200)
		return false;
	return true;
}

/*
 * This function returns a boolean indicating if the response has been built.
 */
bool						Response::isResponseBuilt()
{
	return this->_responseBuilt;
}

/*
 * This function returns a boolean indicating whether the entire response has been sent.
 */
bool						Response::isSentEntirely()
{
	return (this->_bytesSent == this->_responseText.size());
}

#include <iostream>
/*
 * This function formats and returns the text that will be sent to the client.
 */
void						Response::buildResponseText(std::string const &statusText)
{
	std::stringstream			resultStream;
	std::string					result;

	if (this->_responseBuilt)
		return ;

	resultStream << "HTTP/1.1 " << this->_statusCode << " " << statusText << "\r\n";
	resultStream << this->_headers << "\r\n";
	result = resultStream.str();
	this->_responseText.insert(this->_responseText.end(), result.c_str(), result.c_str() + result.length());
	this->_responseText.insert(this->_responseText.end(), this->_body.begin(), this->_body.end());
	this->_responseBuilt = true;
	// print this->_responseText
	// loop through vector
	for (std::vector<unsigned char>::iterator it = this->_responseText.begin(); it != this->_responseText.end(); ++it)
		std::cout << *it;
}
