#ifndef StringUtils_HPP
# define StringUtils_HPP

# include <string>		// std::string
# include <vector>		// std::vector

# define Whitespaces " \v\t\n"

class StringUtils
{
	public:
		static void			trim(std::string&, const char*);
		static void			trimBraces(std::string&);
		static std::string	substrDelimiter(size_t *startPosition, std::string const &source, std::string const &delimiter);
		static void			split(const std::string&, const char*, std::vector<std::string>&);
		static void			splitNoBraces(const std::string&, const char*, std::vector<std::string>&);
		static std::vector<std::string>	splitServers(const std::string&);
		static void			matchIdentifier(const std::string&, std::string&);
		static void			lowerCase(std::string& string);
		static std::string	getString(size_t *startPosition, std::string const &source, std::string const &delimiter);
};

#endif
