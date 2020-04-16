/**********************************************************************************
** Program name: CS 372 Project 2 - simple file transfer system - server 
** Author:       Chen Zou
** Date:         6th August 2019
** Description:  This project requires creating a simple file transfer system that                one pair of users by creating two programs : a chat server and a chat
**               transfers file between client and server.This program is to create 
**				 server that receives command on connection,and respond to client
**			     by either transfer file or list directory according to the command.
** Note: I reused a lot of code from CS344 assignment, which is heavily based on
** the code provided by CS344 professor.
***********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/stat.h>
#include <dirent.h>

#define SIZE 100000

/**********************************************************************************
** Name:		  StartUp
** Description:	  Function to initialize connection between client and server
** Precondition:  Server name and portnumber provided 
** Postcondition: Socket setup and arguments passed by reference updated
***********************************************************************************/
void startUp(int* listenSocketFD, int portNumber) {
	struct sockaddr_in serverAddress;
	
	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress));
	// Create a network-capable socket
	serverAddress.sin_family = AF_INET;
	// Store the port number
	serverAddress.sin_port = htons(portNumber);
	// Any address is allowed for connection to this process
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	
	// Set up the socket
	*listenSocketFD = socket(AF_INET, SOCK_STREAM, 0);
	if (*listenSocketFD < 0) {
		perror("ERROR opening socket");
		exit(1);
	}

	// Enable the socket to begin listening
	if (bind(*listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0){ // Connect socket to port
		printf("ERROR binding on %d!\n", portNumber);
		exit(1);
	}
	// Flip the socket on - it can now receive up to 5 connections
	if (listen(*listenSocketFD, 5) < 0) {
		perror("ERROR listening!");
		exit(1);
	}

	printf("Server open on %d...\n", portNumber);
}
	

/**********************************************************************************
** Name:		  InitialContact
** Description:	  Function to initialize connection between client and server
** Precondition:  Server name and portnumber provided 
** Postcondition: Socket setup and arguments passed by reference updated
***********************************************************************************/
int initiateContact(int* socketFD, char* hostName, int portNumber) {
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;

	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress));
	// Create a network-capable socket
	serverAddress.sin_family = AF_INET;
	// Store the port number
	serverAddress.sin_port = htons(portNumber);
	// Convert the machine name into a special form of address
	serverHostInfo = gethostbyname(hostName);
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }

	// Copy in the address
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length);

	// Set up the socket
	*socketFD = socket(AF_INET, SOCK_STREAM, 0);
	if (*socketFD < 0) {
		perror("CLIENT: ERROR opening socket");
		exit(1);
	}
	// Connect to server, connect socket to address
	if (connect(*socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
		perror("CLIENT: ERROR connecting to server!");
		exit(1);
	}

	printf("\nData connection from client.\n");
}


/**********************************************************************************
** Name:		  receiveCommand
** Description:	  receives and validate command form the client
** Precondition:  connection created, message send from client
** Postcondition: valid command accepted for processing, Error sent for invalid commands
***********************************************************************************/
void getCommand(char* command, char* fileName, int* dataPort, int socketFD) {
	int charsRead, charsWritten;
	// Get message
	char message[32];
	memset(message, '\0', sizeof(message)); // Clear out the buffer again for reuse

	charsRead = recv(socketFD, message, sizeof(message) - 1, 0); // Read data from the socket, leaving \0 at end
	if (charsRead < 0) {
		perror("CLIENT: ERROR reading from socket");
		exit(1);
	}
	// break command list
	char* token = strtok(message, " ");
	// if command is list dir, save command and data port
	if (strcmp(token, "-l") == 0) {
		strcpy(command, token);
		token = strtok(NULL, " ");
		*dataPort = atoi(token);			
	}
	// if command is to get file, save command, filename and data port
	else if (strcmp(token, "-g") == 0) {
		strcpy(command, token);
		token = strtok(NULL, " ");
		strcpy(fileName, token);
		token = strtok(NULL, " ");
		*dataPort = atoi(token);
	}
	// other commmands respond to client with error message
	else{
		char errorMessage[] = "ERROR, please send a valid command!\n";
		//printf("%s\n", errorMessage);
		charsWritten = send(socketFD, errorMessage, sizeof(errorMessage) - 1, 0);
		//printf("%d", charsWritten);
		if (charsWritten < 0) {
			perror("CLIENT: ERROR writing to socket");
			exit(1);
		}
		exit(1);
	}
}


/**********************************************************************************
** Name:		  handleRequest
** Description:	  receives and validate command form the client
** Precondition:  connection created
** Postcondition: valid command accepted for processing, Error sent for invalid commands
***********************************************************************************/
int handleRequest(int socketFD, int portNumber, char* clientName) {
	char command[3];
	memset(command, '\0', sizeof(command));	// Clear out for reuse
	char fileName[255];
	memset(fileName, '\0', sizeof(fileName));

	int dataPort;
	//get command list and allocate to variables
	getCommand(command, fileName, &dataPort, socketFD);
	//printf("command is: %s\n", command);

	int charsWritten = 0;
	char buffer[SIZE]; // buffer for sending message
	memset(buffer, '\0', sizeof(buffer));
	//get file structure http://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program
	DIR* d;
	struct dirent *dir;
	d = opendir(".");
	if (d) {
		while ((dir = readdir(d)) != NULL) {
			// Skip hidden files as per 344 homework
			if (dir->d_name[0] != '.') {
				strcat(buffer, dir->d_name);
				strcat(buffer, "\n");
			}
		}
	}

	// if command is "-l", return the contents of the current directory.
	if (strcmp(command, "-l") == 0) {
		// start data socket
		int dataSocketFD, dataConnectionFD;				
		//set up the data connection, sleep to wait for the socket ready
		sleep(2);
		initiateContact(&dataConnectionFD, clientName, dataPort);
		
		printf("List directory requested on port %d.\n", dataPort);

		// send the directory to client on data connection
		printf("Sending directory contents to client on port %d\n", dataPort);
		charsWritten = send(dataConnectionFD, buffer, sizeof(buffer) - 1, 0);
		if (charsWritten < 0) {
			perror("CLIENT: ERROR writing to socket");
			exit(1);
		}
		close(dataConnectionFD);
		dataConnectionFD = -1;
		wait(NULL);
		exit(0);
	}

	// if command is "-g", valid filename and send the file contents
	else if (strcmp(command, "-g") == 0) {
		// filename not found
		if (strstr(buffer, fileName) == NULL) {
			printf("\nFile \"%s\" requested on port %d.\n", fileName, dataPort);
			printf("File not found. Sending error message to client on port: %d\n", portNumber);
			char validationMessage[] = "FILE NOT FOUND!";
			// send the error message to client on original connection
			charsWritten = send(socketFD, validationMessage, sizeof(validationMessage) - 1, 0);
			if (charsWritten < 0) {
				perror("CLIENT: ERROR writing to socket");
				exit(1);
			}
			exit(1);
		}
		else {
			//file name found, send validatin message
			char validationMessage[] = "FILE FOUND!";
			charsWritten = send(socketFD, validationMessage, sizeof(validationMessage) - 1, 0);
			if (charsWritten < 0) {
				perror("CLIENT: ERROR writing to socket");
				exit(1);
			}
			
			//set up the data connection, sleep to wait for the socket ready
			int dataSocketFD, dataConnectionFD; 
			sleep(2);
			initiateContact(&dataConnectionFD, clientName, dataPort);

			// transfering file content
			printf("File \"%s\" requested on port %d.\n", fileName, dataPort);
			printf("Sending \"%s\" to client on port %d\n", fileName, dataPort);

			// get contents from files, from CS344 homework
			FILE* fp = fopen(fileName, "r");
			memset(buffer, '\0', sizeof(buffer));

			// send file contents to client on data connection, looping until EOF
			// code reference: https://stackoverflow.com/questions/38976582/reading-text-file-until-eof-using-fgets-in-c?rq=1
			charsWritten = 0;
			int newCharsWritten;
			while (fgets(buffer, sizeof(buffer), fp)) {
				newCharsWritten = send(dataConnectionFD, buffer, strlen(buffer), 0);
				if (newCharsWritten < 0) perror("CLIENT: ERROR writing to socket");
				if (newCharsWritten < strlen(buffer)) printf("CLIENT: WARNING: Not all data written to socket!\n");
				charsWritten += newCharsWritten;
			}
			// add @@ to the message indicating the end of message, idea provided by CS344 TA
			charsWritten += send(dataConnectionFD, "@@", 2, 0);
			//printf("char sent: %d\n", charsWritten);
			fclose(fp);

			close(dataConnectionFD);
			dataConnectionFD = -1;
			wait(NULL);
			//printf("data socket closed. waiting for new request...\n");
			exit(0);
		}
	}
}


// main function
int main(int argc, char* argv[]) {
	int listenSocketFD, establishedConnectionFD, portNumber;

	// check if arguments number is correct
	if (argc != 2) {
		fprintf(stderr, "Incorrect number of arguments! please enter port number!");
		exit(1);
	}

	// Get the port number from argument, convert to an integer from a string
	portNumber = atoi(argv[1]);

	//start up the server
	startUp(&listenSocketFD, portNumber);
	struct sockaddr_in clientAddress;
	socklen_t sizeOfClientInfo;

	pid_t spawnpid;
	while (1) {
		// Accept a connection, blocking if one is not available until one connects
		sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		//establishedConnectionFD = accept(listenSocketFD, NULL, NULL);
		if (establishedConnectionFD < 0) perror("ERROR on accept");
		
		//get client IP address, source: https://beej.us/guide/bgnet/html/multi/getpeernameman.html
		socklen_t len;
		struct sockaddr_storage addr;
		char ipstr[INET6_ADDRSTRLEN];
		char host[1024];
		char service[20];
		int port;

		len = sizeof addr;
		getpeername(establishedConnectionFD, (struct sockaddr*)&addr, &len);
		struct sockaddr_in *s = (struct sockaddr_in *)&addr;
		port = ntohs(s->sin_port);
		inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);

		//printf("Peer IP address: %s\n", ipstr);

		// call fork process
		spawnpid = fork();
		switch (spawnpid) {
			// show error if fork failed
			case -1:
				perror("ERROR creating fork");
				exit(0);
				break;
			// Fork successful, handle connections
			case 0:;
				// handle request from client
				handleRequest(establishedConnectionFD, portNumber, ipstr);
				break;

			//default:;
		}

		// wait for zombie programs and kill them
		close(establishedConnectionFD);
		establishedConnectionFD = -1;
		wait(NULL);
	}

	return 0;	
}

