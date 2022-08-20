#ifndef SERVER_HPP
# define SERVER_HPP

# include "ServerConfig.hpp"	// ServerConfig
# include "Task.hpp"			// Task
# include "CGI.hpp"				// CGI
# include <string>				// std::string

class Task; // Forward declaration Task

class Server
{
	public:
		// Enum for type path.
		enum	PathType
		{
			INVALID,
			DIRECTORY,
			DOCUMENT
		};

		// Coplien form
		Server(Server const &src);
		virtual ~Server();
		Server	&operator=(Server const &src);

		// Parameterized constructor
		Server(ServerConfig const &conf);

		// Public functions
		const ServerConfig	&getConfig() const;
		void				handleConnection(Task &task);
	
	private:
		// Private default constructor
		Server();

		// Private functions
		int			_openFile(Task &client, std::string &pathName);
		void		_checkErrorPath(Task &task);
		PathType	_pathType(std::string const &path);
		bool		_isValidContentLength(Task &task);
		void		_generateDirectoryListing(Task &task, std::string const &URI);
		void		_launchCGI(Task &task, std::string const &CGIPath);
		void		_setEnvironmentVars(Task &task);
		void		_trySetVar(const char *name, std::string const &value);
		void		_closePipesAndError(Task &task, CGI *cgi);
		int			_checkForInitialErrors(Task &task);
		void		_deleteFile(Task &task, std::string const &path);
		void		_setPageNotFoundError(Task &task);

		// Private variables
		ServerConfig	_config;
};

#endif
