#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <vector>
#include "ServerConfig.hpp"

class Parser
{
	public:
		virtual ~Parser();
		Parser();

		void								init(const std::string&);
		size_t								getAmountServers() const;
		const std::vector<ServerConfig>&	getServerConfigs() const;
		const ServerConfig&					getServerConfigs(size_t index) const;
	
	private:
		std::string					_filecontent;
		std::vector<ServerConfig>	_serverConfigs;

		std::string	getFilecontent(const char *filename);
		void		syntaxErrorCheck() const;
		void		semicolonsErrorCheck() const;
		void		createServerConfig();
};

#endif
