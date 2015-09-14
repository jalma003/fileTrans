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
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <netinet/in.h>
#include <wait.h>
#include <netdb.h>
#include <errno.h>
#include <pthread.h>
#include <sstream>
#include <pwd.h>

int PORT = 67120;
struct hostent *hp;
char dirName[1024];

#define numThreads 10

using namespace std;

void createSocket(int &socketfd)
{
	if((socketfd = socket(AF_INET, SOCK_STREAM,0)) < 0)
	{
		perror("client socket failed");
		exit(1);
	}
	else
		cout << "socket creation OK" << endl;
}

void connectSocket(int &socketfd, sockaddr_in &client)
{
	if(connect(socketfd, (struct sockaddr *)&client, sizeof(client)) < 0)
	{
		perror("Client connection error");
		exit(1);
	}
	else
		cout << "connect OK" << endl;
}

void createDir(char* name, string path)
{
	path += string(name);
	if(mkdir(path.c_str(), S_IRWXU) < 0)
	{
		cout << "Error(" << errno << ")	: " << strerror(errno) << endl;
		exit(1);
	}
}

void createFile(int &clientfd, string &dir)
{
	char buf[30000];
	memset(&buf, 0, 30000);
	read(clientfd, buf, 30000);
	write(clientfd, "r", 2);
	if (FILE *fp = fopen(dir.c_str(), "wb"))
	{
		fwrite(buf, 1, 30000, fp);
		fclose(fp);
	}
	memset(&buf, 0, 30000);
}

string changeDirName(int thread)
{
	string nameDir = "./Thread";
	ostringstream convert;
	convert << thread;
	nameDir += convert.str();
	nameDir += "files/";

	createDir("", nameDir);
	return nameDir;
}

bool receive(int &socketfd, string path, int thread)
{
	char sig[2];
	char name[1024];
	string tempPath;
	read(socketfd, sig, 2);
	write(socketfd, "r", 2);

	if(string(sig) == "d")
	{
		read(socketfd, name, sizeof(name));
		createDir(name, path);
	}
	else if(string(sig) == "f")
	{
		tempPath = path;
		read(socketfd, name, 1024);
		tempPath += string(name);
		write(socketfd, "r", 2);
		path += string(name);
		createFile(socketfd, path);
	}
	else if(string(sig) == "n")
	{
		cout << dirName << " is not a directory" << endl;
		exit(0);
	}
	else if(string(sig) == "x")
	{
		cout << "Transfer for thread " << thread << " complete" << endl;
		return 0;
	}

	write(socketfd, "r", 2);
	return 1;
}

void *multThreading(void *threadid)
{
	int thread = (long)threadid;
	int socketfd;
	struct sockaddr_in client;
	char buff[30000];
    string temp = string(dirName);
    
	client.sin_family = AF_INET;
	client.sin_port = htons(PORT);
	client.sin_addr = *((struct in_addr *) hp->h_addr);

	string path;
	path = changeDirName(thread);

	createSocket(socketfd);

	connectSocket(socketfd, client);
	write(socketfd, dirName, 1024);
    
	while(receive(socketfd, path, thread));

	close(socketfd);

	pthread_exit(NULL);
}

int main(int argc, char* argv[])
{
	pthread_t threadid;
	if(argc < 3)
	{
		cout << "Not enough arguments" << endl;
		exit(0);
	}

	strcpy(dirName, argv[2]);
    
	if((hp = gethostbyname(argv[1])) == NULL)
	{
		perror("gethostbyname failed");
		exit(1);
	}
	else
		cout << "gethostbyname OK" << endl;

	for(int i = 0; i < numThreads; i++)
	{
		if(pthread_create(&threadid, NULL, multThreading, (void*) i) < 0)
		{
			perror("thread creation error");
			exit(0);
		}
	}
	pthread_exit(NULL);
}
