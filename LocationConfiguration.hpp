/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   LocationConfiguration.hpp                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/20 21:42:51 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/08/20 21:57:11 by bdekonin      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONCONFIGURATION_HPP
# define LOCATIONCONFIGURATION_HPP

# include "ServerConfiguration.hpp"


class LocationConfiguration : public ServerConfiguration
{
	public:
		/* Constructor  */
		LocationConfiguration();

		/* Destructor */
		virtual ~LocationConfiguration();

		/* Copy constructor */
		LocationConfiguration(const LocationConfiguration&);

		/* Operation overload = */
		LocationConfiguration& operator = (const LocationConfiguration& e);

		// Methods
		// ...
	private:
		std::string		_path;
};

#endif // LOCATIONCONFIGURATION_HPP