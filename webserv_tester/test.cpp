/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   test.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/11/06 15:58:20 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/11/06 17:13:30 by bdekonin      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */


#include <string>
#include <iostream>
#include <sstream>


const int bytes = 10;
int main(void)
{
	std::string body("name=Hoi&message=Bobbieee&contact_submitted=submit");

	std::string str = "";

	while (1)
	{
		if (body.empty())
			break;
		std::stringstream stream;
		stream << std::hex << bytes;
		std::string result( stream.str() );

		str += result + "\\r\\n";
		str += body.substr(0, bytes) + "\\r\\n";
		body.erase(0, bytes);
	}
	str += "0\\r\\n\\r\\n";
	std::cout << str << std::endl;
}
