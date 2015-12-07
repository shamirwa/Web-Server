#include "Client.h"
#include <iostream>
#include <sys/types.h>

using namespace std;

#define CLIENT_NUM 30
#define DEFAULT_SERVER_PORT 8000
#define DEFAULT_HTTP_PORT 1.0
#define BUFFER_SIZE 4096;

void error(const char* msg);

int main(int argc, char ** argv){
	int portNo;
	char ipAddr[16];
	double http;
	const char* fileName = "testFile.txt";	

	if(argc>4 ){
		cout<<"Too Many Parameters Entered"<<endl<<
		"Correct Usage are:- "<<endl<<"1)  myClient <hhtp> -p=<PORT> -a=<Server's IP ADDRESS>"<<endl<<
		"2) myClient -a=<Server's IP ADDRESS> - Here the port will be defaulted to 8000 and http version to 1.0"<<endl;

		return 0;
	}else if(argc == 4){
		http = atof(argv[1]);
		portNo = atoi(&(argv[1][3]));
		strcpy(ipAddr, &(argv[2][3]));
	}else if(argc == 2){
		string option1(argv[1]);

		if(option1.compare(0, 3, "-a=") == 0){
			strcpy(ipAddr, &(argv[1][3]));
			portNo = DEFAULT_SERVER_PORT;
			http = DEFAULT_HTTP_PORT;
		}else{
			cout<<"Wrong parameter Entered, Please enter IP Address as follows"<<endl<<
			"myClient -a=<Server's IP ADDRESS> - Here the port is defaulted to 8000 and http version to 1.0"<<endl;
		
			return 0;
		}
	}
	// Create the client objects
 	Client myClients[CLIENT_NUM];  	
	double diff;

	// Get the hostname
	char hostname[30];
	FILE* in;

	if(!(in = popen("hostname","r"))){
		error("ERROR: In executing hostname command");
	}
	fgets(hostname, sizeof(hostname), in);
	pclose(in);

	//FILE* fp;
	//FILE* fp2;

        //fp = fopen("/homes/shamirwa/cs536/client/ReadFile.txt", "a");
	// Open a file data.txt to store the data in the file.
	//fp2 = fopen("/homes/shamirwa/cs536/client/data.txt", "a");


        // Create separate child process for each client
	for(int i=CLIENT_NUM; i>0; i--){
		sleep(1);
		// Invoking the child process
		pid_t pid = fork();

		if(pid < 0){
			error("ERROR: Failed to create the child process");
		}else if(pid == 0){
			if(myClients[i].createSockFd()  == 1){
				error("ERROR: Failed to Create Socket");
			}

			if(myClients[i].setServerAddr(portNo, ipAddr) == 1){
				error("Setting the Server IP Address");
			}

			if(connect(myClients[i].getSockFd(), (struct sockaddr*)myClients[i].getServerAddrPtr(),
				        sizeof(myClients[i].getServerAddr())) < 0){
				error("ERROR: Failed to connect to server");
			}
			
			// Command to retrieve a a file from server
			char command[100];
			// start the timer before sending the request
			myClients[i].setStartTime();
		
			if(http == 1.0){
				sprintf(command, "GET /%s HTTP/1.0\r\n",fileName);	
				int n = write(myClients[i].getSockFd(), command, strlen(command));
				if(n<0) error("ERROR MEssage");
			}else if(http == 1.1){
				sprintf(command, "GET /%s HTTP/1.1\r\nHost: %s\r\n",fileName, hostname);
                                int n = write(myClients[i].getSockFd(), command, strlen(command));
			}
			
			char buff[4096];		
			int readData;
			FILE* fp;
	
			fp = fopen("/homes/shamirwa/cs536/client/ReadFile.txt", "a"); 
			int n = read(myClients[i].getSockFd(), buff, sizeof(buff));
			if(n<0) error("Error: Reading");
			fputs(buff, fp);
	
	
			fseek(fp, 0, SEEK_END);
			int totalRead = ftell(fp);
			rewind(fp);
			fclose(fp);
			
			myClients[i].setEndTime();
			diff = myClients[i].getDiffTime();
		
			// Open a file data.txt to store the data in the file.
			fp = fopen("/homes/shamirwa/cs536/client/data.txt", "a");
			char inforCollected[40];
			sprintf(inforCollected, "Bytes Read: %d\n Time Taken: %f\n HTTP: %f\n",totalRead, diff, http);
			fputs(inforCollected, fp);
			fclose(fp);

			 //Close the client socket
 		        close(myClients[i].getSockFd());

			exit(0);
		}
	}
	//fclose(fp);
	//fclose(fp2);
	return 0;
}
		 		

void error(const char* msg){
	perror(msg);
	exit(1);
}

