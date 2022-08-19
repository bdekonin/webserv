#include "Config.hpp"				// Config
#include "StringUtils.hpp"			// StringUtils
#include <string>					// std::string
#include <vector>					// std::vector
#include <map>						// std::map
#include <sstream>					// std::stringstream
#include <stdexcept>				// std::runtime_error

Config::~Config()
{
}

Config::Config() : _limitClientBodySize(0)
{
	_httpMethods[GET] = false;
	_httpMethods[POST] = false;
	_httpMethods[DELETE] = false;
	_autoindex = false;
}

Config::Config(const Config& src)
{
	*this = src;
}

Config&	Config::operator=(const Config& src)
{
	this->_limitClientBodySize = src.getLimitClientBodySize();
	this->_root = src.getRoot();
	this->_index = src.getIndex();
	this->_autoindex = src.getAutoindex();
	this->_httpMethods[GET] = src.getHttpMethods(GET);
	this->_httpMethods[POST] = src.getHttpMethods(POST);
	this->_httpMethods[DELETE] = src.getHttpMethods(DELETE);
	this->_amountCGI = src.getAmountCGI();
	this->_CGI = src.getMapCGI();
	this->_uploadDir = src.getUploadDir();
	this->_amountErrorpages = src.getAmountErrorPages();
	this->_errorPage = src.getMapErrorPages();
	this->_redirection = src.getRedirection();
	return *this;
}

size_t  Config::getLimitClientBodySize() const
{
	return this->_limitClientBodySize;
}

const std::string&	Config::getRoot() const
{
	return this->_root;
}

const std::vector<std::string>&	    Config::getIndex() const
{
	return this->_index;
}

bool    Config::getAutoindex() const
{
	return this->_autoindex;
}

bool	Config::isValidHttpMethod(const std::string& method) const
{
	if (method == "GET")
		return this->_httpMethods[GET];
	if (method == "POST")
		return this->_httpMethods[POST];
	if (method == "DELETE")
		return this->_httpMethods[DELETE];
	return false;
}

const std::string&	Config::getErrorPage(size_t errorNumber) const
{
	std::map<size_t, std::string>::const_iterator	it = this->_errorPage.find(errorNumber);
	return it->second;
}

bool	Config::hasErrorPage(size_t errorNumber) const
{
	std::map<size_t, std::string>::const_iterator	it = this->_errorPage.find(errorNumber);
	if (it == this->_errorPage.end())
		return false;
	return true;
}

const std::string*	Config::lookUpCGI(const std::string& string) const
{
	std::map<std::string, std::string>::const_iterator	it = this->_CGI.begin();
	std::map<std::string, std::string>::const_iterator	itEnd = this->_CGI.end();

	while (it != itEnd)
	{
		std::string tmp = it->first;
		size_t	len = tmp.length();
		if (string.length() < len)
		{
			it++;
			continue;
		}
		size_t	start = string.length() - len;
		std::string substr = string.substr(start, len);
		if (substr == tmp)
			return &it->second;
		it++;
	}
	return NULL;
}

bool	Config::hasCGI(const std::string& string) const
{
	return lookUpCGI(string);
}

const std::string	Config::getCGI(const std::string& string) const
{
	const std::string*	ptr = lookUpCGI(string);
	if (ptr)
		return *ptr;
	return "";
}

bool	Config::hasUploadDir() const
{
	return this->_uploadDir != "";
}

const std::string&  Config::getUploadDir() const
{
	return this->_uploadDir;
}

size_t	Config::getAmountErrorPages() const
{
	return this->_amountErrorpages;
}

size_t	Config::getAmountCGI() const
{
	return this->_amountCGI;
}

const std::map<size_t, std::string>&	Config::getMapErrorPages() const
{
	return this->_errorPage;
}

const std::map<std::string, std::string>&	Config::getMapCGI() const
{
	return this->_CGI;
}

bool	Config::getHttpMethods(enum	e_httpMethods method) const
{
	return this->_httpMethods[method];
}

void	Config::setLimitClientBodySize(const std::string& str)
{
	std::stringstream			ss;
	size_t						maxBodySize;
	std::vector<std::string>	vec;
	size_t						multiplyBytes = 1;
	const size_t				max = 0 - 1;

	StringUtils::split(str, Whitespaces, vec);
	if (vec.size() == 1)
	{
		this->_limitClientBodySize = 0;
		return;
	}
	else if (vec.size() > 2)
		throw std::runtime_error("configfile: bad formatting client_max_body_size");
	size_t pos = vec[1].find_first_not_of("0123456789", 0);
	if (pos != std::string::npos)
	{
		if (vec[1][pos] == 'k')
			multiplyBytes = 1000;
		else if (vec[1][pos] == 'm')
			multiplyBytes = 1000000;
		else if (vec[1][pos] == 'g')
			multiplyBytes = 1000000000;
		else
			throw std::runtime_error("configfile: client_max_body_size must contain digits only");
		pos++;
		if (vec[1][pos] != '\0')
			throw std::runtime_error("configfile: client_max_body_size must contain digits only");
	}
	ss << vec[1];
	ss >> maxBodySize;
	this->_limitClientBodySize = maxBodySize * multiplyBytes;
	if (this->_limitClientBodySize < maxBodySize)
		this->_limitClientBodySize = max;
}

void	Config::setRoot(const std::string& str)
{
	std::vector<std::string> vec;
	StringUtils::split(str, Whitespaces, vec);
	if (vec.size() != 2)
		throw std::runtime_error("configfile: root bad formatting");
	this->_root = vec[1];
}

void	Config::setIndex(const std::string& str)
{
	std::vector<std::string>	vec;

	StringUtils::split(str, Whitespaces, vec);
	if (vec.size() == 1)
	{
		this->_index.reserve(1);
		this->_index[0] = "";
		return;
	}
	else
	{
		vec.erase(vec.begin());
		this->_index = vec;
	}
}

void	Config::setAutoindex(const std::string& str)
{
	std::vector<std::string> vec;

	StringUtils::split(str, Whitespaces, vec);
	vec.erase(vec.begin());
	for (size_t j = 0; j < vec[0].size(); j++)
		vec[0][j] = std::tolower(vec[0][j]);
	if (vec[0] == "on")
		this->_autoindex = true;
	else if (vec[0] == "off")
		this->_autoindex = false;
	else
		throw std::runtime_error("configfile: bad autoindex");
}

void	Config::setHttpMethods(const std::string& str)
{
	std::vector<std::string>	vec;

	StringUtils::split(str, Whitespaces, vec);
	if (vec.size() == 1)
		return;
	for (size_t i = 1; i < vec.size(); i++)
	{
		for (size_t j = 0; j < vec[i].size(); j++)
			vec[i][j] = std::toupper(vec[i][j]);
		if (vec[i] == "GET")
			this->_httpMethods[GET] = true;
		else if (vec[i] == "POST")
			this->_httpMethods[POST] = true;
		else if (vec[i] == "DELETE")
			this->_httpMethods[DELETE] = true;
		else
			throw std::runtime_error("configfile: unknown httpmethod");
	}
}

void	Config::setCGI(const std::string& str)
{
	std::vector<std::string> vec;

	StringUtils::split(str, Whitespaces, vec);
	if (vec.size() != 3)
		throw std::runtime_error("configfile: CGI bad formatting");
	this->_CGI[vec[1]] = vec[2];
}

void	Config::setUploadDir(const std::string& str)
{
	std::vector<std::string> vec;

	StringUtils::split(str, Whitespaces, vec);
	if (vec.size() != 2)
		throw std::runtime_error("configfile: upload dir bad formatting");
	this->_uploadDir = vec[1];
}

void	Config::setErrorPage(const std::string& str)
{
	std::vector<std::string>	vec;
	std::stringstream			ss;
	size_t						errorNum;

	StringUtils::split(str, Whitespaces, vec);
	if (vec.size() != 3)
		throw std::runtime_error("configfile: bad formatting errorpage");
	ss << vec[1];
	ss >> errorNum;
	this->_errorPage[errorNum] = vec[2];
}

bool	Config::isRedirection() const
{
	return this->_redirection != "";
}

const std::string&	Config::getRedirection() const
{
	return this->_redirection;
}

void	Config::setRedirection(const std::string& str)
{
	std::vector<std::string> vec;

	StringUtils::split(str, Whitespaces, vec);
	if (vec.size() != 3)
		throw std::runtime_error("configfile: bad formatting redirection");
	if (vec[1] != "301")
		throw std::runtime_error("configfile: redirection code must be 301");
	this->_redirection = vec[2];
}

std::ostream&	operator<<(std::ostream& out, const Config& con)
{
	out << "BodySize: " << con.getLimitClientBodySize() << std::endl;
	out << "Root    : " << con.getRoot() << std::endl;
	out << "indexVec: "; for (size_t i = 0; i < con.getIndex().size(); i++){out << con.getIndex()[i] << " ";} out << std::endl;
	out << "autoindx: " << con.getAutoindex() << std::endl;
	out << "Errorpgs:\n"; for (std::map<size_t, std::string>::const_iterator it = con.getMapErrorPages().begin(); it != con.getMapErrorPages().end(); it++){out << "\t\terr: " << it->first << " " << it->second << std::endl;}
	out << "httpGET : " << con.getHttpMethods(Config::GET) << std::endl;
	out << "httpPOST: " << con.getHttpMethods(Config::POST) << std::endl;
	out << "httpDEL : " << con.getHttpMethods(Config::DELETE) << std::endl;
	out << "CGI all :\n"; for (std::map<std::string, std::string>::const_iterator it = con.getMapCGI().begin(); it != con.getMapCGI().end(); it++){out << "\t\tcgi: " << it->first << " " << it->second << std::endl;}
	out << "Upld Dir: " << con.getUploadDir() << std::endl;
	out << "AmntErrs: " << con.getAmountErrorPages() << std::endl;
	out << "AmntCGIs: " << con.getAmountCGI() << std::endl; 
	out << "Redir   : " << con.getRedirection() << std::endl;
	return out;
}
