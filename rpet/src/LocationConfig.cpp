#include "Config.hpp"				// Config cast
#include "LocationConfig.hpp"		// LocationConfig
#include "ServerConfig.hpp"			// Constructor
#include "StringUtils.hpp"			// StringUtils
#include <string>					// std::string
#include <vector>					// std::vector
#include <stdexcept>				// std::runtime_error

typedef void (LocationConfig::*setFunction)(const std::string&);

static std::map<std::string, setFunction>	initFunctionPointers()
{
	std::map<std::string, setFunction>	functionPointerMap;

	functionPointerMap["client_max_body_size"] = &ServerConfig::setLimitClientBodySize;
	functionPointerMap["root"] = &ServerConfig::setRoot;
	functionPointerMap["index"] = &ServerConfig::setIndex;
	functionPointerMap["autoindex"] = &ServerConfig::setAutoindex;
	functionPointerMap["limit_except"] = &ServerConfig::setHttpMethods;
	functionPointerMap["cgi"] = &ServerConfig::setCGI;
	functionPointerMap["upload"] = &ServerConfig::setUploadDir;
	functionPointerMap["error_page"] = &ServerConfig::setErrorPage;
	functionPointerMap["return"] = &ServerConfig::setRedirection;
	return functionPointerMap;
}

static std::map<std::string, setFunction>	functionPointers = initFunctionPointers();

LocationConfig::~LocationConfig()
{
}

LocationConfig::LocationConfig()
{
}

LocationConfig::LocationConfig(const LocationConfig* src)
{
	*this = *src;
}

LocationConfig::LocationConfig(const LocationConfig& src)
{
	*this = src;
}

LocationConfig::LocationConfig(const ServerConfig& src)
{
	*this = src;
}

LocationConfig&	LocationConfig::operator=(const ServerConfig& src)
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

LocationConfig&	LocationConfig::operator=(const LocationConfig& src)
{
	this->_path = src._path;
	this->_limitClientBodySize = src.getLimitClientBodySize();
	this->_root = src.getRoot();
	this->_index = src.getIndex();
	this->_autoindex = src.getAutoindex();
	this->_errorPage = src.getMapErrorPages();
	this->_httpMethods[GET] = src.getHttpMethods(GET);
	this->_httpMethods[POST] = src.getHttpMethods(POST);
	this->_httpMethods[DELETE] = src.getHttpMethods(DELETE);
	this->_CGI = src.getMapCGI();
	this->_uploadDir = src.getUploadDir();
	this->_amountErrorpages = src.getAmountErrorPages();
	this->_amountCGI = src.getAmountCGI();
	this->_redirection = src.getRedirection();
	return *this;
}

void	LocationConfig::call(const std::string& identifier, const std::string& argument)
{
	setFunction function = functionPointers[identifier];
	if (function == NULL)
	{
		std::cerr << "id: " << identifier << std::endl;
		throw std::runtime_error("configfile: unknown identifier");
	}
	(this->*function)(argument);
}

void	LocationConfig::init(const std::string& data)
{
	std::vector<std::string>	tokens;
	std::vector<std::string>	lines;

	StringUtils::split(data, Whitespaces, tokens);
	if (tokens.size() != 3)
		throw std::runtime_error("configfile: location has bad formatting");
	this->_path = tokens[1];
	StringUtils::trimBraces(tokens[2]);
	StringUtils::split(tokens[2], ";", lines);
	for (size_t i = 0; i < lines.size(); i++)
	{
		std::string identifier;
		StringUtils::matchIdentifier(lines[i], identifier);
		StringUtils::lowerCase(identifier);
		this->call(identifier, lines[i]);
	}
	this->_amountErrorpages = _errorPage.size();
	this->_amountCGI = _CGI.size();
}

void	LocationConfig::setPath(const std::string& str)
{
	this->_path = str;
}

const std::string&	LocationConfig::getPath() const
{
	return this->_path;
}

std::ostream&	operator<<(std::ostream& out, const LocationConfig& location)
{
	out << "LocaPath: " << location.getPath() << std::endl;
	out << static_cast<Config>(location);
	return out;
}
