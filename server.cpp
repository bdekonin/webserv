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
# include <vector>

char	*ft_strjoin(char const *s1, char const *s2)
{
	char	*s3;
	int		i;
	int		j;

	i = -1;
	j = -1;
	if (s1 == NULL && s2 == NULL)
		return (NULL);
	if ((s3 = (char *)malloc(sizeof(char)
					* (strlen(s1) + strlen(s2) + 1))) == NULL)
		return (NULL);
	while (s1 != NULL && s1[++j] != '\0')
		s3[++i] = s1[j];
	j = -1;
	while (s2 != NULL && s2[++j] != '\0')
		s3[++i] = s2[j];
	s3[++i] = '\0';
	return (s3);
}

#define PORT 8081

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

        // inet_pton(AF_INET, "example.com", &(address.sin_addr));





		valread = recv( new_socket , buffer, 30000, 0);



        // parse request-line
        char temp[5][1000];
        std::string method, uri, version;

        sscanf(buffer, "%s %s %s", temp[0], temp[1], temp[2]);

            std::ofstream out("request.txt");
            out << buffer;
            out.close();

        method = temp[0];
        uri = temp[1];
        version = temp[2];

        // print vars
        printf("[%d] method: (%s) uri: (%s) version: (%s)\n", new_socket, temp[0], temp[1], temp[2]);

        if (method == "GET")
        {
            if (uri == "/")
            {
                char *response = strdup(main_page);

                printf("Sending response: [%s]\n", response);

                send(new_socket , response , strlen(response) , 0 );
                printf("Main page sent");

                free(response);
            }
            else
            {
                std::ifstream file;
                std::stringstream	buffer;
                
                file.open("/mnt/c/Users/Bob/Documents/webserv" + uri, std::ifstream::in);
                buffer << file.rdbuf();

                printf("looking at [%s%s]\n", "/mnt/c/Users/Bob/Documents/webserv", uri.c_str());
                std::string temp;

                if (uri.find("jpg") == uri.npos)
                    temp = "HTTP/1.1 200 OK\nContent-Type:text/html\nContent-Length: ";
                else
                    temp = "HTTP/1.1 200 OK\nContent-Type:image/jpeg\nContent-Length: ";
                // temp.insert(0, "HTTP/1.1 200 OK\nContent-Type:text/html\nContent-Length: ");

                temp.insert(temp.size(), std::to_string(buffer.str().size()));
                temp.insert(temp.size(), "\n\n");
                std::vector<char> v;

                v.insert(v.end(), temp.begin(), temp.end());
                v.insert(v.end(), buffer.str().begin(), buffer.str().end());

                char *response = &v[0];

                printf("looping\n");

                ssize_t bytes_send = 0;

                for (; bytes_send < buffer.str().size();)
                {
                    ssize_t bytes = send(new_socket, response + bytes_send,  buffer.str().size(), 0);
                    if (bytes == -1)
                    {
                        perror("send");
                        exit(1);
                    }
                    bytes_send += bytes;
                }

                // send(new_socket , response, strlen(response), 0);
                printf("Response [%ld][%s]\n", bytes_send, response);
            }
        }
        close(new_socket);
    }
    return 0;
}