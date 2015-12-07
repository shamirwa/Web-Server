#include "Client.h"
#include <iostream>

using namespace std;

int Client::createSockFd(){
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return 1;

return 0;
}

const int& Client::getSockFd(){
	return sockfd;
}

int Client::setServerAddr(int portNo, char* ipAddr){
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portNo);
	if(inet_pton(AF_INET, ipAddr, &serv_addr.sin_addr) <= 0){
		return 1;
	}
return 0;
}

struct sockaddr_in* Client::getServerAddrPtr(){
	return &serv_addr;
}

struct sockaddr_in Client::getServerAddr(){
	return serv_addr;
}

struct timeval& Client::getStartTime(){
	return start;
}

struct timeval& Client::getEndTime(){
	return end;
}

void Client::setStartTime(){
	gettimeofday(&start, NULL);
}

void Client::setEndTime(){
	gettimeofday(&end, NULL);
}

double Client::getDiffTime(){
	return((end.tv_usec - start.tv_usec)/1000);	
} 	
