#include "Server.h"
#include <iostream>

using namespace std;

void Server::createSockFd(){
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
}

const int& Server::getSockFd(){
	return sockfd;
}

void Server::setPortNo(int port){
	portno = port;
}

const int& Server::getPortNo(){
	return portno;
}

void Server::setServerAddr(){
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
}

struct sockaddr_in* Server::getServerAddrPtr(){
	return &serv_addr;
}

struct sockaddr_in Server::getServerAddr(){
	return serv_addr;
}

int& Server::getNewSockFd(){
	return newsockfd;
}


const bool Server::validatePort(const int& httpPort){
	if(httpPort <= 1024 || httpPort >= 65536){
                cout<<"Invalid Port Number Entered, it should be "
                "between 1024 - 65536"<<endl;

                return false;
        }
        else{
                return true;
        }
}

const bool Server::validateTimeout(const int& serverTimeout){
        if(serverTimeout <=0 ){
                cout<<"Invalid Timeout Value Entered, it should be greater"
		<<" than 0 seconds"<<endl;

                return false;
        }
        else{
                return true;
        }
}



