/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Configuration.hpp                                  :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/20 22:03:45 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/11/07 22:28:42 by bdekonin      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGURATION_HPP
# define CONFIGURATION_HPP

# include "Utils.hpp"
# include "Request.hpp"

# include <iostream>
# include <string>
# include <vector>
# include <map>

# define whitespaces " \v\t\n"
# define forbidden_characters "\'\"|&;<>`$(){}[]*?#~"
# define CLIENT_MAX_BODY_SIZE_MULTIPLIER 1000000

class ServerConfiguration;

/* Constructor
** Default constructor which is parent class of the ServerConfiguration.hpp and Configuration.hpp
** It includes everything a 'Server' and a 'Location Block' should have.
*/

class Configuration
{
	public:

		/* Constructors - */
			/**
			 * @brief Construct a new Configuration object
			 * 
			 */
			Configuration();
			/**
			 * @brief Construct a new Configuration object
			 * 
			 * @param src Object to be copied
			 */
			Configuration(const Configuration &src);

		/* Destructor - */
			virtual ~Configuration();

		/* Operator overloads - */
			/**
			 * @brief Overload of the assignment operator
			 * 
			 * @param src Object to be copied
			 * @return Configuration& 
			 */
			Configuration& operator = (const Configuration& src);

		/* Public member functions - */
			/**
			 * @brief Clears all data
			 * 
			 */
			void clear();

			/**
			 * @brief When there is a location block inside a config the location block will have a seperate config object. some data has to be copied into the next config object.
			 * 
			 * @param src the config object that has to be copied into the current config object.
			 */
			void combine_two_locations(Configuration &src);

			/**
			 * @brief Returns a boolean if the http method is allowed. This function compares it while ignoring the uppwer/lower case.
			 * 
			 * @param method the http method. GET POST DELETE
			 * @return true if the method is allowed
			 * @return false if the method is not allowed
			 */
			bool is_method_allowed(std::string const &method) const;
			bool is_method_allowed(Request::Method const type) const;
		
			/**
			 * @brief Returns a boolean if the http method is allowed. This function compares it while ignoring the uppwer/lower case.
			 * 
			 * @param method the http method. GET POST DELETE
			 * @return true if the method is allowed
			 * @return false if the method is not allowed
			 */
			bool is_method_allowed(const char *method) const;

		/* Setters */
			void set_error_page(std::string &s);
			void set_client_max_body_size(std::string &s);
			void set_methods(std::string &s);
			void set_methods(size_t i, bool b);
			void set_return(std::string &s);
			void set_root(std::string &s);
			void set_autoindex(std::string &s);
			void set_index(std::string &s);
			void set_cgi(std::string &s);

		// Getters
			std::map<size_t, std::string>				&get_error_page();
			const std::map<size_t, std::string> 		&get_error_page() const;
			size_t										get_client_max_body_size();
			size_t										get_client_max_body_size() const;
			bool										get_methods(size_t request);
			bool										get_methods(size_t request) const;
			std::map<size_t, std::string> 				&get_return();
			const std::map<size_t, std::string> 		&get_return() const;
			std::string 								&get_root();
			const std::string 							&get_root() const;
			bool										get_autoindex();
			bool										get_autoindex() const;
			std::vector<std::string> 					&get_index();
			const std::vector<std::string> 				&get_index() const;
			std::map<std::string, std::string> 			&get_cgi();
			const std::map<std::string, std::string>	&get_cgi() const;

	protected:
		std::map<std::string, bool>			_isSet;
		std::map<size_t, std::string>		_error_page; // <error code, path>
		size_t								_client_max_body_size; // max size of body
		bool								_methods[3]; // GET, POST, DELETE
		std::map<size_t, std::string>		_return; // Redirection to ...
		std::string							_root; // root path
		bool								_autoindex; // defaults to false
		std::vector<std::string>			_index; // order of index files
		std::map<std::string, std::string>	_cgi; // path to cgi
		
		/* Private Member Functions */
			/**
			 * @brief This function removes the semicolon from the end of the string
			 * 
			 * @param s the string to remove the semicolon from
			 */
			void remove_semicolen(std::string &s);

			/**
			 * @brief This function checks if the string has forbidden characters
			 * 
			 * @param s the string to check
			 */
			void has_forbidden_charachters(std::string &s);
};

// inline std::ostream&	operator<<(std::ostream& out, const Configuration& c)
// {
// 	{
// 		out << "Error page: " << std::endl;
// 		for (std::map<size_t, std::string>::const_iterator it = c.get_error_page().begin(); it != c.get_error_page().end(); it++)
// 			out << "\t" << it->first << " " << it->second << std::endl;
// 		if (c.get_error_page().size() == 0)
// 			out << "\t" << "None" << std::endl;
// 	}
// 	{
// 		out << "Client max body size:\n\t" << c.get_client_max_body_size() << std::endl;
// 	}
// 	{
// 		out << "Methods:\n";
// 		if (c.get_methods(0) == true)
// 			out << "\tGET\n";
// 		if (c.get_methods(1) == true)
// 			out << "\tPOST\n";
// 		if (c.get_methods(2) == true)
// 			out << "\tDELETE\n";
// 		if (c.get_methods(0) == false && c.get_methods(1) == false && c.get_methods(2) == false)
// 			out << "\tNone\n";
// 	}
// 	{
// 		out << "Return:" << std::endl;
// 		for (std::map<size_t, std::string>::const_iterator it = c.get_return().begin(); it != c.get_return().end(); it++)
// 			out << "\t" << it->first << " " << it->second << std::endl;
// 		if (c.get_return().size() == 0)
// 			out << "\t" << "None" << std::endl;
// 	}
// 	{
// 		if (c.get_root() == "")
// 			out << "Root:\n\tNone" << std::endl;
// 		else
// 			out << "Root:\n\t" << c.get_root() << std::endl;
// 	}
// 	{
// 		if (c.get_autoindex() == true)
// 			out << "Autoindex:\n\tTrue" << std::endl;
// 		else
// 			out << "Autoindex:\n\tFalse" << std::endl;
// 	}
// 	{
// 		out << "Index:" << std::endl;
// 		for (size_t i = 0; i < c.get_index().size(); i++)
// 		{
// 			if (i == 0)
// 				out << "\t" << c.get_index()[i];
// 			else
// 				out <<  " -> " << c.get_index()[i];
// 		}
// 		if (c.get_index().size() == 0)
// 			out << "\t" << "None";
// 		out << std::endl;
// 	}
// 	{
// 		out << "Cgi:" << std::endl;
// 		for (std::map<std::string, std::string>::const_iterator it = c.get_cgi().begin(); it != c.get_cgi().end(); it++)
// 			out << "\t" << it->first << " " << it->second << std::endl;
// 		if (c.get_cgi().size() == 0)
// 			out << "\t" << "None" << std::endl;
// 	}
// 	return out;
// }

#endif // CONFIGURATION_HPP