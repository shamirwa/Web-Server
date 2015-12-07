#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <iostream>
#include "Server.h"
#include "clientCommandHandlers.h"
#include <time.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <signal.h>

using namespace std;

#define DEFAULT_HTTP 1.0
#define DEFAULT_PORT 8000
#define DEFAULT_TIMEOUT 300
#define RESPONSE_SIZE 600
#define MAX_CLIENTS 30

int serverSocket;
int main(int argc, char *argv[])
{
	Server myServer;
	int port = DEFAULT_PORT, timeout = DEFAULT_TIMEOUT;
	double http_version = DEFAULT_HTTP;
	struct sockaddr_in cli_addr;
	socklen_t clilen = sizeof(cli_addr);
	time_t start, end;
	fd_set master;
	fd_set activeFds;
	int maxFd;
        struct timeval tv;
        
	FD_ZERO(&master);
	FD_ZERO(&activeFds);

        // Checking the number of arguments and validating each arguments
	if(argc > 4){
		cout<<"Too Many Parameters Entered"<<endl<< 
                "Usage Master <HTTP> -p=<PORT> -t=<TIMEOUT>"<<endl;
	
		return 0;
	}else if(argc == 4){
		http_version = atof(argv[1]);
		port = atoi(&(argv[2][3]));
		timeout = atoi(&(argv[3][3]));
	}else if(argc ==3){
		string option1(argv[1]);
		string option2(argv[2]);

		if(option1.compare(0,1,"-") == 0){
			port = atoi((option1.substr(3,option1.length())).c_str());

			timeout = atoi((option2.substr(3,option2.length())).c_str());
		}
		else{
			http_version = atof(option1.c_str());

			if(option2.compare(0,2,"-p") == 0){
				port = atoi((option2.substr(3,option2.length())).c_str());
			}
			else if(option2.compare(0,2,"-t") == 0){
				timeout = atoi((option2.substr(3,option2.length())).c_str());
			}	
		}
	}else if(argc == 2){
		string option(argv[1]);
		if(option.compare(0,2,"-p") == 0){
			port = atoi((option.substr(3,option.length())).c_str());
		}
		else if((option.compare(0,2,"-t")) == 0){
			timeout = atoi((option.substr(3,option.length())).c_str());
		}
		else{
			http_version = atof(argv[1]);
		}

	}else{
		port = DEFAULT_PORT;
		http_version = DEFAULT_HTTP;
		timeout = DEFAULT_TIMEOUT;
	} 
	
	if( !myServer.validatePort(port) || !myServer.validateTimeout(timeout) ){
		return 0;
	};
	
	// Setting the port number in the myServer object
	myServer.setPortNo(port);

	// Creating the Socket Descriptor for the myServer object
	myServer.createSockFd();

	if(myServer.getSockFd() < 0)
	{
		error("ERROR: Socket cannot be opened");
	}

        int yes=1;

        // Free the "Address already in use" error message
       if (setsockopt(myServer.getSockFd(),SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
                error("ERROR: While doing setsockopt");
        }


	myServer.setServerAddr();

	// Binding the socket
	if(bind(myServer.getSockFd(), (struct sockaddr*)myServer.getServerAddrPtr(),
		sizeof(myServer.getServerAddr())) < 0){

		error("ERROR: Socket Bind Failed");
	}

 	//Server can listen upto MAX_CLIENTS
	if( listen(myServer.getSockFd(),MAX_CLIENTS) == -1){
		error("ERROR: Listen Error");
	}
	// Initialize the server socket value to global variable
	// for use in Error function
	serverSocket = myServer.getSockFd();

	//Add the listener into master Fd's list
	FD_SET(myServer.getSockFd(), &master);

	// Initialize masxFd with the listener as this is the biggest
	// fd till now
	maxFd = myServer.getSockFd();

        tv.tv_sec = timeout;
	
	FILE* fp;
  	struct timeval startTime, EndTime;
        long diff;    

	for(;;){
		activeFds = master;
		tv.tv_sec = timeout;
                int selectOp = select(maxFd+1, &activeFds, NULL, NULL, &tv);
		if(selectOp == -1){
			close(myServer.getSockFd());
			error("ERROR: SELECT error");
		}else if(selectOp == 0){
			continue;
		}

		for(int i=0; i<=maxFd; i++){
			if(FD_ISSET(i, &activeFds))
			{
				if(i==myServer.getSockFd()){
					//New Connection Request
					// Server waits here until it 
					// listen from atleast one client
					//fp = fopen("/homes/shamirwa/cs536/server/ConnectionTime.txt", "a");
					gettimeofday(&startTime, NULL);
					myServer.getNewSockFd() = accept(
						myServer.getSockFd(),
						(struct sockaddr *) &cli_addr,
						&clilen);
					//gettimeofday(&EndTime, NULL);
					//diff = ((EndTime.tv_usec - startTime.tv_usec));
					//char fileWriteData[40];
					//sprintf(fileWriteData, "Connection Time: %ld",diff);
					//fputs(fileWriteData, fp);
					//fclose(fp);
                                          
        				if (myServer.getNewSockFd() < 0)
             					perror("ERROR: Connection Accept Failed");
 					else{ 
						FD_SET(myServer.getNewSockFd(),
							&master);
						if(myServer.getNewSockFd() > maxFd){
							maxFd = myServer.getNewSockFd();
						}
					}
				}
				else{
					// Invoke the child process to 
					// serve the client	
	  				pid_t pid = fork();
         		
					if (pid < 0)
             					error("ERROR: Failed to Create Child process");
					if (pid == 0){
                				close(myServer.getSockFd());
             	  				serveClientRequest(i,(timeout/MAX_CLIENTS),
							               http_version); 
						close(i);
            					exit(0);
						
					}
					gettimeofday(&EndTime, NULL);
                                        diff = ((EndTime.tv_usec - startTime.tv_usec));
                                        char fileWriteData[40];
                                        sprintf(fileWriteData, "Connection Time: %ld",diff);
                                        fp = fopen("./ConnectionTime.txt", "a");
					fputs(fileWriteData, fp);
                                        fclose(fp);

                                        close(i);
					FD_CLR(i, &activeFds);
					FD_CLR(i, &master);
				}
			}
		}
	
} /* end of for*/
    
return 0;
}
		

void  serveClientRequest(const int& sock, int timeout, const double& http)
{
   int n;
   char buffer[4096];
   char storedBuffer[4096];
   double diff;
   time_t start, end;
   time(&start); 

  if( http == 1.1){
	do{
		n = read(sock, buffer, sizeof(buffer));
		if (n < 0) error("ERROR reading from socket");
    
		if(parseRequest(buffer, http, sock) == 1){
			break;	
		}
		time(&end);
		diff = difftime(end, start);
	}while(diff<timeout);
	
  }else{
	n = read(sock,buffer, sizeof(buffer));
	if (n < 0) error("ERROR reading from socket");

	parseRequest(buffer, http, sock);
  }
}

void error(const char* msg){
	perror(msg);
	close(serverSocket);
	exit(1);
}
int parseRequest(char* request, double http, int socket){

 	char resource[100];
	char *parsedRequest;
	char root[80] = "./";
	char errorType[100] = "400 Bad Request";
	char putFileName[20];
	
	parsedRequest = strtok(request,"\r\n");
	if( (strcmp(parsedRequest, "GET / HTTP/1.0") == 0) && http == 1.1 ){
		strcat(root,"index.html");
                handleGetCommand(root, socket, http, true, false);
		return 1;
        }else if( strcmp(parsedRequest, "GET/") == 0 ||
	    strcmp(parsedRequest, "GET /index.html") == 0 ||
	    strcmp(parsedRequest, "GET / HTTP/1.1") == 0 ||
	    strcmp(parsedRequest, "GET / HTTP/1.0") == 0){
		strcat(root,"index.html");
		handleGetCommand(root, socket, http);
	}else if(sscanf(parsedRequest, "GET /%s HTTP/1.0",resource) == 1 ||
		 sscanf(parsedRequest, "GET /%s HTTP/1.1",resource) == 1){
		strcat(root, resource);
		handleGetCommand(root, socket, http);
	}else if(sscanf(parsedRequest, "HEAD %s",resource) == 1){
		strcat(root, resource);			
		handleGetCommand(root, socket, http, false, true);
	}else if(strcmp(parsedRequest, "HEAD") == 0 ){
		handleGetCommand(NULL, socket, http, false, true);
}else if(sscanf(parsedRequest, "PUT %s %s", putFileName, resource) == 2){ 
		strcat(root, putFileName);
		handlePutResourceCommand(root, resource, socket, http);
	}else if(sscanf(parsedRequest, "DELETE %s", resource) == 1){ 
		strcat(root, resource);
		handleDeleteResourceCommand(root, socket, http);
	}else if(strcmp(parsedRequest, "TRACE") == 0){ 
		handleTraceCommand(socket, request, http);
	}else if(sscanf( parsedRequest, "OPTIONS %s", resource) == 1){ 
	        handleOptionCommand(resource, socket, http);
	}else{
		handleRequestError(errorType, socket, http);	
	}
return 0;
}

// Create the response header for the HTTP RESPONSE Message
void createResponseHeader(char* response, const char* errorCode, double http, long contentLen, const char* contentType, 
				  bool connClose, bool allowCommand){

	char date[40];  // Date command
	char host[40];  // hostname
	FILE* in;

	if(!(in = popen("date","r"))){
	exit(1);
	}
	fgets(date, sizeof(date), in);

	if(!(in = popen("hostname","r"))){
	error("ERROR: Command hostname failed");
	}
	fgets(host, sizeof(host), in);

	if(!contentType && !allowCommand){
		if(http == 1.0){
			sprintf(response,"HTTP/1.0 %s\r\nDate: %s \r\nServer: %s \r\nContent-Length: %ld \r\nConnection:close \r\n\r\n",
                                errorCode, date, host, contentLen);
		}else if(http == 1.1 && connClose){
			sprintf(response,"HTTP/1.1 %s\r\nDate: %s \r\nServer: %s \r\nContent-Length: %ld \r\nConnnection:close \r\n\r\n",
                                errorCode, date, host, contentLen);
		}else{
			sprintf(response,"HTTP/1.1 %s\r\nDate: %s \r\nServer: %s \r\nContent-Length: %ld \r\nConnnection:open \r\n\r\n",
                                errorCode, date, host, contentLen);
		}
	}else if(!contentType && allowCommand){
		if(http == 1.0){
			sprintf(response,"HTTP/1.1 %s\r\nDate: %s \r\nServer: %s \r\nAllow: GET, HEAD \r\n"
				"Content-Length: %ld \r\nConnection:open \r\n",errorCode, date, host, contentLen);
		}else if(http == 1.1 && connClose){
			sprintf(response,"HTTP/1.1 %s\r\nDate: %s \r\nServer: %s \r\nAllow: GET, HEAD, PUT, DELETE,"
				"TRACE, OPTIONS \r\nContent-Length: %ld \r\nConnnection: close \r\n",errorCode, date, host, contentLen);
		}else{
			sprintf(response,"HTTP/1.1 %s\r\nDate: %s \r\nServer: %s \r\nAllow: GET, HEAD, PUT, DELETE,"
				"TRACE, OPTIONS \r\nContent-Length: %ld \r\nConnnection: open \r\n",errorCode, date, host, contentLen);
		}
	}else if(contentType != NULL && !allowCommand){
		if(http == 1.0){
			sprintf(response,"HTTP/1.0 %s\r\nDate: %s \r\nServer: %s \r\nContent-Length: %ld \r\nConnection:close \r\n"
				"Content-Type: %s; charset=UTF-8 \r\n\r\n",errorCode, date, host, contentLen, contentType);
		}else if(http == 1.1 && connClose){
			sprintf(response,"HTTP/1.1 %s\r\nDate: %s \r\nServer: %s \r\nContent-Length: %ld \r\nConnnection: close \r\n"
				"Content-Type: %s; charset=UTF-8 \r\n\r\n",errorCode, date, host, contentLen, contentType);
		}else{
			sprintf(response,"HTTP/1.1 %s\r\nDate: %s \r\nServer: %s \r\nContent-Length: %ld \r\nConnnection: open \r\n"
				"Content-Type: %s; charset=UTF-8 \r\n\r\n",errorCode, date, host, contentLen, contentType);
		}
	}
	pclose(in);
}
		

// Function to handle the GET command
void handleGetCommand(char* absFileName, int socket, double http, bool connClose, bool isHead){
	char errorCode[60];
	char *fileExt;
	bool skipFileExistCheck=false;
	char response[RESPONSE_SIZE];
	//Check if file is available
	bool fileExists = false;
	char *fileData;
        long fileSize;
        size_t totalDataRead;
        FILE* fp = fopen(absFileName, "rb");


	
        if(absFileName!= NULL){
 	// check the directory execute permission for client
	if(checkRootDirectoryPermission()){
		// check if the file exists
		fileExists = isFileAvailable(absFileName);

		if(fileExists){
			// Check the file permission for the client
			struct filePermissions filePerm;

			checkFilePermission(absFileName, filePerm);
			if(filePerm.read){
				totalDataRead = fseek(fp, 0, SEEK_END);
                        	fileSize = ftell(fp);
                        	rewind(fp);
				fclose(fp);
				

				strcpy(errorCode,"200 OK");
				fileExt = strrchr(absFileName,'.');
				fileExt++;

				if( (strcmp(fileExt, "html") == 0) ){
				        createResponseHeader(response, errorCode,
                	                                    http, fileSize, "text/html", connClose);
				}else if( (strcmp(fileExt, "txt") == 0) ){
					createResponseHeader(response, errorCode, 
							     http, fileSize, "text/plain", connClose);
				}else if ( (strcmp(fileExt, "png") == 0) ){
					createResponseHeader(response, errorCode,
                                        	             http, fileSize, "image/png", connClose);
				}else if ( (strcmp(fileExt, "jpeg") == 0) ){
					createResponseHeader(response, errorCode,
							     http, fileSize, "image/jpeg", connClose);
				}else if ( (strcmp(fileExt, "gif") == 0) ){
					createResponseHeader(response, errorCode,
                                	                     http, fileSize, "image/gif", connClose);
				}else if( (strcmp(fileExt, "jpg") == 0) ){
					createResponseHeader(response, errorCode,
                                                             http, fileSize, "image/jpg", connClose);
				}else{
					handleRequestError("415 Unsupported Media Type",
							     socket, http, connClose);
					return;
				}
			}else{
				handleRequestError("401 Unauthorized", socket, http, connClose);	
				return;
			}
		}else{
			handleRequestError("404 Not Found", socket, http, connClose);	
			return;
		}
	}else{
		handleRequestError("401 Unauthorized", socket, http, connClose);
		return;
	}
	}

	if(isHead && !absFileName){
		createResponseHeader(response, "200 OK", http, 0, NULL);
	}
	//After creating response header open file and send the message body also.
	int n = write(socket, response, strlen(response));
	if (n < 0) error("ERROR writing response header in the socket");

	if(!isHead){	
		// Writing the contents of the file into the client socket

		fp = fopen(absFileName, "rb");	
		if(!fp){
			error("ERROR: File cannot be opened");
		}
		else{
			// Allocate memory for fileData
			fileData = (char*)malloc(sizeof(char)*fileSize);

			if(!fileData){
				error("ERROR: In allocating memory to fileData");
			}
			totalDataRead = fread(fileData, 1, fileSize, fp);

			if(totalDataRead != fileSize){
				error("ERROR: Error in reading file data");
			}
			n = write(socket, fileData, totalDataRead);
			if(n<0){
				error("ERROR: In writing file data into the client socket");
			}
		}
		fclose(fp);
		free(fileData);
	}
}	
	
// Function which handles all the error codes related messages
void handleRequestError(const char* errorCode, int socket, double http_version, bool conn){

	char errorResponse[RESPONSE_SIZE];
 	createResponseHeader(errorResponse, errorCode, http_version, 0, NULL, conn);

        //After creating response header open file and send the message body also.
        int n = write(socket, errorResponse, strlen(errorResponse));
        if (n < 0) error("ERROR writing response header in the socket");

}

// Function to check if file is available or not
bool isFileAvailable(char* fileName){

	FILE *fp;
	fp = fopen(fileName, "rb");
	if(!fp){
		return false;
	}
	fclose(fp);
	return true;
}	

// Function to check the file permission 
void checkFilePermission(char* fileName, struct filePermissions& perm){
	
	struct stat fileStat;

	// File is present check for others permission on it.
        stat(fileName, &fileStat);

        if(fileStat.st_mode & S_IROTH){
        	perm.read = true;
        }else if(fileStat.st_mode & S_IWOTH){
                perm.write = true;
        }else if(fileStat.st_mode & S_IXOTH){
                perm.execute = true;
        }
}

//Function to check the directory permission
bool checkRootDirectoryPermission(){
	bool directoryExecutable = false;
	char dirPerm[2];

	FILE *in;
  	if(!(in = popen("ls -l ../ | grep -w Server | cut -c10","r"))){
		error("ERROR: Directory permission command failed");
	}

	fgets(dirPerm, 2, in);
	if(strcmp(dirPerm,"x") == 0){
		return true;
	}
	pclose(in);
	
return false;
}

// Function to handle the Delete command
void handleDeleteResourceCommand(char* root, int socket, double http){

        char delResponse[RESPONSE_SIZE];
        int n;
	//Check if file is available
        bool fileExists = false;

        // check the directory execute permission for client
        if(checkRootDirectoryPermission()){
                // check if the file exists
                fileExists = isFileAvailable(root);

		if(fileExists){
			char command[100] = "rm -rf ";
			strcat(command, root);

			if(system(command) == 1){
			       handleRequestError("500 Internal Server Error", socket, 1.1);
			}
			createResponseHeader(delResponse, "200 OK", http, 0, NULL);
			strcat(delResponse, "\r\n File Successfully Deleted\r\n");

			// Send the response to the Client
			n = write(socket, delResponse, strlen(delResponse));
			if(n<0) error("ERROR: Writing the Delete Response");

		}else{
			handleRequestError("404 Not Found", socket, http);
                        return;
		}
	}else{
		handleRequestError("410 Unauthorized", socket, http);
		return;	
	}
}

//Function to handle the TRACE command
void handleTraceCommand(int socket, char* requestMsg, double http){

	char traceResponse[RESPONSE_SIZE];
        long conLen = strlen(requestMsg);
	strcat(requestMsg, "\n");
	
	// Create the response header
	createResponseHeader(traceResponse, "200 OK", http , conLen, "message/http");
	strcat(traceResponse, requestMsg);

	int n=write(socket, traceResponse, strlen(traceResponse));
	if(n<0) error("ERROR: Write failure for TRACE command");

}

// Function to handle the OPTIONS command
void handleOptionCommand(char* requestedResource, int socket, double http){
	char optionResponse[RESPONSE_SIZE];

	// If request is Option * then send the methods supported by server
	if(strcmp(requestedResource, "*") == 0){
		createResponseHeader(optionResponse, "200 OK", http, 0, NULL, false, true);
	}else if(strcmp(requestedResource, "/HTTP/1.0") == 0){
		createResponseHeader(optionResponse, "200 OK", 1.0, 0, NULL, false, true);	
	}else if(strcmp(requestedResource, "/HTTP/1.1") == 0){
		createResponseHeader(optionResponse, "200 OK",http, 0, NULL, false, true);
	}else{
		handleRequestError("400 Bad Request", socket, http, 0);
		return;
	} 

	// send the response to the client
	int n = write(socket, optionResponse, strlen(optionResponse));
	if(n<0) error("ERROR: Write error for allow command");
}

// Function to handle the PUT command
void handlePutResourceCommand(char* fileName, char* content, int socket, double http){
        char putResponse[RESPONSE_SIZE];
	FILE* fp;
	bool fileExists = false;

        // check the directory execute permission for client
        if(checkRootDirectoryPermission()){
	        // check if the file exists
                fileExists = isFileAvailable(fileName);

                if(fileExists){
                        // Check the file permission for the client
                        struct filePermissions filePerm;

                        checkFilePermission(fileName, filePerm);
                        if(filePerm.write){
				fp = fopen(fileName, "a");
				fputs(content, fp);
				fclose(fp);
	                }else{
				// no write permission on file
	         		handleRequestError("410 Unauthorized", socket, http);
                                return;
			}
		}else{
			// Create a new file and write the data into it
			fp = fopen(fileName, "w");
			fputs(content, fp);
			fclose(fp);
		}
	}else{
		handleRequestError("410 Unauthorized", socket, http);
                return;
	}

	char message[] = "File Content Successfully PUT\r\n";
	int length = strlen(message);
  	// Create response header and send the response to the client
	createResponseHeader(putResponse, "200 OK",http, length, NULL, false, false);		
    
         strcat(putResponse, message);
	// send the response to client
	int n = write(socket, putResponse, strlen(putResponse));
	if(n<0) error("ERROR: Write error for allow command");
}


