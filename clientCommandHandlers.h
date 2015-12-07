#ifndef CLIENTCOMMANDHANDLERS_H
#define CLIENTCOMMANDHANDLERS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

struct filePermissions{
	bool read;
	bool write;
	bool execute;
};

void serveClientRequest(const int& sock, int timeout, const double& http);

void error(const char* msg);

int parseRequest(char* request, double http, int socket);

void createResponseHeader(char* response, const char* errorCode, double http, long contentLen,
			   const char* contentType, bool connClose = false, bool allowCommand = false);

void handleGetCommand(char* absFileName, int socket, double http, 
                      bool connClose = false, bool isHead = false);

void handleRequestError(const char* errorCode, int socket, double http_version,  bool conn = false);

bool isFileAvailable(char* fileName);

void checkFilePermission(char* fileName, struct filePermissions& perm);

bool checkRootDirectoryPermission();

void handlePutResourceCommand(char* fileName, char* content, int socket, double http);

void handleDeleteResourceCommand(char* root, int socket, double http);

void handleTraceCommand(int socket, char* requestMsg, double http);

void handleOptionCommand(char* requestedResource, int socket, double http);

#endif
