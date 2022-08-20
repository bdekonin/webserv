#ifndef WEBSERVER_HPP
# define WEBSERVER_HPP

# include "Server.hpp"			// Server
# include "Task.hpp"			// Task
# include "Client.hpp"			// Client
# include "Request.hpp"			// Request
# include "ServerConfig.hpp"	// ServerConfig
# include <vector>				// std::vector
# include <map>					// std::map
# include <sys/select.h>		// fd_set

# define READBUFFERSIZE 4096
# define RECVBUFFERSIZE 4096

class WebServer
{
	public:
		// Enum for task removal mode.
		enum	TaskIOType
		{
			READ,
			WRITE
		};

		WebServer();
		virtual ~WebServer();

		void			run();
		void			setup(std::vector<ServerConfig> const &serverConigs);

	private:
		int				_openSocket(int port);
		void			_acceptConnection(int socketFD);
		void			_readRequest(Client &client);
		void			_findServerForRequest(Client &client);
		void			_readFile(Task &task);
		void			_writeFile(Task &task);
		void			_readCGI(Task &task);
		void			_writeCGI(Task &task);
		bool			_sendResponse(Task &task);
		void			_addTask(Task &task);
		void			_markFDForRemoval(int fd, fd_set &set, TaskIOType mode);
		void			_replaceDefaultErrorMessage(std::string &body, int errorCode);
		void			_responsesSetup();
		TaskIOType		_getTaskIOType(Task &task);
		void			_removeTasksForClient(int clientFD);
		bool			_isFDInRemovalSet(int fd, std::vector<int> &removalVector);
		int				_getPortFromSocket(int fd);

		std::vector<int>					_sockets;
		std::map<int, std::vector<Server> >	_servers;
		std::map<int, Task>					_readTaskMap;
		std::map<int, Task>					_writeTaskMap;
		std::map<int, Client>				_clientMap;
		std::map<int, std::string>			_responseStatus;
		fd_set								_readable;
		fd_set								_writeable;
		std::vector<int>					_readFDToRemove;
		std::vector<int>					_writeFDToRemove;
		std::string							_defaultError;
};

#endif
