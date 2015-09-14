/*
* Course: CS 100 Fall 2013
*
* First Name: John
* Last Name: Almaraz
* Username: jalma003
* email address: jalma003@ucr.edu
*
*
* Assignment: HW #9
*
* I hereby certify that the contents of this file represent
* my own original individual work. Nowhere herein is there
* code from any outside resources such as another individual,
* a website, or publishings unless specifically designated as
* permissible by the instructor or TA. */

#include <iostream>
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <errno.h>
#include <wait.h>
#include <unistd.h>
#include <cstdlib>

using namespace std;

int PORT = 67120;

void getDir(int &clientfd, char* dir)
{
	string tempDir = string(dir);
	DIR* ptr;
	struct stat status;
	struct dirent* direntPtr;
	char buf[30000], sig[2];
	string path;
	ptr = opendir(dir);
	if(ptr)
	{
		write(clientfd, "d", 2), read(clientfd, sig, 2);
		write(clientfd, dir, 1024), read(clientfd, sig, 2);
		while((direntPtr = readdir(ptr)))
		{
			path = tempDir + "/" + string(direntPtr->d_name);
			stat(path.c_str(), &status);
			if(S_ISDIR(status.st_mode) && string(direntPtr->d_name) != "." &&
               string(direntPtr->d_name) != "..")
				getDir(clientfd, (char*)path.c_str());
			else if(string(direntPtr->d_name) != "." &&
					string(direntPtr->d_name) != "..")
			{
				write(clientfd, "f", 2), read(clientfd, sig, 2);
				write(clientfd, path.c_str(), 1024), read(clientfd, sig, 2);
				FILE *fp;
				memset(&buf, 0, 30000);
				if((fp = fopen(path.c_str(), "r")))
				{
					fread(buf, 1, sizeof(buf), fp);
					fclose(fp);
				}
				write(clientfd, buf, sizeof(buf)), read(clientfd, sig, 2);
				memset(&buf, 0, 30000);
			}
		}
	}
	else
	{
		cout.flush();
		stat(dir, &status);
		if(!S_ISDIR(status.st_mode))
			write(clientfd, "n", 2);
	}
}

void createSocket(int &socketfd)
{
	if((socketfd = socket(AF_INET,SOCK_STREAM,0)) < 0)
	{
		perror("Socket creation failed");
		exit(1);
	}
	else
		cout << "Socket creation OK" << endl;
}

void bindSock(int &socketfd, struct sockaddr_in &server)
{
	if(bind(socketfd,(struct sockaddr *)&server, sizeof(server)) < 0)
	{
		perror("Binding error");
		exit(1);
	}
	else
		cout << "Binding OK" << endl;
}

void listening(int &socketfd)
{
	if(listen(socketfd, 10) < 0)
	{
		perror("listening failure");
		exit(1);
	}
	else
		cout << "listening..." << endl;
}

void acceptSock(int &socketfd, int &clientfd, sockaddr_in &client, 
				socklen_t &size, char* buff)
{
	int pid, status;
	char *sig;
	if((clientfd = accept(socketfd,(struct sockaddr *)&client, &size)) < 0)
	{
		perror("accept failed");
		exit(1);
	}
	else
	{
		cout << "accept OK" << endl;
		pid = fork();
		if(pid == 0)
		{
			read(clientfd, buff, 1024);
			close(socketfd);
			getDir(clientfd, buff);
			read(clientfd, sig, 2);
			write(clientfd, "x", 2);
			close(clientfd);
			cout << "transfer complete" << endl;
			exit(0);
		}
	}
}

int main()
{
	int socketfd, clientfd;
	struct sockaddr_in server, client;
	char buff[30000];
	
	createSocket(socketfd);

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT);
	
	bindSock(socketfd, server);

	listening(socketfd);

	socklen_t size = sizeof(server);
	sleep(1);
	while(1)
		acceptSock(socketfd, clientfd, client, size, buff);
		
	close(clientfd);
	close(socketfd);
	
	return 0;
}
