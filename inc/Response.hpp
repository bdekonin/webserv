/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Response.hpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/09/05 15:07:07 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/11/05 15:03:16 by rkieboom      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#ifndef DEBUG
# define DEBUG 0
#endif

# include <vector>
# include <map>
# include <string>
# include <cstring>

#include "Utils.hpp"

# include "Configuration.hpp"

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include "Colors.h"
#include <unistd.h>
#include <sstream>

class Response
{
	public:
		/* Constructor  */
		Response();

		/* Destructor */
		virtual ~Response();

		/* Copy constructor */
		Response(const Response &src);

		/* Operation overload = */
		Response& operator = (const Response& e);

		void set_status_code(int status_code);
		void set_header(std::string const &header);
		void set_body(const char *body, size_t length, size_t start);
		void set_body(const char *body, size_t length);
		bool is_body_empty() const;
		int get_status_code() const;
		const std::string &get_headers() const;
		void clear();

		void build_response_text();
		std::vector <unsigned char> &get_response();
		void set_default_headers(std::string extension);

	private:
		int	_status_code;
		std::string _headers;
		bool _is_build;

		std::map<int, std::string> _response_codes;
		std::vector<unsigned char> _response;

	public:
		std::vector<char> _body;
		void set_3xx_response(Configuration &config); /* Redirections */
		void set_405_response(Configuration &config); /* Method Not Allowed */ // Allow: GET, POST, DELETE
		void set_xxx_response(Configuration &config, int code);
	private:
		void set_500_response(); /* Internal Server Error CLOSE READ ERROR*/
};

#endif // RESPONSE_HPP
