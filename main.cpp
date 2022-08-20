/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/19 16:16:08 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/08/20 21:59:03 by bdekonin      ########   odam.nl         */
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

void	split(const std::string& str, const char* delims, std::vector<std::string>& out)
{
	size_t	posBegin = 0;
	size_t	posEnd;
	size_t  posDelim;
	size_t	braceOpen;
	size_t	braceClose;
	size_t	subLength;

	std::cout << str << std::endl;

	while (posBegin < str.length())
	{
		braceOpen = str.find('{', posBegin);
		if (braceOpen != std::string::npos)
			braceClose = getCurlyBraceMatch(str, braceOpen);
		posDelim = str.find_first_of(delims, posBegin);
		// if (braceOpen < posDelim)
		// 	posEnd = braceClose + 1;
		// else
		posEnd = std::min(posDelim, str.length());
		subLength = posEnd - posBegin;
		if (subLength)
		{
			std::string sub = str.substr(posBegin, subLength);
			{
				size_t begin = sub.find_first_not_of(whitespaces);
				if (begin != std::string::npos)
					sub = sub.substr(begin);
			}
			
			if (sub.find_first_not_of(whitespaces) != std::string::npos)
				out.push_back(sub);
		}
		posBegin = posEnd + 1;
	}
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

void readblocks(std::vector<std::string> &block)
{
	std::string identifier;
	std::string value;
	std::string temp;

	for (size_t i = 0; i < block.size(); i++)
	{

		identifier = block[i].substr(0, block[i].find_first_of(whitespaces)); // gets identifier
		temp = block[i].substr(block[i].find_first_of(whitespaces) + 1); // gets value with whitespaces
		value = temp.substr(temp.find_first_not_of(whitespaces)); // removes front whitespaces

		value = value.substr(0, value.find_last_not_of(whitespaces) + 1); // removes back whitespaces + 1 for ;. or location / and the {} of location

		if (identifier == "listen")
		std::cout << std::setw(25) << identifier << " : " << std::setw(25) << value << std::endl;
	}
	std::cout << std::endl;
}


int main(int argc, char **argv)
{
	if (argv[1] == NULL)
	{
		std::cout << "No file given" << std::endl;
		return 0;
	}
	std::string filecontent = getFilecontent(argv[1]);
	
	std::vector<std::vector<std::string> > blocks;
	
	blocks = splitServer(filecontent);


	for (int i = 0; i < blocks.size(); ++i)
	{
		readblocks(blocks[i]);
		for (int j = 0; j < 100; ++j)
		{
			std::cout << "_";
		}
		std::cout << std::endl;
	}

	return 0;
}