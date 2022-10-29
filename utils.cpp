/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   utils.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/23 11:40:33 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/10/29 18:37:15 by bdekonin      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

# include "inc/utils.hpp"
# include <cstring>
#include <sstream>

/// @brief Function that converrts a string to a integer. Throws an error if the string is not a integer.
/// @param str String that needs to be converted to a integer.
/// @return Returns the integer that is converted from the string.
int ft_stoi(const std::string &str) 
{
  std::stringstream ss(str);
  int num;
  if((ss >> num).fail())
    throw std::invalid_argument("stoi");
  return num;
}

/// @brief Function that creates a string with x amount of spaces. Used for creating and spacing auto index files.
static std::string get_x_spaces(int x)
{
	if (x < 0)
		return "     ";
	std::string spaces(x, ' ');
	return (spaces);
}

/// @brief Function that creates one line for the autoindex generator
/// @param path Path of the file
/// @param name Name of the file
/// @param ts Timestamp of the file
/// @param bytes Bytes of the file
/// @param is_file Boolean that checks if the file is a file or a directory
/// @return Returns the string that is created for the autoindex generator
std::string create_autoindex_line(const std::string &path, const char *name, time_t ts, const unsigned long bytes, bool is_file)
{
	size_t spaces = 0;
	std::string line = "<a href=\"PATH\">NAME</a> SPACE DATE               BYTES";
	std::string date = "";
	char buff[100];

	strftime(buff, 100, "%d-%b-%Y %H:%M:%S", localtime(&ts));


	spaces = 90 - (path.size() + 2);
	if (is_file == false)
	{
		spaces -= 1;
		line.replace(line.find("PATH"), 4, path + "/");
	}
	else
		line.replace(line.find("PATH"), 4, path);

	line.replace(line.find("SPACE"), 5, get_x_spaces(spaces));

	if (is_file == false)
		line.replace(line.find("NAME"), 4, std::string(name) + "/");
	else
		line.replace(line.find("NAME"), 4, name);
	line.replace(line.find("DATE"), 4, buff);
	line.replace(line.find("BYTES"), 5, SSTR(bytes));

	return (line);
}

/// @brief Converts string to upper
/// @param str String that needs to be converted to upper
void to_upper(std::string &str)
{
	for (size_t i = 0; i < str.size(); i++)
		str[i] = toupper(str[i]);
}
/// @brief Converts string to lower
/// @param str String that needs to be converted to lower
void to_lower(std::string &str)
{
	for (size_t i = 0; i < str.size(); i++)
		str[i] = tolower(str[i]);
}

/// @brief Function that counts characters in a string
/// @param str String that needs to be counted
/// @param c Character that needs to be counted
/// @return Returns the amount of characters in the string
size_t		count(std::string const &str, char c)
{
	size_t count = 0;
	for (size_t i = 0; i < str.length(); i++)
		if (str[i] == c)
			count++;
	return (count);
}
size_t		count(std::string const str, char c)
{
	size_t count = 0;
	for (size_t i = 0; i < str.length(); i++)
		if (str[i] == c)
			count++;
	return (count);
}

/// @brief Function that matches the '{' with the '}' in a string
/// @param str String that needs to be checked
/// @param curlyBraceOpen Position of the '{'
/// @return Returns the position of the '}'
size_t	getCurlyBraceMatch(std::string const &str, size_t curlyBraceOpen)
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

/// @brief Function that matches the '{' with the '}' in a string
/// @param v Vector that needs to be checked
/// @param curlyBraceOpen Position of the '{'
/// @return Returns the position of the '}'
size_t	getCurlyBraceMatch(const std::vector<std::string> &v, size_t curlyBraceOpen)
{
	size_t	pos = curlyBraceOpen;
	int		BraceSubString = 1;

	// v[pos].find('{') == std::string::npos

	while (pos < v.size() && BraceSubString != 0)
	{
		if (v[pos].find('{') != std::string::npos)
			BraceSubString++;
		else if (v[pos].find('}') != std::string::npos)
			BraceSubString--;
		pos++;
	}
	return pos;
}


/// @brief Similar to the function ft_split. This function splits a string on a character and returns a vector with the split strings.
/// @param str String that needs to be split
/// @param delims Characters that needs to be split on
/// @param out Vector that will be filled with the split strings
void	split(std::string const &str, const char* delims, std::vector<std::string> &out)
{
	size_t	posBegin = 0;
	size_t	posEnd;
	size_t  posDelim;
	size_t	braceOpen;
	size_t	braceClose = 0;
	size_t	subLength;

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
		(void)braceClose;
		posBegin = posEnd + 1;
	}
}

static int		trim_left(std::string &s1, const char *set)
{
	int i;
	int j;

	i = 0;
	while (s1[i])
	{
		j = 0;
		while (s1[i] != set[j] && set[j])
			j++;
		if (!set[j])
			break ;
		i++;
	}
	return (i);
}

static int		trim_right(std::string &s1, const char *set)
{
	int i;
	int j;

	i = s1.size() - 1;
	while (i >= 0)
	{
		j = 0;
		while (s1[i] != set[j] && set[j])
			j++;
		if (!set[j])
			break ;
		i--;
	}
	return (i);
}

std::string		ft_strtrim(std::string &s1, const char *set)
{
	std::string	str;
	int		left;
	int		right;

	if (s1.size() == 0)
		return (std::string(""));
	if (!set)
		return (std::string(s1));
	left = trim_left(s1, set);
	right = trim_right(s1, set);
	if (right - left + 1 < 0)
		return (std::string(""));
	str = s1.substr(left, right - left + 1);
	return (str);
}

char *ft_strnstr(const char *s1, const char *s2, size_t len)
{
	size_t l2;
	l2 = strlen(s2);
	if (!l2)
		return (char *)s1;
	while (len >= l2) {
		len--;
		if (!memcmp(s1, s2, l2))
			return (char *)s1;
		s1++;
	}
	return NULL;
}