/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Response.hpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/09/05 15:07:07 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/09/05 15:08:18 by bdekonin      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
# define RESPONSE_HPP

class Response
{
	public:
		/* Constructor  */
		Response();

		/* Destructor */
		virtual ~Response();

		/* Copy constructor */
		Response(const Response&);

		/* Operation overload = */
		Response& operator = (const Response& e);

		// Methods
		/*
			set_statuscode // bad request forbidden etc
			set_headers
			set_body
			get_respone_in_string
		*/
	private:
		// ...
};

#endif // RESPONSE_HPP