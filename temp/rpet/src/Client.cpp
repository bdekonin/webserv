#include "Client.hpp"			// Client
#include "Request.hpp"			// Request
#include "Response.hpp"			// Response
#include <strings.h>			// bzero
#include <arpa/inet.h>			// inet
#include <string>				// std::string

/*
 * Default constructor.
 */
Client::Client() : _request(Request()), _response(Response())
{
}

/*
 * Copy constructor.
 */
Client::Client(Client const &src)
{
	*this = src;
}

/*
 * Assignment operator.
 */
Client			&Client::operator=(Client const &src)
{
	this->fd = src.fd;
	this->_request = src._request;
	this->_response = src._response;
	this->_address = src._address;
	return *this;
}

/*
 * This function will reset the client and it's request and response.
 */
void			Client::reset()
{
	this->_request.reset();
	this->_response.reset();
	this->_address.clear();
}

/*
 * Destructor.
 */
Client::~Client()
{
}

/*
 * Parameterized constructor.
 */
Client::Client(int socketFD, struct sockaddr_in *addressInfo) : fd(socketFD), _request(Request()), _response(Response())
{
	char	addressBuffer[1024];

	bzero(addressBuffer, 1024);
	inet_ntop(AF_INET, &addressInfo->sin_addr, addressBuffer, 1024);
	this->_address.append(addressBuffer);
	printf("Client Connected from [%s]\n", addressBuffer);
}

/*
 * This function returns the request.
 */
Request				&Client::getRequest()
{
	return this->_request;
}

/*
 * This function returns the response.
 */
Response			&Client::getResponse()
{
	return this->_response;
}

/*
 * This function returns the address.
 */
std::string			&Client::getAddress()
{
	return this->_address;
}

/*
 * This function checks if the request is bad, and updates the response accordingly.
 */
void				Client::checkBadRequest()
{
	if (this->_request.isBadRequest() || this->_request.getHeader("Host") == "")
		this->_response.setStatusCode(400);
}
