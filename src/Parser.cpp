/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Parser.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: rkieboom <rkieboom@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/11/04 21:54:10 by rkieboom      #+#    #+#                 */
/*   Updated: 2022/11/06 17:34:57 by lsmit         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Parser.hpp"
#include <cstring>

Parser::Parser(std::string filename)
: _filename(filename), _filecontent("")
{
}

		/* Destructor */
Parser::~Parser()
{
}

		/* Copy constructor */
Parser::Parser(const Parser &e)
{
	*this = e;
}

		/* Operation overload = */
Parser& Parser::operator = (const Parser& e)//Nog gemaakt worden
{
	(void)e;
	return *this;
}

/// @brief Parses the config file and returns a vector of ServerConfiguration objects
/// @return this returns a vector of ServerConfiguration objects. Each ServerConfiguration object has a vector of LocationConfiguration objects if they exist in the config file.
std::vector<ServerConfiguration> Parser::init()
{
	std::vector<std::vector<std::string> > blocks; // vector of blocks, will be used temporarily to store the blocks
	ServerConfiguration temp_server;
	std::vector<ServerConfiguration> servers;
	
	this->_get_content();
	blocks = this->splitServer();
	for (size_t i = 0; i < blocks.size(); i++)
	{
		temp_server = ServerConfiguration();
		this->parse_block(blocks[i], temp_server, temp_server);
		servers.push_back(temp_server);
	}
	for (size_t i = 0; i < servers.size(); i++)
		for (size_t j = 0; j < servers[i].get_locations().size(); j++)
			servers[i].get_locations()[j].get_cgi().insert(servers[i].get_cgi().begin(), servers[i].get_cgi().end());

	return (servers);
}

void Parser::_get_content()
{
	if (this->_filename.empty())
		throw std::runtime_error("parser: No filename given");
	if (get_root_options(this->_filename.c_str()) <= 0)
		throw std::runtime_error("parser: Filename is a directory or does not exist");
	int fd = open(this->_filename.c_str(), O_RDONLY);
	if (fd == -1)
		throw std::runtime_error("parser: Failed to open config file");

	this->handle_file(fd, this->_filecontent);
	close(fd);
}

/// @brief Function that splits the filecontent into blocks and returns a vector of blocks
void Parser::handle_file(int fd, std::string &content)
{
	int ret = 0;
	char buf[4096 + 1];
	
	bzero(buf, 4096 + 1);
	while ((ret = read(fd, buf, 4096)) > 0)
	{
		content.insert(content.end(), buf, buf + ret);
		if (ret < 4096)
			break;
		bzero(buf, 4096);
	}
}
		
/// @brief Function that splits the filecontent into blocks. a block is everything that belongs between the { }, without the { }
/// @return 
std::vector<std::vector<std::string> > Parser::splitServer()
{
	size_t posBegin = 0; 
	size_t bracketOpen = 0;
	size_t bracketClose = 0;
	size_t length = 0;
	std::vector<std::vector<std::string> > serverBlocks;
	std::string &fc = this->_filecontent;

	if (count(fc, '{') != count(fc, '}'))
		throw std::runtime_error("Error: Brackets not closed");
	
	while (posBegin < fc.length())
	{
		posBegin = fc.find_first_not_of(whitespaces, posBegin);
		if (posBegin == std::string::npos) // end of the string (EOF)
			break;
		if (fc.compare(posBegin, 6, "server"))
			throw std::runtime_error("config: no server block found");
		bracketOpen = fc.find_first_not_of(whitespaces, posBegin + 6);
		if (bracketOpen == std::string::npos)
			throw std::runtime_error("config: server missing body");
		if (fc[bracketOpen] != '{')
			throw std::runtime_error("config: server missing body");
		bracketClose = getCurlyBraceMatch(fc, bracketOpen);
		length = bracketClose - posBegin + 1;
		if (length)
		{
			std::string s = fc.substr(posBegin, length);
			{
				size_t start = s.find('{');
				size_t end = s.rfind('}');
				size_t len = end - start - 1;
				s = s.substr(start + 1, len);
			}
			std::vector<std::string> lines;
			split(s, "\n", lines);
			serverBlocks.push_back(lines);
		}
		posBegin = bracketClose + 1;
	}
	return serverBlocks;
}

/// @brief This function loops through the lines of the block and parses them.
/// @param block The block that needs to be parsed.
/// @param config The Configuration that will be used to store the parsed data.
/// @param server The ServerConfiguration that will be used to store the parsed data. (Only used for the recursive call for the location identifier). If not used call this function with the same parameter for both @Param config and @Param server.
/// @return void
void Parser::parse_block(std::vector<std::string> &block, Configuration &config, ServerConfiguration &server)
{
	/// Function that converts the blocks (made in this.init()) into a ServerConfiguration.
	/// When this function finds a 'location' identifier,
	/// it goes recursive and uses the @Param config to create a LocationConfiguration.
	/// which will append to the Server after location has been fully set.
	std::string identifier;
	std::string value;
	std::string temp;

	for (size_t i = 0; i < block.size(); i++)
	{
		size_t size;

		size = block[i].find_first_of(whitespaces);
		size = (size > block[i].length() || size == std::string::npos) ? block[i].length() : size;
		identifier = block[i].substr(0, size); // gets identifier

		size = block[i].find_first_of(whitespaces);
		size = (size == std::string::npos) ? block[i].length() - 1 : size;
		temp = block[i].substr(size + 1); // gets value with whitespaces

		size = temp.find_first_not_of(whitespaces);
		size = (size > temp.length() || size == std::string::npos) ? temp.length() : size;
		value = temp.substr(size); // removes front whitespaces

		size = value.find_last_not_of(whitespaces);
		size = (size > value.length() || size == std::string::npos) ? value.length() : size;
		value = value.substr(0, size + 1); // removes back whitespaces + 1 for ;. or location / and the {} of location


		if (identifier == "location")
		{
			if (identifier == value || value.empty())
				throw std::runtime_error("config: location value is empty");
			size_t bracketOpen;
			size_t bracketClose;
			std::vector<std::string> copy; // a copy of the location block
			bracketOpen = i + 1;
			bracketClose = i;
			bracketClose = getCurlyBraceMatch(block, bracketOpen + 1) - 1;
			copy = std::vector<std::string>(block.begin() + bracketOpen + 1, block.begin() + bracketClose);
			/* removes the { when you add it to the end of the location identifier + value */
			if (value[value.length() - 1] == '{')
			{
				size_t last_whitspace;
				value = value.substr(0, value.length() - 1);
				last_whitspace = value.find_last_not_of(whitespaces);
				value = value.substr(0, last_whitspace + 1);
			}

			if (*value.begin() != '/')
				throw std::runtime_error("config: location value does not start with /");
			if (*value.rbegin() != '/')
				throw std::runtime_error("config: location value does not end with /");

			block.erase(block.begin() + bracketOpen - 1, block.begin() + bracketClose + 1);

			// checks is the location is nested in another location. if yes it add the parent location to the front.
			LocationConfiguration location(value);

			try
			{
				std::string path = location.get_path();
				LocationConfiguration &tempLocation = dynamic_cast<LocationConfiguration&>(config);

				

				if (path[0] == '/')
					location.set_path(tempLocation.get_path() + path.substr(1));
				else
					location.set_path(std::string(tempLocation.get_path() + path));

				// TODO Configure Syntax eg. location / + location /post = //post change to /post
				// or change location A to /A or location /post + location TEST to /post/TEST and not /postTEST
			}
			catch(const std::exception &e)
			{
				// std::cerr << e.what() << '\n';
			}

			parse_block(copy, location, server);

			location.combine_two_locations(config); // combines the location block data with the parent block data

			i--;
			
			server.get_locations().push_back(location);
			continue;
		}
		if (identifier == "error_page")
		{
			config.set_error_page(value);
		}
		else if (identifier == "limit_except")
		{
			config.set_methods(value);
		}
		else if (identifier == "return")
			config.set_return(value);
		else if (identifier == "root")
			config.set_root(value);
		else if (identifier == "autoindex")
			config.set_autoindex(value);
		else if (identifier == "index")
			config.set_index(value);
		else if (identifier == "cgi")
			config.set_cgi(value);
		else if (identifier == "listen")
			server.set_listen(value);
		else if (identifier == "server_name")
			server.set_server_names(value);
		else if (identifier == "client_max_body_size")
			config.set_client_max_body_size(value);
		else
			throw std::runtime_error("config: unknown identifier " + identifier);
	}
}
