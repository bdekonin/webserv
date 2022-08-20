/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   LocationConfiguration.hpp                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/20 21:42:51 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/08/21 01:04:43 by bdekonin      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONCONFIGURATION_HPP
# define LOCATIONCONFIGURATION_HPP

# include "Configuration.hpp"


class LocationConfiguration : public Configuration
{
	public:
		/* Constructor  */
		LocationConfiguration()
		: Configuration()
		{
		}
		LocationConfiguration(std::string &s)
		: Configuration(), _path(s)
		{
		}

		/* Destructor */
		virtual ~LocationConfiguration()
		{
			
		}

		/* Copy constructor */
		LocationConfiguration(const LocationConfiguration &src)
		{
			*this = src;
		}

		/* Operation overload = */
		LocationConfiguration& operator = (const LocationConfiguration& src)
		{
			this->_path = src._path;
			return *this;
		}

		// Methods
		void setPath(std::string &s)
		{
			this->_path = s;
		}
		std::string &getPath()
		{
			return (this->_path);
		}
	private:
		std::string		_path;
};

#endif // LOCATIONCONFIGURATION_HPP