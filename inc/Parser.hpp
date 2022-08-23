/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Parser.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/22 23:01:41 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/08/23 11:31:20 by bdekonin      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_HPP
# define PARSER_HPP

# include <string>
# include <iostream>

# include <fstream>
# include <sstream>

# include <vector>

# include "Configuration.hpp" // Base Class
# include "ServerConfiguration.hpp" // Derived from Configuration
# include "LocationConfiguration.hpp" // Derived from Configuration

# include "utils.hpp" // Utility functions

class Parser
{
	public:
		/* Constructor  */
		Parser(std::string filename)
		: _filename(filename), _filecontent("")
		{
		}

		/* Destructor */
		virtual ~Parser()
		{
			
		}

		/* Copy constructor */
		Parser(const Parser&);

		/* Operation overload = */
		Parser& operator = (const Parser& e);

		// Methods
		std::vector<ServerConfiguration> init()
		{
			std::vector<std::vector<std::string> > blocks; // vector of blocks, will be used temporarily to store the blocks
			ServerConfiguration temp_server;
			std::vector<ServerConfiguration> servers;
			
			this->_get_content();
			blocks = this->splitServer(this->_filecontent);
			for (int i = 0; i < blocks.size(); i++)
			{
				temp_server = ServerConfiguration();
				this->parse_block(blocks[i], temp_server, temp_server);
				servers.push_back(temp_server);
			}
			return (servers);
		}
	private:
		std::string			_filename;
		std::string			_filecontent;

		void _get_content() // Function that uses the _filename to read the file and store it in _filecontent and throws an error if it fails
		{
			if (this->_filename.empty())
				throw std::runtime_error("parser: No filename given");

			std::ifstream in(this->_filename, std::ios::in);
			if (!in)
				throw std::runtime_error("parser: Failed to open config file");
			std::stringstream contents;
			contents << in.rdbuf();
			in.close();
			this->_filecontent = contents.str();
		}
		std::vector<std::vector<std::string> > splitServer(const std::string &content) // Splits the Content in multiple blocks
		{
			size_t posBegin = 0; 
			size_t bracketOpen = 0;
			size_t bracketClose = 0;
			size_t length = 0;
			std::vector<std::vector<std::string> > serverBlocks;

			if (count(content, '{') != count(content, '}'))
				throw std::runtime_error("Error: Brackets not closed");
			
			while (posBegin < content.length())
			{
				posBegin = content.find_first_not_of(whitespaces, posBegin);
				if (posBegin == std::string::npos) // end of the string (EOF)
					break;
				if (content.compare(posBegin, 6, "server"))
					throw std::runtime_error("config: no server block found");
				bracketOpen = content.find_first_not_of(whitespaces, posBegin + 6);
				if (bracketOpen == std::string::npos)
					throw std::runtime_error("config: server missing body");
				if (content[bracketOpen] != '{')
					throw std::runtime_error("config: server missing body");
				bracketClose = getCurlyBraceMatch(content, bracketOpen);
				length = bracketClose - posBegin + 1;
				if (length)
				{
					std::string s = content.substr(posBegin, length);
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

		size_t parse_block(std::vector<std::string> &block, Configuration &config, ServerConfiguration &server)
		{
			std::string identifier;
			std::string value;
			std::string temp;
			size_t i = 0;

			for (; i < block.size(); i++)
			{

				identifier = block[i].substr(0, block[i].find_first_of(whitespaces)); // gets identifier
				temp = block[i].substr(block[i].find_first_of(whitespaces) + 1); // gets value with whitespaces
				value = temp.substr(temp.find_first_not_of(whitespaces)); // removes front whitespaces

				value = value.substr(0, value.find_last_not_of(whitespaces) + 1); // removes back whitespaces + 1 for ;. or location / and the {} of location

				if (identifier == "location")
				{
					size_t bracketOpen;
					size_t bracketClose;
					std::vector<std::string> copy; // a copy of the location block

					bracketOpen = i + 1;
					bracketClose = i;

					bracketClose = getCurlyBraceMatch(block, bracketOpen + 1) - 1;

					copy = std::vector<std::string>(block.begin() + bracketOpen + 1, block.begin() + bracketClose);
					block.erase(block.begin() + bracketOpen - 1, block.begin() + bracketClose + 1);

					// checks is the location is nested in another location. if yes it add the parent location to the front.
					LocationConfiguration location(value);

					try
					{
						std::string path = location.get_path();
						LocationConfiguration &tempLocation = dynamic_cast<LocationConfiguration&>(config);

						location.set_path(std::string(tempLocation.get_path() + path));

						// TODO Configure Syntax eg. location / + location /post = //post change to /post
						// or change location A to /A or location /post + location TEST to /post/TEST and not /postTEST
					}
					catch(const std::bad_cast &e)
					{
						// std::cerr << e.what() << '\n';
					}

					parse_block(copy, location, server);
					i--;
					
					// // TODO maybe change to current configuration + new location
					// // so if location block dont have a value it will be the overwritten by the server block
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
			return i;
		}
};

#endif // PARSER_HPP