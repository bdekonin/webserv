/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/19 16:16:08 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/08/23 11:50:58 by bdekonin      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>

# include "inc/Configuration.hpp" // Base Class
# include "inc/ServerConfiguration.hpp" // Derived from Configuration
# include "inc/LocationConfiguration.hpp" // Derived from Configuration


# include "inc/Parser.hpp"
int main(int argc, char **argv)
{
	std::vector<ServerConfiguration> servers;
	Parser parser(argv[1]);

	servers = parser.init();

	for (int i = 0; i < servers.size(); i++)
	{
		std::cout << servers[i] << std::endl;
	}
}