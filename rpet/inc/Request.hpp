#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <string>		// std::string
# include <map>			// std::map
# include <vector>		// std::vector

class Request
{
	public:
		// Enum for the request read mode.
		enum	ReadMode
		{
			NOT_SET,
			NO_BODY,
			CONTENT_LENGTH,
			CHUNKED
		};

		// Coplien form stuff
		Request();
		Request(Request const &src);
		virtual ~Request();
		Request			&operator=(Request const &src);

		// Getters and setters
		void						reset();
		std::string					getMethod() const;
		std::string					getURI() const;
		std::string					getQueryString() const;
		std::string					getProtocol() const;
		std::string					getHeader(std::string const &headerName);
		std::vector<unsigned char>	&getBody();
		std::string					getMatchedServerName() const;
		std::string					getPort();
		size_t						getBodyLength() const;
		std::string					getBodyLengthString() const;
		std::vector<unsigned char>	&getIncomingMessage();
		void						setMethod(std::string const &method);
		void						setURI(std::string const &URI);
		void						setQueryString(std::string const &queryString);
		void						setProtocol(std::string const &protocol);
		void						setHeader(std::string const &header);
		void						setBody(std::vector<unsigned char>::iterator it, size_t length);
		void						setMatchedServerName(std::string const &matchedServerName);
		void						setPort(int port);
		void						setIncomingMessage(const char *message, int length);
		bool						isComplete();
		bool						isBadRequest();
		bool						isChunked();

	private:
		// Private variables
		std::string							_method;
		std::string							_URI;
		std::string							_queryString;
		std::string							_protocol;
		std::map<std::string, std::string>	_headers;
		std::vector<unsigned char>			_body;
		std::string							_matchedServerName;
		std::string							_port;
		std::vector<unsigned char>			_incomingMessage;
		ReadMode							_readMode;
		size_t								_contentLength;
		bool								_badRequest;
		bool								_doneReading;

		void				_parseHeaders(std::string &allHeaders);
		void				_parseRequestLine(std::string &header);
		void				_findReadMode();
		void				_addContentLength();
		void				_parseChunk();
};

#endif
