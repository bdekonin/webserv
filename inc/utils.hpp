/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   utils.hpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/23 11:26:24 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/08/23 11:42:34 by bdekonin      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
# define UTILS_HPP

# include <string>
# include <vector>
# include "Configuration.hpp"

void	split(const std::string& str, const char* delims, std::vector<std::string>& out);
size_t	getCurlyBraceMatch(const std::vector<std::string> &v, size_t curlyBraceOpen);
size_t	getCurlyBraceMatch(const std::string& str, size_t curlyBraceOpen);
size_t count(std::string str, char c); // Function that counts the amount of times c is in str


#endif // UTILS_HPP