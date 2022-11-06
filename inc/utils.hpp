/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   utils.hpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/23 11:26:24 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/11/06 13:51:15 by rkieboom      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
# define UTILS_HPP

# include <string>
# include <vector>
# include <map>
# include <time.h>

# define whitespaces " \v\t\n"

std::string create_autoindex_line(const std::string &path, const char *name, time_t ts, const unsigned long bytes, bool is_file);


/**
 * @brief Get the root options object
 * 
 * @param str path to check
 * @return int -1 if not exist
 * @return  0 if directory
 * @return 1 if not a directory
 */
int get_root_options(const char *path);
void			to_upper(std::string &str);
void			to_lower(std::string &str);
void			split(const std::string& str, const char* delims, std::vector<std::string>& out);
size_t			getCurlyBraceMatch(const std::vector<std::string> &v, size_t curlyBraceOpen);
size_t			getCurlyBraceMatch(const std::string& str, size_t curlyBraceOpen);
size_t 			count(std::string str, char c); // Function that counts the amount of times c is in str
std::string		ft_strtrim(std::string &s1, const char *set);
char 			*ft_strnstr(const char *s1, const char *s2, size_t len);
void			split(const std::string& str, const char* delims, std::vector<std::string>& out);
int 			ft_stoi(const std::string &str);

namespace ft
{
	class RespondCodes
	{
		public:
			/* Constructor  */
			RespondCodes();
			bool isValidRespondCode(int const &code);
			/* Destructor */
			virtual ~RespondCodes();

			std::map<int, std::string> get_respond_codes();
		public:
			std::map<int, std::string> response_codes;
	};

	class MimeTypes
	{
		public:
			/* Constructor  */
			MimeTypes();

			/* Destructor */
			virtual ~MimeTypes();

			/* Get the mime type for a given extension */
			std::string getMimeType(std::string extension);

		public:
			std::map<std::string, std::string> mimes;
	};
};





#endif // UTILS_HPP