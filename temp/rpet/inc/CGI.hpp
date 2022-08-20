#ifndef CGI_HPP
# define CGI_HPP

# include <string>	// std::string

class CGI
{
	public:
		// Coplien form stuff
		CGI();
		CGI(CGI const &src);
		CGI		&operator=(CGI const &src);
		virtual ~CGI();

		// Public functions
		std::string const	&getResponseBody();
		void				setResponseBody(std::string const &body);

		// Public variables
		int			fdin[2];
		int			fdout[2];

	private:
		std::string		_responseBody;
};

#endif
