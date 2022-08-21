/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   LocationConfiguration.hpp                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/20 21:42:51 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/08/21 21:50:31 by bdekonin      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONCONFIGURATION_HPP
# define LOCATIONCONFIGURATION_HPP

# include "Configuration.hpp"
// # include "ServerConfiguration.hpp"


class LocationConfiguration : public Configuration
{
	public:
		/* Constructor  */
		LocationConfiguration()
		{
		}
		LocationConfiguration(std::string &s)
		: _path(s)
		{
		}

		/* Destructor */
		virtual ~LocationConfiguration()
		{
			
		}

		/* Copy constructor */
		LocationConfiguration(const LocationConfiguration *src)
		{
			*this = *src;
		}
		LocationConfiguration(const LocationConfiguration &src)
		{
			*this = src;
		}


		/* Operation overload = */
		LocationConfiguration& operator = (const LocationConfiguration& src)
		{
			this->_error_page = src.get_error_page();
			this->_client_max_body_size = src.get_client_max_body_size();
			this->_methods[0] = src.get_methods(0);
			this->_methods[1] = src.get_methods(1);
			this->_methods[2] = src.get_methods(2);
			this->_return = src.get_return();
			this->_root = src.get_root();
			this->_autoindex = src.get_autoindex();
			this->_index = src.get_index();
			this->_cgi = src.get_cgi();
			this->_path = src.get_path(); // TODO check if this is correct
			return *this;
		}

		// Methods
		void setPath(std::string &s)
		{
			this->_path = s;
		}
		std::string &get_path()
		{
			return (this->_path);
		}
		const std::string &get_path() const
		{
			return (this->_path);
		}
	private:
		std::string		_path;
};

std::ostream&	operator<<(std::ostream& out, const LocationConfiguration& c)
{
	out << "Path:\n\t" << c.get_path() << std::endl;
	out << static_cast<const Configuration&>(c);
	return (out);
}

#endif // LOCATIONCONFIGURATION_HPP