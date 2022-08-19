#include "ServerConfig.hpp"			// ServerConfig
#include "Config.hpp"				// Config
#include "StringUtils.hpp"			// StringUtils
#include <sstream>					// std::stringstream
#include <map>						// std::map
#include <vector>					// std::vector
#include <stdexcept>				// std::runtime_error

typedef void (ServerConfig::*setFunction)(const std::string&);

static std::map<std::string, setFunction>	initFunctionPointers()
{
	std::map<std::string, setFunction>	functionPointerMap;
	functionPointerMap["location"] = &ServerConfig::setLocationConfig;
	functionPointerMap["listen"] = &ServerConfig::setPort;
	functionPointerMap["server_name"] = &ServerConfig::setServerName;

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

std::map<std::string, setFunction>	functionPointers = initFunctionPointers();


ServerConfig::~ServerConfig()
{
}

ServerConfig::ServerConfig() : Config::Config()
{
}

ServerConfig::ServerConfig(ServerConfig const& src) : Config::Config(src)
{
	*this = src;
}

ServerConfig& ServerConfig::operator=(const ServerConfig& src)
{
	this->_amountLocations = src.getAmountLocations();
	this->_defaultServerName = src.getDefaultServerName();
	this->_portNumber = src.getPort();
	this->_serverNames = src.getServerNames();
	this->_locations = src._locations;
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

void	ServerConfig::call(const std::string& identifier, const std::string& argument)
{
	setFunction function = functionPointers[identifier];
	if (function == NULL)
	{
		std::cerr << "id: " << identifier << std::endl;
		throw std::runtime_error("configfile: unknown identifier");
	}
	(this->*function)(argument);
}

void	ServerConfig::init(std::string& str)
{
	std::vector<std::string>	lines;
	std::string					identifier;

	StringUtils::trimBraces(str);
	StringUtils::split(str, ";", lines);
	for (size_t i = 0; i < lines.size(); i++)
	{
		StringUtils::matchIdentifier(lines[i], identifier);
		StringUtils::lowerCase(identifier);
		if (!identifier.length())
			continue;
		this->call(identifier, lines[i]);
	}
	this->setDefaultServerName();
	this->_amountLocations = _locations.size();
	this->_amountErrorpages = _errorPage.size();
	this->_amountCGI = _CGI.size();
}

void	ServerConfig::setDefaultServerName()
{
	if (getServerNames().size() == 0)
		throw std::runtime_error("configfile: no default servername found");
	this->_defaultServerName = getServerNames()[0];
}

void	ServerConfig::setPort(const std::string& str)
{
	std::vector<std::string> vec;

	StringUtils::split(str, Whitespaces, vec);
	for (size_t i = 1; i < vec.size(); i++)
	{
		std::stringstream	ss;
		size_t				num;
		size_t 				pos = vec[i].find_first_not_of("1234567890", 0);

		if (pos != std::string::npos)
			throw std::runtime_error("configfile: port must contain digits only");
		ss << vec[i];
		ss >> num;
		if (num > 65535)
			throw std::runtime_error("configfile: port input greater than 65535");
		this->_portNumber.push_back(num);
	}
}

void	ServerConfig::setServerName(const std::string& str)
{
	std::vector<std::string> vec;

	StringUtils::split(str, Whitespaces, vec);
	if (vec.size() <= 1)
		vec.push_back("");
	vec.erase(vec.begin());
	this->_serverNames = vec;
}

void	ServerConfig::setLocationConfig(const std::string& str)
{
	std::vector<std::string>	vec;
	LocationConfig				locationConfig;

	StringUtils::split(str, Whitespaces, vec);
	if (vec.size() != 3)
		throw std::runtime_error("configfile: location bad formatting");
	locationConfig.setPath(vec[1]);
	locationConfig = *this;
	locationConfig.init(str);
	this->_locations.insert(std::pair<std::string, LocationConfig>(vec[1], locationConfig));
}

const std::string&	ServerConfig::getDefaultServerName() const
{
	return this->_defaultServerName;
}

const std::vector<size_t>&	ServerConfig::getPort() const
{
	return this->_portNumber;
}

const std::vector<std::string>&	ServerConfig::getServerNames() const
{
	return this->_serverNames;
}

static size_t	getMatchLength(const std::string& URI, const std::string& locationPath)
{
	size_t	matchLength = 0;

	if (locationPath.size() > URI.size())
		return 0;
	for (size_t i = 0; i < locationPath.size(); i++)
	{
		if (locationPath[i] != URI[i])
			return 0;
		matchLength++;
	}
	return matchLength;
}

const LocationConfig*	ServerConfig::getLocationConfig(const std::string& path) const
{
	std::map<std::string, LocationConfig>::const_iterator	it = this->_locations.begin();
	const std::map<std::string, LocationConfig>::const_iterator	itend = this->_locations.end();
	const LocationConfig	*bestMatch = NULL;
	size_t					bestMatchLength = 0;

	while (it != itend)
	{
		size_t	matchLength = getMatchLength(path, it->first);
		if (matchLength > bestMatchLength)
			bestMatch = &it->second;
		it++;
	}
	return bestMatch;
}

size_t	ServerConfig::getAmountLocations() const
{
	return this->_amountLocations;
}

const std::map<std::string, LocationConfig>& ServerConfig::getMapLocations() const
{
	return this->_locations;
}

std::ostream&	operator<<(std::ostream& out, const ServerConfig& server)
{
	out << "Dfltname: " << server.getDefaultServerName() << std::endl;
	out << "AmntLocs: " << server.getAmountLocations() << std::endl;
	out << "Port nrs: "; for (size_t i = 0; i < server.getPort().size(); i++){out << server.getPort()[i] << " ";} out << std::endl;
	out << "ServNmes: "; for (size_t i = 0; i < server.getServerNames().size(); i++){out << server.getServerNames()[i] << " ";} out << std::endl;
	out << "location:\n"; for (std::map<std::string, LocationConfig>::const_iterator it = server.getMapLocations().begin(); it != server.getMapLocations().end(); it++){out << "\t\tloc: " << it->first << std::endl;}
	out << static_cast<Config>(server);
	return out;
}
