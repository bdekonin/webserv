// Server side C program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

# include <iostream>
# include <iomanip>
# include <sstream>
# include <fstream>
# include <string>
# include <limits>
# include <cstdio>

#define PORT 8080

#define main_page "HTTP/1.1 200 OK\nContent-Type:text/html\nContent-Length: 95\n\n<h1>Hello</h1><p>&nbsp;</p><p><a href='link_1'>Link 1</a></p><p><a href='link_2'>Link 2</a></p>"

#define link_1 "HTTP/1.1 200 OK\nContent-Type:text/html\nContent-Length: 15\n\n<h1>Link 1</h1>"
#define link_2 "HTTP/1.1 200 OK\nContent-Type:text/html\nContent-Length: 15\n\n<h1>Link 2</h1>"

#define favi "HTTP/1.1 200 OK\nContent-Type:image/x-icon\nContent-Length: 5430\n\n"
int main(int argc, char const *argv[])
{
    int server_fd, new_socket; long valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    
    char *hello = strdup("Hello from server");
    
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("In socket");
        exit(EXIT_FAILURE);
    }
    

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
    
    memset(address.sin_zero, '\0', sizeof address.sin_zero);
    
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("In bind");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 10) < 0)
    {
        perror("In listen");
        exit(EXIT_FAILURE);
    }
    while(1)
    {
        printf("\n+++++++ Waiting for new connection ++++++++\n\n");
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
        {
            perror("In accept");
            exit(EXIT_FAILURE);
        }
        char buffer[30000] = {0};


		valread = recv( new_socket , buffer, 30000, 0);

        // parse request-line
        std::string method, uri, version;

        sscanf(buffer, "%s %s %s", method, uri, version);

        

		// process
				if (buffer[10] == '1')
		{
			// do link_1
			char *arr= strdup(link_1);

			send(new_socket , arr , strlen(arr) , 0 );

		}
		else if (buffer[10] == '2')
		{
			// do link_2
			char *arr= strdup(link_2);

			send(new_socket , arr , strlen(arr) , 0 );
		}
        else if (buffer[4] == '/' && buffer[5] == 'f')
        {
            std::ifstream file;
            std::stringstream	buffer;

            file.open("/mnt/c/Users/Bob/Documents/webserv/favicon.ico", std::ifstream::in);

            buffer << file.rdbuf();

            std::string str = favi + buffer.str();

            char *arr = strdup(str.c_str());
            // print all variables
            std::cout << "buffer: [" << buffer.str() << "]" << std::endl;
            // size of buffer
            std::cout << "size of buffer: " << buffer.str().size() << std::endl;
        


			send(new_socket , str.c_str() , strlen(favi) + buffer.str().size() , 0 );
        }
		else
		{
			char *arr= strdup(main_page);

			send(new_socket , arr , strlen(arr) , 0 );
		}




        // valread = read( new_socket , buffer, 30000);
        printf("\n%s\n", buffer);

        // write(new_socket , hello , strlen(hello));
        // printf("------------------Hello message sent-------------------\n");
        close(new_socket);
    }
    return 0;
}