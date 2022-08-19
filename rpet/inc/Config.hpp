#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <vector>
#include <map>
#include <ostream>

class Config
{
	public:
		//constructors
		virtual ~Config();
		Config();
		Config(const Config&);
		Config&	operator=(const Config&);

		enum	e_httpMethods
		{
			GET,
			POST,
			DELETE
		};

		//get()
		size_t  									getLimitClientBodySize() const;
		const std::string&							getRoot() const;
		const std::vector<std::string>&				getIndex() const;
		bool    									getAutoindex() const;
		bool    									isValidHttpMethod(const std::string&) const;
		const std::string&							getErrorPage(size_t) const;
		bool										hasErrorPage(size_t) const;
		bool										hasCGI(const std::string&) const;
		const std::string							getCGI(const std::string&) const;
		bool										hasUploadDir() const;
		const std::string&							getUploadDir() const;
		size_t										getAmountErrorPages() const;
		size_t										getAmountCGI() const;
		const std::map<size_t, std::string>&		getMapErrorPages() const;
		const std::map<std::string, std::string>&	getMapCGI() const;
		bool										getHttpMethods(enum	e_httpMethods method) const;
		bool										isRedirection() const;
		const std::string&							getRedirection() const;

		//set()
		void	setLimitClientBodySize(const std::string&);
		void	setRoot(const std::string&);
		void	setIndex(const std::string&);
		void	setAutoindex(const std::string&);
		void	setHttpMethods(const std::string&);
		void	setCGI(const std::string&);
		void	setUploadDir(const std::string&);
		void	setErrorPage(const std::string&);
		void	setRedirection(const std::string&);

	protected:
		size_t								_limitClientBodySize;
		std::string							_root;
		std::vector<std::string>			_index;
		bool								_autoindex;
		bool								_httpMethods[3];
		size_t								_amountCGI;
		std::map<std::string, std::string>	_CGI;
		std::string							_uploadDir;
		size_t								_amountErrorpages;
		std::map<size_t, std::string>		_errorPage;
		std::string							_redirection;

	private:
		const std::string*	lookUpCGI(const std::string& string) const;
};

std::ostream&	operator<<(std::ostream& out, const Config& con);

#endif
