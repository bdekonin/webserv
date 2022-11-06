/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Parser.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/22 23:01:41 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/11/05 15:03:34 by rkieboom      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_HPP
# define PARSER_HPP

# include <string>
# include <iostream>

# include <fstream>
# include <sstream>

# include <sys/types.h>
# include <sys/uio.h>
# include <unistd.h>

# include <fcntl.h>

# include <vector>

# include "Configuration.hpp" // Base Class
# include "ServerConfiguration.hpp" // Derived from Configuration
# include "LocationConfiguration.hpp" // Derived from Configuration

# include "Utils.hpp" // Utility functions

class Parser
{
	public:
		/* Constructor  */
		Parser(std::string filename);

		/* Destructor */
		virtual ~Parser();

		/* Copy constructor */
		Parser(const Parser&);

		/* Operation overload = */
		Parser& operator = (const Parser& e);

		/// @brief Parses the config file and returns a vector of ServerConfiguration objects
		/// @return this returns a vector of ServerConfiguration objects. Each ServerConfiguration object has a vector of LocationConfiguration objects if they exist in the config file.
		std::vector<ServerConfiguration> init();

	private:
		std::string			_filename;
		std::string			_filecontent;

		/// @brief Function that uses the _filename to read the file and store it in _filecontent and throws an error if it fails
		void _get_content();

		/// @brief Function that splits the filecontent into blocks and returns a vector of blocks
		void handle_file(int fd, std::string &content);
		
		/// @brief Function that splits the filecontent into blocks. a block is everything that belongs between the { }, without the { }
		/// @return 
		std::vector<std::vector<std::string> > splitServer();

		/// @brief This function loops through the lines of the block and parses them.
		/// @param block The block that needs to be parsed.
		/// @param config The Configuration that will be used to store the parsed data.
		/// @param server The ServerConfiguration that will be used to store the parsed data. (Only used for the recursive call for the location identifier). If not used call this function with the same parameter for both @Param config and @Param server.
		/// @return void
		void parse_block(std::vector<std::string> &block, Configuration &config, ServerConfiguration &server);
};

#endif // PARSER_HPP
