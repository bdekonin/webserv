/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   test.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: bdekonin <bdekonin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/11/06 15:58:20 by bdekonin      #+#    #+#                 */
/*   Updated: 2022/11/06 19:58:18 by bdekonin      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */


#include <string>
#include <iostream>
#include <sstream>
#include <cstdlib>

#define NAME "HALLO_DIT_ZIT_IN_DE_DEFINE_EN_IS_EEN_STRING"
#define MSG "IK_MOET_HIER_RANDOM_DINGEN_TYPEN_OM_TE_TESTEN_OF_HET_WERKT"

// int getRandomInt(int max)
int getRandomInt(int min, int max)
{
	int random = rand() % max + min;
	return (random);
}


int main(void)
{
	int bytes;


	std::string body("name=100&message=200&contact_submitted=submit");
	body.replace(body.find("100"), 3, NAME);
	body.replace(body.find("200"), 3, MSG);

	std::string str = "";
	while (1)
	{
		if (body.empty())
			break;
		bytes = getRandomInt(10, 1);
		if (bytes > body.size())
			bytes = body.size();
		std::stringstream stream;
		stream << std::hex << bytes;
		// stream << bytes;
		std::string result( stream.str() );

		str += result + "\\r\\n";
		str += body.substr(0, bytes) + "\\r\\n";
		body.erase(0, bytes);
	}
	str += "0\\r\\n\\r\\n";
	std::cout << str << std::endl;
}


// 3\r\nnam\r\n6\r\ne=bob&\r\n7\r\nmessage\r\n5\r\n=hall\r\n3\r\no&c\r\n5\r\nontac\r\n6\r\nt_subm\r\n2\r\nit\r\n9\r\nted=submi\r\n1\r\nt\r\n0\r\n\r\n


// const int bytes = 10;
// int main(void)
// {
	
// 	std::string body("name=100&message=200&contact_submitted=submit");

// 	body.replace(body.find(NAME), sizeof(NAME) - 1, "100");
// 	body.replace(body.find(MSG), sizeof(MSG) - 1, "200");

// 	std::string str = "";
// 	while (1)
// 	{
// 		if (body.empty())
// 			break;
// 		std::stringstream stream;
// 		stream << std::hex << bytes;
// 		std::string result( stream.str() );

// 		str += result + "\\r\\n";
// 		str += body.substr(0, bytes) + "\\r\\n";
// 		body.erase(0, bytes);
// 	}
// 	str += "0\\r\\n\\r\\n";
// 	std::cout << str << std::endl;
// }

