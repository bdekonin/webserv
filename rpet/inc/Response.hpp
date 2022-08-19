#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <string>
# include <vector>

class Response
{
	public:
		// Coplien form
		Response();
		Response(Response const &src);
		Response		&operator=(Response const &src);
		virtual ~Response();

		// Public functions
		void						reset();
		void						setBody(std::string const &body);
		void						setBody(const char *body, int length);
		void						setHeader(std::string const &header);
		void						setStatusCode(int code);
		void						setDefaultError();
		void						setBytesSent(size_t bytesSent);
		std::vector<unsigned char>	getBody();
		std::string					getHeaders();
		int							getStatusCode();
		std::vector<unsigned char>	getResponseText();
		size_t						getBytesSent();
		bool						isDefaultError();
		bool						isError();
		bool						isResponseBuilt();
		bool						isSentEntirely();
		void						buildResponseText(std::string const &statusText);

	private:
		// Private variables
		std::vector<unsigned char>	_body;
		std::string					_headers;
		int							_statusCode;
		bool						_defaultError;
		std::vector<unsigned char>	_responseText;
		bool						_responseBuilt;
		size_t						_bytesSent;
};

#endif
