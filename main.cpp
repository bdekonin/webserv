/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/19 16:16:08 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/08/20 15:23:20 by bdekonin      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <fstream>
#include <sstream>

#include <vector>
#include <map>
#include <set>

#include <string>
#include <cstring>
#include <algorithm>
#include <iomanip>

#define whitespaces " \v\t\n"


class serverConfig
{
	public:
		std::vector<size_t>						_listen;
		std::string								_root;

		std::vector<std::string>				_server_names;
		std::map<size_t, std::string> 			_error_pages;
		size_t									_client_max_body_size;
		std::map<std::string, std::string>		_cgi_param;
		std::string								_cgi_pass;
		std::map<std::string, int>				_location; // change int to hpp
		std::set<std::string>					_methods;
		std::vector<std::string>				_index; // order of index files
		bool									_autoindex;
		std::string								_alias;
		bool									_aliasSet;	
};

std::string getFilecontent(const char *filename)
{
	std::ifstream in(filename, std::ios::in);
	if (!in)
		throw std::runtime_error("Failed to open config file");
	std::stringstream contents;
	contents << in.rdbuf();
	in.close();
	return contents.str();
}


static size_t	getCurlyBraceMatch(const std::string& str, size_t curlyBraceOpen)
{
	size_t	pos = curlyBraceOpen;
	int		BraceSubString = 1;

	while (pos < str.length() && BraceSubString != 0)
	{
		pos = str.find_first_of("{}", pos + 1);
		if (str[pos] == '{')
			BraceSubString++;
		else if (str[pos] == '}')
			BraceSubString--;
	}
	return pos;
}

std::vector<std::vector<std::string> > splitServer(const std::string &content)
{
	size_t posBegin = 0; 
	size_t bracketOpen = 0;
	size_t bracketClose = 0;
	size_t length = 0;
	std::vector<std::vector<std::string> > serverBlocks;
	
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
			s.push_back('\n');
			std::vector<std::string> block;
			std::string temp = "";
			bool inString = false;
			
			
			
			for (int i = 0; i < s.length(); ++i)
			{
				if(s[i] =='\n'){
					if (temp.length() > 0)
						block.push_back(temp);
					temp = "";
					inString = false;
				}
				else{
					if (inString == false)
					{
						if (s[i] == 32 || s[i] == '\t')
							s[i] = '\b';
						else if (s[i] != 32 || s[i] != '\t')
						{
							inString = true;
						}
					}
					if (s[i] != '\b')
						temp.push_back(s[i]);
				}
			}
			serverBlocks.push_back(block);
		}
		posBegin = bracketClose + 1;
	}

	// Everything went well. Removing
	// line[0]: server
	// line[1]: {
	// line[line.size() - 1]: }

	for (int i = 0; i < serverBlocks.size(); ++i)
	{
		serverBlocks[i].erase(serverBlocks[i].begin());
		serverBlocks[i].erase(serverBlocks[i].begin());
		serverBlocks[i].erase(serverBlocks[i].end() - 1);
	}
	return serverBlocks;
}

void readblocks(std::vector<std::string> &block)
{
	std::string id;
	std::string value;
	std::string temp;

	for (size_t i = 0; i < block.size(); i++)
	{
		id = block[i].substr(0, block[i].find_first_of(whitespaces)); // gets id
		temp = block[i].substr(block[i].find_first_of(whitespaces) + 1); // gets value with whitespaces
		value = temp.substr(temp.find_first_not_of(whitespaces)); // removes front whitespaces

		value = value.substr(0, value.find_last_not_of(whitespaces) + 1); // removes back whitespaces + 1 for ;. or location / and the {} of location
		
		if (id == "listen")
		{
			
		}
		std::cout << std::setw(20) << id << " : " << std::setw(20) << value << std::endl;
	}
	std::cout << std::endl;
}


int main(int argc, char **argv)
{
	argv[1] = strdup("config/default_configfile.conf");
	std::string filecontent = getFilecontent(argv[1]);
	
	std::vector<std::vector<std::string> > blocks;
	
	blocks = splitServer(filecontent);

	// for (int i = 0; i < blocks.size(); ++i)
	// {
	// 	for (int j = 0; j < blocks[i].size(); ++j)
	// 	{
	// 		std::cout << i << " " << blocks[i][j] << std::endl;
	// 	}
	// 	std::cout << std::endl;
	// }
	for (int i = 0; i < blocks.size(); ++i)
	{
		readblocks(blocks[i]);
		for (int j = 0; j < 50; ++j)
		{
			std::cout << "_";
		}
		std::cout << std::endl;
	}

	
	return 0;
}