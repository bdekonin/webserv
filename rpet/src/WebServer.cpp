#include "WebServer.hpp"	// WebServer
#include <sys/socket.h>		// socket, bind, listen, AF_INET, SOCK_STREAM,
							// recv, getsockname
#include <sys/select.h>		// select
#include <netinet/in.h>		// struct sockaddr_in, INADDR_ANY
#include <stdexcept>		// std::runtime_error
#include <vector>			// std::vector
#include <string>			// std::string
#include <strings.h>		// bzero
#include <unistd.h>			// read
#include <fcntl.h>			// fcntl, F_SETFL, O_NONBLOCK
#include <cstdlib>			// exit
#include <sstream>			// stringstream, istringstream
#include "Server.hpp"		// Server
#include "ServerConfig.hpp"	// ServerConfig
#include "Task.hpp"			// Task
#include "Request.hpp"		// Request
#include "Response.hpp"		// Response
#include "CGI.hpp"			// CGI
#include "StringUtils.hpp"	// StringUtils
#include <arpa/inet.h>		// inet_ntop, ntohs
#include <cstdio>			// perror

/*
 * The constructor is the only starting point for the WebServer. We
 * create the sockets and Server objects, and then run the main loop
 * indefinitely.
 */
WebServer::WebServer()
{
}

/*
 * Destructor
 */
WebServer::~WebServer()
{
}

/*
 * This function is the main loop of the program, it will keep running
 * forever. It waits for incoming connections or other work to be ready,
 * and then handles it.
 */
void		WebServer::run()
{
	fd_set								read_tmp;
	fd_set								write_tmp;
	std::map<int, Task>::iterator		it;
	Task								currentTask;

	while (true)
	{
		this->_readFDToRemove.clear();
		this->_writeFDToRemove.clear();
		read_tmp = this->_readable;
		write_tmp = this->_writeable;
		if (select(FD_SETSIZE, &read_tmp, &write_tmp, NULL, NULL) < 0)
		{
			perror("select");
			continue ;
		}
		for (it = this->_readTaskMap.begin(); it != this->_readTaskMap.end(); it++)
		{
			currentTask = it->second;
			if (this->_isFDInRemovalSet(currentTask.fd, this->_readFDToRemove) == true)
				continue ;
			if (FD_ISSET(it->first, &read_tmp))
			{
				if (currentTask.type == Task::WAIT_FOR_CONNECTION)
					this->_acceptConnection(it->first);
				else if (currentTask.type == Task::CLIENT_READ)
					this->_readRequest(this->_clientMap[it->first]);
				else if (currentTask.type == Task::FILE_READ)
					this->_readFile(currentTask);
				else if (currentTask.type == Task::CGI_READ)
					this->_readCGI(currentTask);
			}
		}
		for (it = this->_writeTaskMap.begin(); it != this->_writeTaskMap.end(); it++)
		{
			currentTask = it->second;
			if (this->_isFDInRemovalSet(currentTask.fd, this->_writeFDToRemove) == true)
				continue ;
			if (FD_ISSET(it->first, &write_tmp))
			{
				if (currentTask.type == Task::CLIENT_RESPONSE)
				{
					if (this->_sendResponse(currentTask))
						FD_CLR(it->first, &this->_writeable);
				}
				else if (currentTask.type == Task::FILE_WRITE)
					this->_writeFile(currentTask);
				else if(currentTask.type == Task::CGI_WRITE)
					this->_writeCGI(currentTask);
			}
		}
		for (std::vector<int>::iterator it = this->_readFDToRemove.begin(); it != this->_readFDToRemove.end(); it++)
			this->_readTaskMap.erase(*it);
		for (std::vector<int>::iterator it = this->_writeFDToRemove.begin(); it != this->_writeFDToRemove.end(); it++)
			this->_writeTaskMap.erase(*it);
	}
}

/*
 * This function creates sockets, binds them to ports and starts listening to
 * them.
 */
void		WebServer::setup(std::vector<ServerConfig> const &serverConfigs)
{
	// Iterate over the ServerConfigs we've received from the parser.
	// Create a Server instance with this config for every port it wants
	// to listen on.
	std::vector<ServerConfig>::const_iterator		it;
	std::vector<size_t>								ports;
	std::vector<size_t>::const_iterator				portIt;
	ServerConfig									current;
	std::map<int, std::vector<Server> >::iterator	serverIt;
	for (it = serverConfigs.begin(); it != serverConfigs.end(); it++)
	{
		current = *it;
		ports = current.getPort();
		for (portIt = ports.begin(); portIt != ports.end(); portIt++)
			this->_servers[*portIt].push_back(Server(current));
	}
	for (serverIt = this->_servers.begin(); serverIt != this->_servers.end(); serverIt++)
		this->_sockets.push_back(this->_openSocket(serverIt->first));

	// Zero out the fd sets.
	FD_ZERO(&this->_readable);
	FD_ZERO(&this->_writeable);

	// Put the server sockets into the readable fd set.
	std::vector<int>::iterator		socketIt;
	Task							tmpTask;
	for (socketIt = this->_sockets.begin(); socketIt != this->_sockets.end(); socketIt++)
	{
		tmpTask.type = Task::WAIT_FOR_CONNECTION;
		tmpTask.fd = *socketIt;
		this->_addTask(tmpTask);
	}
	this->_responsesSetup();
}

/*
 * This function opens a socket and returns the file descriptor.
 */
int			WebServer::_openSocket(int port)
{
	struct sockaddr_in		serverAddress;
	int						ret;
	int						socketFD;

	socketFD = socket(AF_INET, SOCK_STREAM, 0);
	if (socketFD < 0)
		throw std::runtime_error("Failed to create socket.");
	int options = 1;
	ret = setsockopt(socketFD, SOL_SOCKET, SO_REUSEPORT, &options, sizeof(options));
	if (socketFD < 0)
		throw std::runtime_error("Failed to set socket options.");

	// Initialize the address struct that bind will use.
	bzero(&serverAddress, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(port);

	// Bind the socket to a port.
	ret = bind(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
	if (ret < 0)
		throw std::runtime_error("Failed to bind socket to port.");

	// Set the socket to listen for incoming connections.
	ret = listen(socketFD, 10);
	if (ret < 0)
		throw std::runtime_error("Failed to make socket listen to incoming connections.");

	return socketFD;
}

/*
 * This function accepts a connection from a socket, and returns the
 * client's socket file descriptor.
 */
void		WebServer::_acceptConnection(int socketFD)
{
	struct sockaddr_in	clientAddress;
	int					addressSize = sizeof(struct sockaddr_in);
	int					clientSocketFD;

	// Accept the incoming connection.
	clientSocketFD = accept(socketFD, (struct sockaddr*)&clientAddress, (socklen_t*)&addressSize);
	fcntl(clientSocketFD, F_SETFL, O_NONBLOCK);
	Client				client(clientSocketFD, &clientAddress);
	if (client.fd < 0)
	{
		perror("accept");
		return ;
	}
	this->_clientMap[client.fd] = client;

	// Create a new task.
	Task				task(Task::CLIENT_READ, client.fd, 0, &this->_clientMap[client.fd]);
	this->_addTask(task);
}

/*
 * This function reads the incoming HTTP request.
 */
void		WebServer::_readRequest(Client &client)
{
	char	buffer[RECVBUFFERSIZE + 1];
	int		bytesRead;

	// Read from the socket. If we read 0 bytes, the connection was
	// closed by the client.
	bzero(buffer, RECVBUFFERSIZE + 1);
	bytesRead = recv(client.fd, buffer, RECVBUFFERSIZE, 0);
	if (bytesRead <= 0)
	{
		// Connection was closed on the client's side.
		close(client.fd);
		this->_removeTasksForClient(client.fd);
		this->_clientMap.erase(client.fd);
		return ;
	}

	// Update the request, and ask if we're done reading.
	client.getRequest().setIncomingMessage(buffer, bytesRead);
	if (!client.getRequest().isComplete())
		return ;

	// Check if the request is bad from the start.
	client.checkBadRequest();

	// Now find the server to handle this request.
	this->_findServerForRequest(client);
}

/*
 * This function finds the server that should handle the request of the client.
 */
void		WebServer::_findServerForRequest(Client &client)
{
	std::string									host;
	std::string									hostName;
	int											port;
	Task										task;
	std::vector<Server>							serversForPort;
	bool										serverFound;
	std::vector<Server>::iterator				serverIt;
	ServerConfig								serverConfig;
	std::vector<std::string>::const_iterator	nameIt;

	task.client = &client;

	// Find the port and hostname.
	port = this->_getPortFromSocket(client.fd);
	host = client.getRequest().getHeader("Host");
	if (host == "")
		hostName = client.getAddress();
	else
	{
		size_t		colonPosition = host.find(":");
		hostName = host.substr(0, colonPosition);

		// Check if a port was provided, and if it matches the port the request came in on.
		if (colonPosition != std::string::npos)
		{
			std::stringstream		conversionStream;
			int						givenPort;

			conversionStream << host.substr(colonPosition + 1);
			conversionStream >> givenPort;
			if (conversionStream.str().size() == 0 || givenPort != port)
				client.getResponse().setStatusCode(400);
		}
	}
	client.getRequest().setPort(port);

	// Get the servers for this port.
	serversForPort = this->_servers[port];

	// Find the correct server to handle this request.
	serverFound = false;
	for (serverIt = serversForPort.begin(); serverIt != serversForPort.end(); serverIt++)
	{
		if (serverFound)
			break;
		serverConfig = serverIt->getConfig();
		std::vector<std::string> const serverNames = serverConfig.getServerNames();
		for (nameIt = serverNames.begin(); nameIt != serverNames.end(); nameIt++)
		{
			if (*nameIt == hostName)
			{
				client.getRequest().setMatchedServerName(hostName);
				serverIt->handleConnection(task);
				serverFound = true;
				break;
			}
		}
	}
	// Drop to the default if we haven't found a suitable server in the list.
	if (!serverFound)
	{
		serverConfig = serversForPort[0].getConfig();
		client.getRequest().setMatchedServerName(serverConfig.getDefaultServerName());
		serversForPort[0].handleConnection(task);
	}
	this->_addTask(task);
}

/*
 * This function reads a file into a buffer, to send to the Client later.
 */
void		WebServer::_readFile(Task &task)
{
	char		buffer[READBUFFERSIZE + 1];
	int			bytesRead;
	Task		responseTask;

	bzero(buffer, READBUFFERSIZE + 1);
	bytesRead = read(task.fd, buffer, READBUFFERSIZE);
	if (bytesRead < 0)
	{
		perror("read");
		return ;
	}
	else if (bytesRead > 0)
	{
		task.client->getResponse().setBody(buffer, bytesRead);
		return ;
	}

	close(task.fd);
	responseTask.type = Task::CLIENT_RESPONSE;
	responseTask.client = task.client;
	responseTask.fd = task.client->fd;
	this->_addTask(responseTask);
	this->_markFDForRemoval(task.fd, this->_readable, READ);
}

/*
 * This function is used to upload a file.
 */
void		WebServer::_writeFile(Task &task)
{
	std::stringstream			conversionStream;
	size_t						contentLength;
	std::vector<unsigned char>	&bodyVector = task.client->getRequest().getBody();
	Task						responseTask;
	char						*body = reinterpret_cast<char*>(&bodyVector[0]);

	conversionStream << task.client->getRequest().getHeader("Content-Length");
	conversionStream >> contentLength;
	write(task.fd, body, contentLength);
	close(task.fd);
	responseTask.type = Task::CLIENT_RESPONSE;
	responseTask.client = task.client;
	responseTask.fd = task.client->fd;
	this->_addTask(responseTask);
	this->_markFDForRemoval(task.fd, this->_writeable, WRITE);
}

/*
 * This function reads the output from the CGI.
 */
void		WebServer::_readCGI(Task &task)
{
	char		buffer[READBUFFERSIZE + 1];
	int			bytesRead;
	Task		responseTask;
	CGI			*cgi;

	cgi = task.cgi;
	bzero(buffer, READBUFFERSIZE + 1);
	bytesRead = read(task.fd, buffer, READBUFFERSIZE);
	if (bytesRead < 0)
	{
		perror("read");
		return ;
	}
	else if (bytesRead > 0)
	{
		cgi->setResponseBody(buffer);
		return ;
	}

	size_t		lineStart = 0;
	size_t		prevStart;
	std::string	responseMessage = cgi->getResponseBody();
	std::string	header, body;

	// Process the headers.
	while (true)
	{
		prevStart = lineStart;
		header = StringUtils::getString(&lineStart, responseMessage, "\r\n");
		if (prevStart == lineStart || header == "")
			break;
		else
			task.client->getResponse().setHeader(header);
	}

	// Put the remaining message into the body.
	body = StringUtils::getString(&lineStart, responseMessage, "\r\n\r\n");
	task.client->getResponse().setBody(body);
	close(task.fd);
	responseTask.type = Task::CLIENT_RESPONSE;
	responseTask.client = task.client;
	responseTask.fd = task.client->fd;
	this->_addTask(responseTask);
	this->_markFDForRemoval(task.fd, this->_readable, READ);

	// We're done with the CGI, delete it.
	task.cgi = 0;
	delete cgi;
}

/*
 *
 */
void		WebServer::_writeCGI(Task &task)
{
	int							ret;
	std::vector<unsigned char>	&bodyVector = task.client->getRequest().getBody();
	char						*body = reinterpret_cast<char*>(&bodyVector[0]);
	Task						newTask;

	ret = write(task.fd, body, task.client->getRequest().getBodyLength());
	close(task.fd);
	newTask = task;
	newTask.fd = task.cgi->fdout[0];
	newTask.type = Task::CGI_READ;
	this->_addTask(newTask);
	this->_markFDForRemoval(task.fd, this->_writeable, WRITE);
}

/*
 * This function sends a response back to the client.
 */
bool		WebServer::_sendResponse(Task &task)
{
	int							statusCode;
	int							ret;
	std::vector<unsigned char>	responseVector;
	char						*responseChar;
	std::string					body;
	std::stringstream			contentLength;
	size_t						bytesSent;
	Client						*client = task.client;
	Response					&response = client->getResponse();

	// Check if we need to build the response first.
	if (response.isResponseBuilt() == false)
	{
		statusCode = response.getStatusCode();
		contentLength << "Content-Length: ";
		if (response.isError() && response.isDefaultError() == true)
		{
			body = this->_defaultError;
			_replaceDefaultErrorMessage(body, statusCode);
			response.setBody(body);
			contentLength << body.size();
		}
		else
		{
			std::vector<unsigned char>	tmp = response.getBody();
			contentLength << tmp.size();
		}
		response.setHeader(contentLength.str());
		response.buildResponseText(this->_responseStatus[response.getStatusCode()]);
	}

	// Get the response text and try to send it.
	responseVector = response.getResponseText();
	bytesSent = response.getBytesSent();
	responseChar = reinterpret_cast<char*>(&responseVector[bytesSent]);
	ret = send(client->fd, responseChar, responseVector.size() - bytesSent, 0);
	response.setBytesSent(ret);

	// Check if the entire message was sent.
	if (response.isSentEntirely() == false)
		return false;

	// We're done, reset the client and remove the task.
	task.client->reset();
	this->_markFDForRemoval(task.fd, this->_writeable, WRITE);
	return true;
}

/*
 * This function adds a task to the correct task map, and sets the fd in the
 * correct fd set.
 */
void		WebServer::_addTask(Task &task)
{
	if (this->_getTaskIOType(task) == READ)
	{
		if (this->_isFDInRemovalSet(task.fd, this->_readFDToRemove) == true)
		{
			if (task.type == Task::FILE_READ)
				close(task.fd);
			return ;
		}
		this->_readTaskMap[task.fd] = task;
		FD_SET(task.fd, &this->_readable);
	}
	else
	{
		if (this->_isFDInRemovalSet(task.fd, this->_writeFDToRemove) == true)
			return ;
		this->_writeTaskMap[task.fd] = task;
		FD_SET(task.fd, &this->_writeable);
	}
}

/*
 * This function marks a file descriptor for removal at the end of the
 * select loop.
 */
void		WebServer::_markFDForRemoval(int fd, fd_set &set, TaskIOType mode)
{
	if (mode == READ)
		this->_readFDToRemove.push_back(fd);
	else
		this->_writeFDToRemove.push_back(fd);

	FD_CLR(fd, &set);
}

/*
 * This function edits the body to the correct message in case an error occurred.
 */
void		WebServer::_replaceDefaultErrorMessage(std::string &body, int errorCode)
{
	size_t				pos;
	std::stringstream	errorCodeString;
	std::string const	replaceCode = "ERROR_CODE";
	std::string const	replaceMessage = "ERROR_MESSAGE";

	if ((pos = body.find(replaceCode)) != std::string::npos)
	{
		errorCodeString << errorCode;
		body = body.replace(pos, replaceCode.length(), errorCodeString.str());
	}
	if ((pos = body.find(replaceMessage)) != std::string::npos)
	{
		body = body.replace(pos, replaceMessage.length(), this->_responseStatus[errorCode]);
	}
}

/*
 * This function puts all the possible HTML errors in a map
 */
void		WebServer::_responsesSetup()
{
	this->_defaultError = "<!DOCTYPE html>\r\n<html>\r\n<head>\r\n<title>Spinnenwebservetjes</title>\r\n<style>\r\nbody{background: url('/spin.png');\r\nbackground-repeat: no-repeat; background-position-y: 100px;}\r\n</style>\r\n</head>\r\n<body>\r\n<h1>HTTP error: ERROR_CODE ERROR_MESSAGE</h1>\r\n<p>&copy; Spinnenwebservetjes</p>\r\n</body>\r\n</html>\r\n";
	this->_responseStatus[200] = "OK";
	this->_responseStatus[201] = "Created";
	this->_responseStatus[204] = "No Content";
	this->_responseStatus[301] = "Moved Permanently";
	this->_responseStatus[400] = "Bad Request";
	this->_responseStatus[403] = "Forbidden";
	this->_responseStatus[404] = "Not Found";
	this->_responseStatus[405] = "Method Not Allowed";
	this->_responseStatus[413] = "Payload Too Large";
	this->_responseStatus[500] = "Internal Server Error";
}

/*
 * This function returns a TaskIOType enum based on the Task given as argument.
 */
WebServer::TaskIOType	WebServer::_getTaskIOType(Task &task)
{
	if (task.type == Task::WAIT_FOR_CONNECTION ||
			task.type == Task::CLIENT_READ ||
			task.type == Task::FILE_READ ||
			task.type == Task::CGI_READ)
		return READ;
	return WRITE;
}

/*
 * This function will remove all current tasks that have the clientFD as their
 * client's fd.
 */
void		WebServer::_removeTasksForClient(int clientFD)
{
	std::map<int, Task>::iterator		it;

	for (it = this->_readTaskMap.begin(); it != this->_readTaskMap.end(); it++)
	{
		if (it->second.type != Task::WAIT_FOR_CONNECTION && it->second.client->fd == clientFD)
		{
			close(it->first);
			this->_markFDForRemoval(it->second.fd, this->_readable, READ);
		}
	}
	for (it = this->_writeTaskMap.begin(); it != this->_writeTaskMap.end(); it++)
	{
		if (it->second.client->fd == clientFD)
		{
			close(it->first);
			this->_markFDForRemoval(it->second.fd, this->_writeable, WRITE);
		}
	}
}

/*
 * This function will check if an FD is present in a set for deletion
 */
bool		WebServer::_isFDInRemovalSet(int fd, std::vector<int> &removalVector)
{
	std::vector<int>::iterator	it;


	for (it = removalVector.begin(); it != removalVector.end(); it++)
	{
		if (*it == fd)
			return true;
	}
	return false;
}

/*
 * This function gets the port from a socket.
 */
int			WebServer::_getPortFromSocket(int fd)
{
	struct sockaddr_in	address;
	int					addressSize = sizeof(struct sockaddr_in);

	int ret = getsockname(fd, (struct sockaddr*)&address, (socklen_t*)&addressSize);
	if (ret < 0)
		perror("getsockname");
	return (ntohs(address.sin_port));
}
