/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   utils.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/23 11:40:33 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/09/13 12:26:57 by bdekonin      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "inc/utils.hpp"





size_t		count(std::string str, char c)
{
	size_t count = 0;
	for (size_t i = 0; i < str.length(); i++)
		if (str[i] == c)
			count++;
	return (count);
}

size_t	getCurlyBraceMatch(const std::string& str, size_t curlyBraceOpen)
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

void	split(const std::string& str, const char* delims, std::vector<std::string>& out)
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