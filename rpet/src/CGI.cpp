#include "CGI.hpp"	// CGI
#include <string>	// std::string

/*
 * Default constructor
 */
CGI::CGI()
{
}

/*
 * Copy constructor
 */
CGI::CGI(CGI const &src)
{
	*this = src;
}

/*
 * Assignment operator
 */
CGI		&CGI::operator=(CGI const &src)
{
	this->fdin[0] = src.fdin[0];
	this->fdin[1] = src.fdin[1];
	this->fdout[0] = src.fdout[0];
	this->fdout[1] = src.fdout[1];
	this->_responseBody = src._responseBody;
	return *this;
}

/*
 * Destructor
 */
CGI::~CGI()
{
}

/*
 * This function returns the body for the CGI
 */
std::string const	&CGI::getResponseBody()
{
	return this->_responseBody;
}

/*
 * This function sets the body for the CGI
 */
void				CGI::setResponseBody(std::string const &body)
{
	this->_responseBody.append(body);
}
