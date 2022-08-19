#include "StringUtils.hpp"			// StringUtils
#include <string>					// std::string
#include <vector>					// std::vector
#include <stdexcept>				// std::runtime_error

void	StringUtils::trim(std::string& str, const char *chars)
{
	size_t start = str.find_first_not_of(chars);
	if (start == std::string::npos)
		start = 0;
	size_t end = str.find_last_not_of(chars);
	if (end == std::string::npos)
	{
		str = ";";
		return;
	}
	size_t len = end - start + 1;
	str = str.substr(start, len);
}

void	StringUtils::trimBraces(std::string& str)
{
	size_t start = str.find('{');
	size_t end = str.rfind('}');
	size_t len = end - start - 1;
	str = str.substr(start + 1, len);
	trim(str, Whitespaces);
}

std::string	StringUtils::substrDelimiter(size_t *startPosition, std::string const &source, std::string const &delimiter)
{
	size_t			endPosition;
	std::string		result;

	endPosition = source.find(delimiter, *startPosition);
	if (endPosition != std::string::npos)
	{
		result = source.substr(*startPosition, endPosition - *startPosition);
		*startPosition = endPosition + delimiter.size();
	}
	else
	{
		result = source.substr(*startPosition);
		*startPosition = endPosition;
	}
	return result;
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

void	StringUtils::split(const std::string& str, const char* delims, std::vector<std::string>& out)
{
	size_t	posBegin = 0;
	size_t	posEnd;
	size_t  posDelim;
	size_t	braceOpen;
	size_t	braceClose;
	size_t	subLength;

	while (posBegin < str.length())
	{
		braceOpen = str.find('{', posBegin);
		if (braceOpen != std::string::npos)
			braceClose = getCurlyBraceMatch(str, braceOpen);
		posDelim = str.find_first_of(delims, posBegin);
		if (braceOpen < posDelim)
			posEnd = braceClose + 1;
		else
			posEnd = std::min(posDelim, str.length());
		subLength = posEnd - posBegin;
		if (subLength)
		{
			std::string sub = str.substr(posBegin, subLength);
			StringUtils::trim(sub, Whitespaces);
			out.push_back(sub);
		}
		posBegin = posEnd + 1;
	}
}

void	StringUtils::splitNoBraces(const std::string& str, const char* delims, std::vector<std::string>& out)
{
	size_t	posBegin = 0;
	size_t	posEnd;
	size_t  posDelim;
	size_t	subLength;

	while (posBegin < str.length())
	{
		posDelim = str.find_first_of(delims, posBegin);
		posEnd = std::min(posDelim, str.length());
		subLength = posEnd - posBegin;
		if (subLength)
		{
			std::string sub = str.substr(posBegin, subLength);
			StringUtils::trim(sub, Whitespaces);
			out.push_back(sub);
		}
		posBegin = posEnd + 1;
	}
}

std::vector<std::string>	StringUtils::splitServers(const std::string& filecontents)
{
	std::vector<std::string>	vec;
	size_t	posBegin = 0;
	size_t	posEnd;
	size_t	braceOpen;
	size_t	subLength;

	while (posBegin < filecontents.length())
	{
		posBegin = filecontents.find_first_not_of(Whitespaces, posBegin);
		if (posBegin == std::string::npos)
			break;
		if (filecontents.compare(posBegin, 6, "server"))
			throw std::runtime_error("configfile: not a server");
		braceOpen = filecontents.find_first_not_of(Whitespaces, posBegin + 6);
		if (braceOpen == std::string::npos)
			throw std::runtime_error("configfile: server missing body");
		if (filecontents[braceOpen] != '{')
			throw std::runtime_error("configfile: server missing body");
		posEnd = getCurlyBraceMatch(filecontents, braceOpen);
		subLength = posEnd - posBegin + 1;
		if (subLength)
		{
			std::string substr = filecontents.substr(posBegin, subLength);
			StringUtils::trim(substr, Whitespaces);
			vec.push_back(substr);
		}
		posBegin = posEnd + 1;
	}
	return vec;
}

void	StringUtils::matchIdentifier(const std::string& str, std::string& identifier)
{
	std::vector<std::string> vec;

	StringUtils::split(str, Whitespaces, vec);
	if (vec.size())
		identifier = vec[0];
	else
		identifier = "";
}

void	StringUtils::lowerCase(std::string& string)
{
	for (size_t i = 0; i < string.length(); i++)
		string[i] = std::tolower(string[i]);
}

/*
 * This function gets a substring from a bigger string from a starting
 * position (which gets updated after, to point to the end of the returned
 * substring), and a delimiter.
 */
std::string		StringUtils::getString(size_t *startPosition, std::string const &source, std::string const &delimiter)
{
	size_t			endPosition;
	std::string		returnString;

	endPosition = source.find(delimiter, *startPosition);
	returnString = source.substr(*startPosition, endPosition - *startPosition);
	*startPosition = endPosition + delimiter.size();
	return returnString;
}
