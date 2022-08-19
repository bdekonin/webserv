#include "Parser.hpp"				// Parser
#include "StringUtils.hpp"			// StringUtils
#include <string>					// std::string
#include <vector>					// std::vector
#include <fstream>					// std::ifstream
#include <sstream>					// std::stringstream
#include <stdexcept>				// std::runtime_error

Parser::~Parser(void)
{
}

Parser::Parser(void)
{
}


void	Parser::init(const std::string& configFile)
{
	this->_filecontent = getFilecontent(configFile.c_str());
	syntaxErrorCheck();
	semicolonsErrorCheck();
	createServerConfig();
}

size_t	Parser::getAmountServers() const
{
	return this->_serverConfigs.size();
}

const std::vector<ServerConfig>&	Parser::getServerConfigs() const
{
	return this->_serverConfigs;
}

const ServerConfig&	Parser::getServerConfigs(size_t index) const
{
	return this->_serverConfigs[index];
}

std::string Parser::getFilecontent(const char *filename)
{
	std::ifstream in(filename, std::ios::in);
	if (!in)
		throw std::runtime_error("Failed to open config file");
	std::stringstream contents;
	contents << in.rdbuf();
	in.close();
	return contents.str();
}

void	Parser::syntaxErrorCheck() const
{
	size_t	countOpen = 0;
	size_t	countClose = 0;
	size_t	pos = 0;

	while (pos != std::string::npos)
	{
		if (this->_filecontent[pos] == '{')
			countOpen++;
		else if (this->_filecontent[pos] == '}')
			countClose++;
		pos = this->_filecontent.find_first_of("{}", pos + 1);
	}
	if (countOpen > countClose)
		throw std::runtime_error("configfile: missing ('}')");
	if (countClose > countOpen)
		throw std::runtime_error("configfile: extraneous closing brace ('}')");
}

static bool	isEndSemicolon(const std::string& str)
{
	size_t pos = str.find_last_not_of(Whitespaces);
	if (pos == std::string::npos)
		return (false);
	return str[pos] == ';';
}

void	Parser::semicolonsErrorCheck() const
{
	std::string					str = this->_filecontent;
	size_t						pos;
	std::vector<std::string> 	lines;

	StringUtils::splitNoBraces(str, "\n", lines);
	for (size_t i = 0; i < lines.size(); i++)
	{
		if (isEndSemicolon(lines[i]) == false)
		{
			pos = lines[i].find_last_not_of(Whitespaces);
			if (lines[i][pos] == '}')
				continue;
			pos = lines[i].find_last_not_of(Whitespaces);
			if (lines[i][pos] == '{')
				continue;
			if (i + 1 < lines.size())
				i++;
			else
				throw std::runtime_error("configfile: missing semicolon (1)");
			pos = lines[i].find_last_not_of(Whitespaces);
			if (lines[i][pos] == '{')
				continue;
			throw std::runtime_error("configfile: missing semicolon (2)");
		}
	}
}

void	Parser::createServerConfig()
{
	const std::string			id = "server";
	std::vector<std::string>	lines = StringUtils::splitServers(this->_filecontent);

	for (size_t i = 0; i < lines.size(); i++)
	{
		if (!lines[i].compare(0, id.size(), id))
		{
			std::string serverBlock = lines[i].substr(id.size(), lines[i].length() - id.size());
			ServerConfig newServer;
			newServer.init(serverBlock);
			this->_serverConfigs.push_back(newServer);
		}
		else
			throw std::runtime_error("configfile: unidentified input");
 	}
	if (this->_serverConfigs.size() == 0)
		throw std::runtime_error("configfile: missing server(s)");
}
