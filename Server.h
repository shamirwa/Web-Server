#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

class Server{

	private:
		int sockfd, newsockfd, portno;
		struct sockaddr_in serv_addr;

	public:
		Server(){
			sockfd = -1;
			newsockfd = -1;
       			portno = 8000;
			bzero((char*) &serv_addr, sizeof(serv_addr));
		}

		void createSockFd();

		const int& getSockFd();
		
		int& getNewSockFd();		

		void setPortNo(int port);


		const int& getPortNo();

		void setServerAddr();
       		
		struct sockaddr_in* getServerAddrPtr();

		struct sockaddr_in getServerAddr();

		const bool validatePort(const int&);

		const bool validateTimeout(const int&);

};

#endif
