#ifndef TASK_HPP
# define TASK_HPP

# include "Server.hpp"
# include "Client.hpp"
# include "CGI.hpp"

class Server; // Forward declaration Server

class Task
{
	public:
		// Enum for the type of task.
		enum	TaskType
		{
			WAIT_FOR_CONNECTION,
			CLIENT_READ,
			CLIENT_RESPONSE,
			FILE_READ,
			FILE_WRITE,
			CGI_READ,
			CGI_WRITE
		};

		// Coplien form stuff
		Task();
		Task(Task const &src);
		virtual ~Task();
		Task			&operator=(Task const &src);

		// Parameterized constructor
		Task(TaskType type, int fd, Server *server, Client *client);

		// Public variables
		TaskType		type;
		int				fd;
		Server			*server;
		Client			*client;
		CGI				*cgi;

	private:
};

#endif
