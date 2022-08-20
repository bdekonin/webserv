#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "Request.hpp"		// Request
# include "Response.hpp"	// Response
# include <netinet/in.h>	// sockaddr_in
# include <string>			// std::string

class Client
{
	public:
		// Coplien form stuff
		Client();
		Client(Client const &src);
		Client	&operator=(Client const &src);
		virtual ~Client();
	
		// Parameterized constructor
		Client(int socketFD, struct sockaddr_in *addressInfo);

		// Public functions
		void				reset();
		Request				&getRequest();
		Response			&getResponse();
		std::string			&getAddress();
		void				checkBadRequest();

		// Public variables
		int					fd;

	private:
		// Private variables
		Request				_request;
		Response			_response;
		std::string			_address;
};

#endif
