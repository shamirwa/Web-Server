#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

class Client{

	private:
		int sockfd;
		struct sockaddr_in serv_addr;
		struct timeval start, end;	

	public:
		Client(){
			sockfd = -1;
			bzero((char*) &serv_addr, sizeof(serv_addr));
		}

		int createSockFd();

		const int& getSockFd();
		
		int setServerAddr(int portNo, char* ipAddr);
       		
		struct sockaddr_in* getServerAddrPtr();

		struct sockaddr_in getServerAddr();
		
		struct timeval& getStartTime();
			
		struct timeval& getEndTime();
	
		void setStartTime();

		void setEndTime();

		double getDiffTime();
};

#endif
