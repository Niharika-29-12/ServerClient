/*
	Final Project Submission: Team Details
	Section: 4
	Subject: Advanced System Programming
	
	Team Members:
	-------------------------------
	|      NAME       | Student ID |
	|-----------------|------------|
	| Manjinder Singh | 110097177  |
	| Harbhajan Singh | 110100089  |
	--------------------------------
	
	The below submitted code presents the final submission for the Advanced System Programming course.
	It showcases our understanding of the ASP curriculum and demonstrates our teamwork in coding for the development of Client Server Interaction Logic.

*/

/*
	References :
	1. ASP Class Socket Lecture Notes.
	2. ASP Class Code Snippets.
	3. Socket Documentation - https://www.man7.org/linux/man-pages/man2/socket.2.html
*/

/*
	Details about final project:-
	Assignment Topic - Client/Server Communication using Socket Programming
	Language - C
	Coding Style - Google
	Indent Width - 1
	Column Width -120
	Date of Submission - August 15, 2023
*/

// Including the required set of header files which is essential for the execution of the program(mirror program file).
#include <sys/wait.h>
#include <netinet/in.h> 
#include <sys/stat.h>
#include <netdb.h>
#include <string.h>
#include <fnmatch.h>
#include <ftw.h> 
#include <errno.h>
#include <dirent.h>
#include <stdarg.h>
#include <arpa/inet.h>
#include <sys/signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <time.h>

// Declaring some global macros/constants that will be used throughout the execution of the program(mirror program file).
// Declared global macro/cosntant for debug mode.
#define STATUS_OF_DEBUG_MODE 0

// Declared some global macros/constants for maximum limits.
#define MAXIMUM_ALLOWED_RESPONSE_SIZE 1024
#define MAXIMUM_BUFFER_SIZE_ALLOWED 1024
#define MAXIMUM_ARGUMENTS_ALLOWED 7
#define MAXIMUM_EXTENSION_LIMIT 6

// Declared some global macros/constants for response type for - Structure, file, text.
#define RESP_TYPE_STRUCT 2
#define RESP_TYPE_FILE 3
#define RESP_TYPE_TEXT 1

// Declared global macro/constant for port number reference.
#define NUM_OF_MIRROR_PORT 45678

// Declared some global macros/constants for defining file names and referring to Home directory.
#define TEMPORARY_FILE_LIST "temporary_file_list.txt"
#define TEMPORARY_TAR_ARCH "temp.tar.gz"
#define USER_HOME_DIRECTORY getenv("HOME")

// Defining DEBUG_LOG macro based on debug mode status.
#if STATUS_OF_DEBUG_MODE == 1 // Setting enabled mode for debugging.
	#define DEBUG_LOG(fmt, ...) msPrintStatements("[LOG] " fmt, ##__VA_ARGS__)
	 void msPrintStatements(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
    }
#else // In case debugging mode is disabled.
    #define DEBUG_LOG(fmt, ...) ((void)0) 
#endif

// Defined 'msAddressInfo' of type struct to bundle together - IP & Port no. for transfer.
typedef struct {
    char msIPAddr[INET_ADDRSTRLEN];
    int msPortNum;
} msAddressInformation; 

// "msClientNum" declared globally for checking Client Connection Count.
int msClientNum = 0;

// Function Declaration that will be handling various commands from client.
void msClientProcess(int msFileDesOfClientSock);

// Declared functions to execute commands.
void filesrch(int msFileDesOfClientSock, char** msArgs);
int getdirf(int msFileDesOfClientSock, char** msArgs);
int tarfgetz(int msFileDesOfClientSock, char** msArgs);
int ffgets(int msFileDesOfClientSock, char** msArgs, int msArgsLen);
int targzf(int msFileDesOfClientSock, char** msArgs, int msExtensionCount);

// Declared function to perform data transfer to client.
int msResponseForSendingFile(int msFileDesOfClientSock, const char* msFileName);
int msResponseForSendingText( int msFileDesOfClientSock, char* msBuffer);

// Declared functions for file tree searching in a recursive manner.
int msRecSearchExtension(char *msDirectoryName, char **msFileTypes, int msExtensionCount, int *msFileTotalCount);
int msRecSearchDate(char *msRootPath, time_t msBeginDate, time_t msEndDate, int *msFileTotalCount);
int msRecSearchName(char *msDirectoryName, char **msFileNames, int msFileNameCount, int *msFileTotalCount);
int msRecSearchSize(char *msDirectoryName, int msLowestSize, int msHighestSize, int *msFileTotalCount);

// Declared function for the conversion to unix time from I/P time.
time_t msConvertTimeToUnixFromDate(const char *msTimeStr, int msDateType);

// Declared function for the removal of line break(s) from the buffer completely.
void msRemoveLineBreakFromBuff(char msBuffer[]);

/* 
Function Name - main()

Function Purpose - Logic execution of the main function for Cient-Mirror-Server Connection(in the mirror program file)..

1.  Initialize a server socket and enters into an infinite loop.

2. Assigning client number -> determine connection type(either server or mirror) -> fork a child process to manage client requests of redirection based on connection type.

3. This function enacts as primary hub of the program for accepting clients, load balancing, and routing them to various handling processes based on request type.
*/
int main(int msArgsCount, char *msArgsValue[])
{
	int msServerSockFileDesc, msClientSockFileDesc, msPortNumber, msStatusOfChildProcess;
	socklen_t msSockLen;
	struct sockaddr_in msServerAddress;	// For IPv4 Family
	
	// Handling error in case there is an issue while creating socket.
	if ((msServerSockFileDesc = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("Issue Encounterd: error during the creation of socket.\n");
		exit(1);
	}

	msServerAddress.sin_family = AF_INET;
	msServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);	
	msServerAddress.sin_port = htons(NUM_OF_MIRROR_PORT);

	bind(msServerSockFileDesc, (struct sockaddr *) &msServerAddress, sizeof(msServerAddress));
	listen(msServerSockFileDesc, 5);
	printf("IP Address binded to this port number --> %d.Listening...\n", NUM_OF_MIRROR_PORT);
	while (1)
	{	
		// Increasing the value of client number by 1.
		msClientNum++;
				
		// Connecting to the Mirror Server.
			msClientSockFileDesc = accept(msServerSockFileDesc, (struct sockaddr *) NULL, NULL);
			printf("Client at %d is connected successfully.\n", msClientNum);
			
			// Now, Forking of a child process for handling requests from client.
			if (!fork()){	 
				msClientProcess(msClientSockFileDesc);
				close(msClientSockFileDesc);
				exit(0);
			}
			waitpid(0, &msStatusOfChildProcess, WNOHANG);
		
		
		
	}
}	

/*
Function Name - msClientProcess

Function Purpose - 1. Handles the communication and processing of various commands from a client connection over a socket.

2.  Received Command from client -> Parsing -> Perform specific action based on command type as per point num 3.

3.  It handles commands:-
In case the command is - 'filesrch' - it calls the filesrch function with appropriate args.
In case the command is - 'tarfgetz' it calls the tarfgetz function with appropriate args.
In case the command is - 'getdirf' - it calls the getdirf function with appropriate args.
In case the command is - 'fgets' - it calls the ffgets function with appropriate args.
In case the command is - 'targzf' - it calls the targzf function with appropriate args.
In case the command is - 'quit' - it sends a response to the client, then closes the socket connection, and Finally exits the loop.

4. Read Command -> Extract Args -> Calls relevant functions -> Process command.

5. Lastly, if no command matches with the above list, it will display appropriate message. 
*/
void msClientProcess(int msFileDesOfClientSock)
{
	char msCmdBuffer[MAXIMUM_BUFFER_SIZE_ALLOWED];
	char msRespText[MAXIMUM_BUFFER_SIZE_ALLOWED];
	int msNumOfBytesRead = 0;
	
	// Utilizing the function "snprintf()" for formatting the message.
	snprintf(msRespText, sizeof(msRespText), "Now, you can send commands.");
	send(msFileDesOfClientSock, msRespText, strlen(msRespText), 0);

	while (1)
	{	
		
		// Firstly, the command buffer is fully cleared.
		memset(msCmdBuffer, 0, sizeof(msCmdBuffer)); 
		
		msNumOfBytesRead = read(msFileDesOfClientSock, msCmdBuffer, MAXIMUM_BUFFER_SIZE_ALLOWED);
		msRemoveLineBreakFromBuff(msCmdBuffer);
		
		// Checking in case there is no data received i.e. client has closed connection.
		if (msNumOfBytesRead <= 0) { 
           printf("%d <-- This client is disconnected at the moment.\n", msClientNum);
           break; 
		}
		
		DEBUG_LOG("I/P received from the client--> %s.\n", msCmdBuffer);
		
		char *msArgs[MAXIMUM_ARGUMENTS_ALLOWED];
		int msNumberOfArguments = 0;
		
		// After receiving the command from client, parsing will take place using strtok.
		char* msToken = strtok(msCmdBuffer, " "); 
		char* msCmd = msToken;
		
		while (msToken != NULL) {
			msToken = strtok(NULL, " "); // Fetching the next available token.
			if (msToken != NULL) { // Making sure if the token is not NULL before storing.
				msArgs[msNumberOfArguments++] = msToken; // Saving token in "msArgs" array.
			}
		}
		msArgs[msNumberOfArguments] = NULL; // Adding NULL	to the last index of array.
		
		DEBUG_LOG("Command after parsing--> %s.\n",msCmd);
		
		// Processing based on the command type and calling appropriate function to generate response.
		if (strcmp(msCmd, "filesrch") == 0)
		{
			// Calling "filesrch" function to return details of file.
			filesrch(msFileDesOfClientSock, msArgs);
		}
		else if (strcmp(msCmd, "tarfgetz") == 0)
		{
			// Calling "tarfgetz" function to get file(s) within specified size limit.
			int msFetchedFilesResult = tarfgetz(msFileDesOfClientSock, msArgs);
			
			// Checking the value returned by the "tarfgetz" function call.
               if (msFetchedFilesResult == 1) {
					msResponseForSendingText(msFileDesOfClientSock, "Issue Encountered with the tarfgetz function.");
					printf("Issue Encountered with the tarfgetz function.\n");
				}
		}
		else if (strcmp(msCmd, "getdirf") == 0)
           {
			   // Calling "getdirf" function to get file(s) within specified date limit.
               int msFetchedFilesResult = getdirf(msFileDesOfClientSock, msArgs);
			   
			 // Checking the value returned by the "getdirf" function call.
             if (msFetchedFilesResult == 1) {
					msResponseForSendingText(msFileDesOfClientSock, "Issue Encountered with the getdirf function.");
					printf("Issue Encountered with the getdirf function.\n");
				}
		}
		else if (strcmp(msCmd, "fgets") == 0)
		{	
			 // Calling "ffgets" function to get file(s) based on the names of the files supplied in the command.
             int msFetchedFilesResult = ffgets(msFileDesOfClientSock, msArgs, msNumberOfArguments);
			
			// Checking the value returned by the "ffgets" function call.
			if (msFetchedFilesResult == 1) {
				msResponseForSendingText(msFileDesOfClientSock, "Issue Encountered with the ffgets function.");
				printf("Issue Encountered with the ffgets function.\n");
			}
		}
		else if (strcmp(msCmd, "targzf") == 0)
		{
   			// Calling "targzf" function to get file(s) based on the specific extensions supplied in the command.
			int msFetchedFilesResult = targzf(msFileDesOfClientSock, msArgs, msNumberOfArguments);

			// Checking the value returned by the "targzf" function call.
			if (msFetchedFilesResult == 1) {
				msResponseForSendingText(msFileDesOfClientSock, "Issue Encountered with the targzf function.");
				printf("Issue Encountered with the targzf function.\n");
			}

		}

		else if (strcmp(msCmd, "quit") == 0)
		{		
			printf("Request from client is to QUIT.Quitting now...\n");
	
			// The quit request from client will be acknowledged and then socket will be closed.
			msResponseForSendingText(msFileDesOfClientSock, "The close connection request will be acknowledged by the server.");
            close(msFileDesOfClientSock); 
			printf("Socket of client is now closed.\n");
            break;
            exit(0); 
		}
		
		else
		{	
			// In case of invalid command is supplied, client will be informed.
			msResponseForSendingText(msFileDesOfClientSock, "The command is not valid. Please try with the valid commands.\n"); ///????????add valid command names
			continue;	// It will continue with the commands input from the user until Ctrl+C is pressed.
		}
	}
}

/*
Function Name - msResponseForSendingText

Function Purpose - 1. This function sends a text response to a client via a socket connection.

2. Sends a response along with data(text format) -> Sends actual response text to the client.

3. The main purpose is to handle the process of sending textual information to client connection and acts as a messager for communication between server and the connected clients.
*/
int msResponseForSendingText(int msFileDesOfClientSock, char* msRespText){
	
	// Text Response will be sent.
	long msRespType = RESP_TYPE_TEXT;
	send(msFileDesOfClientSock, &msRespType, sizeof(msRespType), 0);
	
	DEBUG_LOG("Response(Text format) sending to the client %d - %s\n", msClientNum, msRespText);
	// Response of text format sending to Client.
	send(msFileDesOfClientSock, msRespText, strlen(msRespText), 0);
	
	return 0;
}

/*
Function Name - msResponseForSendingFile

Function Purpose - 1. This function mainly sends the content of specific file(s) to a connected client(s) via a socket connection.

2. It starts with Opening a file -> Reads its content in chunks/parts -> Send each of those to the client.

3. Once the entire content is transferred -> File is closed -> Return '0' to send signal for successful completion. 
*/
int msResponseForSendingFile(int msFileSocket, const char* msFileName) {
    // The file is opened first that is to be transferred.
    int msFileDesc = open(msFileName, O_RDONLY);
    if (msFileDesc < 0) {
        perror("Issue Encountered: error with the opening operation of file is failed.");
        exit(EXIT_FAILURE);
    }

    char msBuffer[MAXIMUM_ALLOWED_RESPONSE_SIZE];
    ssize_t msReadVal;

    // First, reading the contents of the file and then sending it to client.
    while ((msReadVal = read(msFileDesc, msBuffer, 1024)) > 0) {
        send(msFileSocket, msBuffer, msReadVal, 0);
    }

    // In the end, closing the file.
    close(msFileDesc);

    return 0;
}
	
/*
Function Name - filesrch

Function Purpose - 1. This function will perform a search for a particular file in the home directory path by utilizing the 'find' command.

2. It will construct command then captures its O/P through a pipe and extract details like size and modification time using  'stat' function. 

3. Finally, it generates a response message that will be send to the client.

Basically, it serves as a purpose that allows various clients to request for file information from the server's filesystem.
*/
void filesrch(int msFileDesOfClientSock, char** msArgs){
	
	// Fetching first argument that is file name( in the variable - "msFileName") 
	char* msFileName = msArgs[0];
	
	DEBUG_LOG("File search operation is in process...\n");
	char msResponse[MAXIMUM_ALLOWED_RESPONSE_SIZE];
	DEBUG_LOG("Name of the File --> %s\n", msFileName);
					
    char* msHomeDir = USER_HOME_DIRECTORY; // Path of home directory is obtained.
    char* msCmd = (char*) malloc(strlen(msHomeDir) + strlen(msFileName) + 27); // Memory allocation for the command of type string.
    sprintf(msCmd, "find %s -name '%s' -print -quit", msHomeDir, msFileName); // Construction of the command(find).
	DEBUG_LOG("Command execution is in progress... %s\n", msCmd);
    FILE* msPipe = popen(msCmd, "r"); // Pipe opening for the command.
    if (msPipe != NULL) { // Piping Operation.
        char msReadLine[256];
        if (fgets(msReadLine, sizeof(msReadLine), msPipe) != NULL) { // Getting the first line of the pipe output to read.
            msReadLine[strcspn(msReadLine, "\n")] = '\0'; // Newline character will be removed from the line end.
            struct stat msFileInfo;
            if (stat(msReadLine, &msFileInfo) == 0) { // Utilizing stat() function to obtain file information.
				time_t msFileCreationTime;
				#ifdef __APPLE__
					msFileCreationTime = msFileInfo.st_birthtime;
				#else
					msFileCreationTime = msFileInfo.st_mtime;
				#endif
				char* msTime = ctime(&msFileCreationTime);
				msRemoveLineBreakFromBuff(msTime);
                sprintf(msResponse, "%s (%lld bytes were created %s)", msReadLine, (long long) msFileInfo.st_size, msTime); // Displaying File Information on console.
            } else {
                sprintf(msResponse,"Fails to get file information for %s", msReadLine);
            }
        } else {
            sprintf(msResponse,"The file cannot be located.");
        }
        pclose(msPipe); // Closing pipe in the end.
    } else {
        DEBUG_LOG("Issue Encountered:  pipe opening operation to command is failed.\n");
    }
    free(msCmd); // Release the memory allocated specifically for the command.
	
	// Sending response from server -> client
	msResponseForSendingText(msFileDesOfClientSock, msResponse);
			
}

/*
Function Name - msRecSearchExtension

Function Purpose - 1. This function performs a recursive search within a directory and its sub-directories.

2. It will match file(s) based on provided extensions in the command arguments -> Store their paths in a temporary_file_list -> Keeps track of the count of total matching files.

Basically, the function serves as a purpose to perform effective search operation and then categorize the file(s) by extension(s) type in a directory structure.
*/
int msRecSearchExtension(char *msDirectoryName, char **msFileTypes, int msExtensionCount, int *msFileTotalCount) {
    DIR *msDir;
    struct dirent *msDirEntry;
    char msBuffer[MAXIMUM_BUFFER_SIZE_ALLOWED];
    int msStatus;
    int msExtensionCurrentIndex;
    FILE *msFilePtr;

    DEBUG_LOG("Initializing the Recursive Search for Extensions...\n");

    if ((msDir = opendir(msDirectoryName)) == NULL) {
        perror("Issue Encountered: Opening of directory operation is failed.");
        return 1;
    }

    msFilePtr = fopen(TEMPORARY_FILE_LIST, "a");
    if (msFilePtr == NULL) {
        fprintf(stderr, "Issue Encountered: Unable to open file.\n");
        return 1;
    }

    while ((msDirEntry = readdir(msDir)) != NULL) {

        if (msDirEntry->d_type == DT_DIR && strcmp(msDirEntry->d_name, ".") != 0 && strcmp(msDirEntry->d_name, "..") != 0) {
            if (msDirectoryName[strlen(msDirectoryName) - 1] == '/') {
                sprintf(msBuffer, "%s%s", msDirectoryName, msDirEntry->d_name);
            } else {
                sprintf(msBuffer, "%s/%s", msDirectoryName, msDirEntry->d_name);
            }

            msRecSearchExtension(msBuffer, msFileTypes, msExtensionCount, msFileTotalCount);
        } else {
            if(strcmp(msDirEntry->d_name, ".") == 0 || strcmp(msDirEntry->d_name, "..") == 0)
                continue;
            DEBUG_LOG("Directory read-> %s\n", msDirEntry->d_name);
            for (msExtensionCurrentIndex = 0; msExtensionCurrentIndex < msExtensionCount; msExtensionCurrentIndex++) {
                DEBUG_LOG("Extension that is matching - [%s] with [%s]\n", msFileTypes[msExtensionCurrentIndex], msDirEntry->d_name);
                if (fnmatch(msFileTypes[msExtensionCurrentIndex], msDirEntry->d_name, FNM_PATHNAME) == 0) {
                    if (msDirectoryName[strlen(msDirectoryName) - 1] == '/') {
                        sprintf(msBuffer, "%s%s", msDirectoryName, msDirEntry->d_name);
                    } else {
                        sprintf(msBuffer, "%s/%s", msDirectoryName, msDirEntry->d_name);
                    }
                    fprintf(msFilePtr, "%s\n", msBuffer);
					DEBUG_LOG("File(s) stored to %s: %s \n", TEMPORARY_FILE_LIST, msBuffer);
                    *msFileTotalCount+=1;
                    break;
                }
            }
        }
    }
    closedir(msDir);
    fclose(msFilePtr);

    return 0;
}

/*
Function Name - targzf

Function Purpose - 1. This function searches for file(s) with the provided extensions in the command arguments in the home directory of user.

2. Then it creates a compressed archive( of Tar format) of these file(s) -> Then it is sent to client -> Manages temporary file(s).

Its main purpose is to ensure efficient packaging and transfer of various fil(s) based on client(s) connection request made to the server.
*/
int targzf(int msFileDesOfClientSock, char **msExtensions, int msExtensionCount) {
    DIR *msDir;
    struct dirent *msDirEntry;
    char *msHomeDir = USER_HOME_DIRECTORY;
    char msBuffer[MAXIMUM_BUFFER_SIZE_ALLOWED];
    char *msTarCmdFormat = "tar -czf %s -T %s"; // Command format for Tar.
    int msStatus, msExtensionCurrentIndex;
	char *msFileTypes[msExtensionCount];
	
    DEBUG_LOG("Path of Home directory is -> %s\n", msHomeDir);
    DEBUG_LOG("%d <-- List of the Extensions which are parsed.", msExtensionCount);

    for (msExtensionCurrentIndex = 0; msExtensionCurrentIndex < msExtensionCount; msExtensionCurrentIndex++) {
        DEBUG_LOG(" [%s]", msExtensions[msExtensionCurrentIndex]);
    }
	DEBUG_LOG("\n......................\n");
	
    if ((msDir = opendir(msHomeDir)) == NULL) {
        DEBUG_LOG("Issue Encountered: Opening of directory is failed.");
        return 1;
    }
    for (msExtensionCurrentIndex = 0; msExtensionCurrentIndex < msExtensionCount; msExtensionCurrentIndex++) {
        msFileTypes[msExtensionCurrentIndex] = malloc(strlen(msExtensions[msExtensionCurrentIndex]) + 2);
        sprintf(msFileTypes[msExtensionCurrentIndex], "*.%s", msExtensions[msExtensionCurrentIndex]);
    }
    int msFileTotalCount = 0;
    if (msRecSearchExtension(msHomeDir, msFileTypes, msExtensionCount, &msFileTotalCount) != 0) {
		DEBUG_LOG("Issue Encountered: Error in locating the file(s).");
        return 1;
    }
	
	DEBUG_LOG("Total count of Files found -> %d\n", msFileTotalCount);
	
    if (msFileTotalCount == 0) {
		
		// In case, no file found is found.
		DEBUG_LOG("No file is found wrt your search.\n");
		msResponseForSendingText(msFileDesOfClientSock, "No file is found wrt your search.");
        return 0;
    }
    sprintf(msBuffer, msTarCmdFormat, TEMPORARY_TAR_ARCH, TEMPORARY_FILE_LIST);
    DEBUG_LOG("Command execution is on progress -> %s\n", msBuffer);
    msStatus = system(msBuffer);
    if (msStatus != 0) {
        DEBUG_LOG("Issue Encountered: Error while creating the tar(zip) file.\n");
        return 1;
    }
    // Fetching the size of the tar(zip) file.
    FILE *msFilePtr;
    long int msFileSize;
    msFilePtr = fopen(TEMPORARY_TAR_ARCH, "rb");
    if (msFilePtr == NULL) {
        DEBUG_LOG("Issue Encountered: Error while opening the file.\n");
        return 1;
    }
    fseek(msFilePtr, 0, SEEK_END);
    msFileSize = ftell(msFilePtr);
    fseek(msFilePtr, 0, SEEK_SET);
	
    // File size is sent using "send" command.
    send(msFileDesOfClientSock, &msFileSize, sizeof(msFileSize), 0);
	
    // File Transfer Operation.
    if (msResponseForSendingFile(msFileDesOfClientSock, TEMPORARY_TAR_ARCH) != 0) {
        DEBUG_LOG("Issue Encountered: Error while transferring the file.\n");
		return 1;
    }
    else {
        DEBUG_LOG("The operation of File transfer is completed.\n");
    }
    fclose(msFilePtr);
	
	// Removing the file - "temporary_file_list.txt".
    msStatus = remove(TEMPORARY_FILE_LIST);
    if (msStatus != 0) {
        DEBUG_LOG("Issue Encountered: Error while deleting the file - temporary_file_list.txt\n");
    }
    
    // Removing the file - "temp.tar.gz".
    msStatus = remove(TEMPORARY_TAR_ARCH);
    if (msStatus != 0) {
        DEBUG_LOG("Issue Encountered: Error while deleting the file - temp.tar.gz\n");
    }
	
    return 0;
}

/*
Function Name - msRecSearchName

Function Purpose - 1. This function will be recursively searches for supplied set of file names(s) within a provided directory and its associated sub directories.

2. It will be reading the directory entries and performing a check if each and every entry is a sub directory or a file.

3. In case it is a 'sub directory' , then the fucntion will be recursively seraches within that directory until everything is checked in its sub directories.

4. In case it is a 'file' and also matched with the one of the specified filename(s), its path will be saved to a temporary file list.

5. Then the matched files list will be used in creating the archive of those set.

6. After the search operation through all the entries in the directory, the function will close the directory and the temporary file list.

7. In the end, the total number of matched file(s) is updated and returned finally.
*/
int msRecSearchName(char *msDirectoryName, char **msFileNames, int msFileCount, int *msFileTotalCount) {
    DIR *msDir;
    struct dirent *msDirEntry;
    char msBuffer[MAXIMUM_BUFFER_SIZE_ALLOWED];
    int msStatus;
    int msCurrIndex;
	FILE *msFilePtr;
	
	DEBUG_LOG("Initializing the Recursive Search based on File Names...\n");
	
    if ((msDir = opendir(msDirectoryName)) == NULL) {
        perror("Issue Encountered: Opening of directory is failed.");
        return 1;
    }
	
	msFilePtr = fopen(TEMPORARY_FILE_LIST, "a");
    if (msFilePtr == NULL) {
        fprintf(stderr, "Issue Encountered: Opening of file is failed.\n");
        return 1;
    }
	
    while ((msDirEntry = readdir(msDir)) != NULL) {
		
        if (msDirEntry->d_type == DT_DIR && strcmp(msDirEntry->d_name, ".") != 0 && strcmp(msDirEntry->d_name, "..") != 0) {
            if (msDirectoryName[strlen(msDirectoryName) - 1] == '/') {
				sprintf(msBuffer, "%s%s", msDirectoryName, msDirEntry->d_name);
			} else {
				sprintf(msBuffer, "%s/%s", msDirectoryName, msDirEntry->d_name);
			}
			
            msRecSearchExtension(msBuffer, msFileNames, msFileCount, msFileTotalCount);
        } else {
			if(strcmp(msDirEntry->d_name, ".") == 0 || strcmp(msDirEntry->d_name, "..") == 0)
				continue;
			DEBUG_LOG("Directory that is currently being read -> %s\n", msDirEntry->d_name);
            for (msCurrIndex = 0; msCurrIndex < msFileCount; msCurrIndex++) {
				DEBUG_LOG("Matching the I/P file name -> [%s] with the file in system -> [%s]\n", msFileNames[msCurrIndex], msDirEntry->d_name);
                if (strcmp(msFileNames[msCurrIndex], msDirEntry->d_name) == 0) {

					if (msDirectoryName[strlen(msDirectoryName) - 1] == '/') {
						sprintf(msBuffer, "%s%s", msDirectoryName, msDirEntry->d_name);
					} else {
						sprintf(msBuffer, "%s/%s", msDirectoryName, msDirEntry->d_name);
					}
					fprintf(msFilePtr, "%s\n", msBuffer);
					DEBUG_LOG("File is saved to the -> %s: %s \n", TEMPORARY_FILE_LIST, msBuffer);
                    *msFileTotalCount+=1;
                    break;
                }
            }
        }
    }
    closedir(msDir);
	fclose(msFilePtr);
	
    return 0;
}

/*
Function Name - ffgets

Function Purpose - 1. This function searches for the the specific set of files in th home directory of user.

2. Then it proceeds with the creation of compressed archive that contain these files, and sends it to the client connection that requested.

3. It then proceeds with the recursively searches for matching filename(s), then creating tar file and then perform file transfer to clients which are connected to the server..

4. Lastly, the temporary files are managed and deleted once the acknowledgemnt of successful transfer is received.
*/
int ffgets(int msClientFileDesc, char **msFileNames, int msFileTotalCount)
{	
	DIR *msDir;
    char *msTarCmd = malloc(sizeof(char) * MAXIMUM_BUFFER_SIZE_ALLOWED);
	char *msTarCmdFormat = "tar -czf %s -T %s";
    char *msHomeDir = USER_HOME_DIRECTORY;
	int msCurrIndex, msStatus;
    char msFiles[MAXIMUM_BUFFER_SIZE_ALLOWED-21];
	
	// Firstly, the command buffer is fully cleared.
    memset(msFiles, 0, sizeof(msFiles));

    DEBUG_LOG("Path of Home directory is -> %s\n", msHomeDir);
    DEBUG_LOG("%d <--The total number of parsed file Names.", msFileTotalCount);

    for (msCurrIndex = 0; msCurrIndex < msFileTotalCount; msCurrIndex++) {
        DEBUG_LOG(" [%s]", msFileNames[msCurrIndex]);
    }
	DEBUG_LOG("\n.........................\n");
	
    if ((msDir = opendir(msHomeDir)) == NULL) {
        DEBUG_LOG("Issue Encountered: Opening of directory is failed.\n");
        return 1;
    }

    int msFileCount = 0;
    if (msRecSearchName(msHomeDir, msFileNames, msFileTotalCount, &msFileCount) != 0) {
		DEBUG_LOG("Issue Encountered: Error in locating file(s).");
        return 1;
    }

    if (msFileCount == 0)
    {
        // In case files do not exist in the directory.
		msResponseForSendingText(msClientFileDesc, "There were no matching file(s) found.");
        return 0;
    }

    // Saving file(s) in the "temp.tar.gz"
    sprintf(msTarCmd, msTarCmdFormat, TEMPORARY_TAR_ARCH, TEMPORARY_FILE_LIST);;
	
	DEBUG_LOG("Number of files that were found-> %d\n", msFileCount);
    DEBUG_LOG("The command that will be executed-> %s\n", msTarCmd);

    // Execution of the "tar" command.
    system(msTarCmd);

    // Sharing file in the compressed form from Server -> Client.
    FILE *msFilePtr;
    char *msBuffer;
    long msFileLen;
    long int msFileSizeFgets;

    msFilePtr = fopen(TEMPORARY_TAR_ARCH, "rb"); // Performing the Open(binary mode) operation on the file.

    if (msFilePtr == NULL)
    {
        DEBUG_LOG("Issue Encountered: Opening of file is failed.\n");
        return 1;
    }

    fseek(msFilePtr, 0, SEEK_END);
    msFileSizeFgets = ftell(msFilePtr);
    fseek(msFilePtr, 0, SEEK_SET);

    msBuffer = (char *)malloc((msFileLen + 1) * sizeof(char)); // For the data of file, memory allocation will be done.
    fread(msBuffer, msFileLen, 1, msFilePtr);                   // Performing read operation on the file to store in buffer.

    // Sending the file size.
    send(msClientFileDesc, &msFileSizeFgets, sizeof(msFileSizeFgets), 0);
    // Saving the response during the transferring the file.
    int msFileTransferStatus = msResponseForSendingFile(msClientFileDesc, TEMPORARY_TAR_ARCH);
    if (msFileTransferStatus != 0)
    {
        DEBUG_LOG("Issue Encountered: Error while transferring the file(s).\n");
		return 1;
    }
    else
    {
        DEBUG_LOG("The operation of file transfer is completed successfully.\n");
    }
	
	// Now, deleting the file named - "temporary_file_list.txt"
    msStatus = remove(TEMPORARY_FILE_LIST);
    if (msStatus != 0) {
        DEBUG_LOG("Issue Encountered: Error while deleting the file - temporary_file_list.txt\n");
        return 0;
    }
    
    // Now, deleting the file named - "temp.tar.gz"
    msStatus = remove(TEMPORARY_TAR_ARCH);
    if (msStatus != 0) {
        DEBUG_LOG("Issue Encountered: Error while deleting the file - temp.tar.gz\n");
        return 0;
    }

    fclose(msFilePtr); // Performing the close operation on the file.

    return 0;
}

/*
Function Name - msRecSearchSize

Function Purpose - 1. This function will be recursively searches for files within a specified directory and its associated sub directories that lies within the particular size range.

2. It will read the directory entries, also checking if each entry is either a sub directory or a file.

3. In case, it is a 'sub directory', the function will be recusively perform search operation within that directory.

4. In case, it is a 'file' and its size lies within (msLowestSize, msHighestSize), then the path is saved to a temporary file list.

5. This list assists in further processing the matched file(s).

6. Once searching through all entries in the directory, the function will close the directory as well as the temporary file list.

7. In the end, the total count of the matched file(s) is updated and returned in the end.
*/
int msRecSearchSize(char *msDirectoryName, int msLowestSize, int msHighestSize, int *msFileTotalCount)
{

    DIR *msDir;
    struct dirent *msDirEntry;
    char msBuffer[MAXIMUM_BUFFER_SIZE_ALLOWED];
	FILE *msFilePtr;
	
	// Firstly, the command buffer is fully cleared.
	memset(msBuffer, 0, sizeof(msBuffer));
	
	DEBUG_LOG("Beginning the recursive search task by the size of file(s)...\n");
	
    // In case there is an issue while opening the root directory.
    if ((msDir = opendir(msDirectoryName)) == NULL) {
        DEBUG_LOG("Issue Encountered: Error while opening the directory.\n");
        return 1;
    }
	
	// Opening "TEMPORARY_FILE_LIST" in the append mode.
	msFilePtr = fopen(TEMPORARY_FILE_LIST, "a");
    if (msFilePtr == NULL) {
        fprintf(stderr, "Issue Encountered: Error during the opening of file.\n");
        return 1;
    }
    
     while ((msDirEntry = readdir(msDir)) != NULL)
    {
        // Omiting the Directories with single dot(.) and two consecutive dots(..)
        if (strcmp(msDirEntry->d_name, ".") == 0 || strcmp(msDirEntry->d_name, "..") == 0)
        {
            continue;
        }
		
		DEBUG_LOG("The directory that is read -> %s\n", msDirEntry->d_name);
		
		if (msDirectoryName[strlen(msDirectoryName) - 1] == '/') {
				sprintf(msBuffer, "%s%s", msDirectoryName, msDirEntry->d_name);
		} else {
			sprintf(msBuffer, "%s/%s", msDirectoryName, msDirEntry->d_name);
		}
		
        // Fetching the stats of File and Directories.
        struct stat msStatBuffer;
        if (stat(msBuffer, &msStatBuffer) == -1)
        {
            continue;
        }
		
		// In case it is directory then perform search operation recursively.
		if (msDirEntry->d_type == DT_DIR)
        {
            msRecSearchSize(msBuffer, msLowestSize, msHighestSize, msFileTotalCount);
        }
        // In case it is a file then checking its size if its in within the spcified limit.
       else 
       {
            DEBUG_LOG("The File -> %s is having size -> %ld\n", msDirEntry->d_name, msStatBuffer.st_size);

        // Performing a check if file size is within range(msLowestSize, msHighestSize).
           if (S_ISREG(msStatBuffer.st_mode) && msStatBuffer.st_size >= msLowestSize && msStatBuffer.st_size <= msHighestSize && msLowestSize <= msHighestSize) 
           {
             
				if (msDirectoryName[strlen(msDirectoryName) - 1] == '/') {
					sprintf(msBuffer, "%s%s", msDirectoryName, msDirEntry->d_name);
				} else {
					sprintf(msBuffer, "%s/%s", msDirectoryName, msDirEntry->d_name);
				}
				
				fprintf(msFilePtr, "%s\n", msBuffer);
				DEBUG_LOG("The file is saving to -> %s: %s \n", TEMPORARY_FILE_LIST, msBuffer);
				*msFileTotalCount+=1;

           }
        }
    }
    closedir(msDir);
	fclose(msFilePtr);
    return 0;

}

/*
Function Name - tarfgetz

Function Purpose - 1. This function ensured the purpose to compress and send file(s) of particualr sizes within a range (msLowestSize, msHighestSize) from a particular root directory to a client.

2. It considers lowest and highest size as a limit, then searches for files within that root directory that lies in the range (msLowestSize, msHighestSize), and compresses them using the 'tar' command. 

3. In case any of the files are found within that range and compression operation is success then the compressed file is sent from the server to the client connection.

4. In case no file is found within the mentioned size range (msLowestSize, msHighestSize) , a response is sent to the client that conveys the meaning that no file is found.

5. In the end, the function then performs cleanup by removing temproray file(s) during the execution of the function.
*/
int tarfgetz(int msFileDesOfClientSock, char** msArgs) {

    size_t msLowestSize = atoi(msArgs[0]);
    size_t msHighestSize = atoi(msArgs[1]);
    char *msRootPath = USER_HOME_DIRECTORY;	
    char msBuffer[MAXIMUM_BUFFER_SIZE_ALLOWED];
	char *msTarCmdFormat = "tar -czf %s -T %s"; // Tar Command format.
	int msStatus, msFileTotalCount = 0;
	
	DEBUG_LOG("Beginning the tarfgetz process...\n");
	DEBUG_LOG("File Limit Details -->");
	DEBUG_LOG("Lower Limit for File Size -> %ld\n",msLowestSize);
	DEBUG_LOG("Higehst Limit for File Size -> %ld\n",msHighestSize);
	
	// Fetching the files.
    msRecSearchSize(msRootPath, msLowestSize, msHighestSize, &msFileTotalCount);
    
    DEBUG_LOG("Total number of files found are -> %d\n",msFileTotalCount);
    sprintf(msBuffer, msTarCmdFormat, TEMPORARY_TAR_ARCH, TEMPORARY_FILE_LIST);

    if (msFileTotalCount > 0) {
		DEBUG_LOG("Currently executing the -> %s\n", msBuffer);
        int msStatus = system(msBuffer);
        if (msStatus == 0) {
            DEBUG_LOG("The execution of command is successfully completed.\n");
            // Sending the file(compressed) from Server -> Client.
            FILE *msFilePtr;
            char *msBuffer;
            long msFileSize;
            long int msFileSizeFgets;

            msFilePtr = fopen(TEMPORARY_TAR_ARCH, "rb"); // Performing the Open(binary mode) operation on the file.

            if (msFilePtr == NULL)
            {
                DEBUG_LOG("Issue Encountered: Error while opening the file.\n");
                return 1;
            }

            fseek(msFilePtr, 0, SEEK_END);
            msFileSize = ftell(msFilePtr);
            fseek(msFilePtr, 0, SEEK_SET);

            msBuffer = (char *)malloc((msFileSize + 1) * sizeof(char)); // Memory allocation will be done for data of file.
            fread(msBuffer, msFileSize, 1, msFilePtr);                   // File data will be read into the msBuffer.
			
            // Sending the file size.
            send(msFileDesOfClientSock, &msFileSize, sizeof(msFileSize), 0);
            // Saving the response during the transferring the file.
            int msFileTransferStatus = msResponseForSendingFile(msFileDesOfClientSock, TEMPORARY_TAR_ARCH);
            if (msFileTransferStatus != 0)
            {
                DEBUG_LOG("Issue Encountered: Error while transferring the file(s).\n");
            }
            else
            {
                DEBUG_LOG("Success Message: The operation of file transfer is successfully completed.\n");
            }

            fclose(msFilePtr);  // Performing the close operation on the file.
        } else {
            DEBUG_LOG("Issue Encountered: Error while executing the command.\n");
			return 1;
        }
    } else {
        // Sending a reponse in the text form in case no file is located/matched.
		DEBUG_LOG("Sorry, no files were found.\n");
		msResponseForSendingText(msFileDesOfClientSock, "Sorry, no files were found.");
    }
	
	// Now, deleting the file named - "temporary_file_list.txt"
    msStatus = remove(TEMPORARY_FILE_LIST);
    if (msStatus != 0) {
        DEBUG_LOG("Issue Encountered: Error while deleting the file - temporary_file_list.txt\n");
    }
    
    // Now, deleting the file named - "temp.tar.gz"
    msStatus = remove(TEMPORARY_TAR_ARCH);
    if (msStatus != 0) {
        DEBUG_LOG("Issue Encountered: Error while deleting the file - temp.tar.gz\n");
    }
	
    return 0;
}

/*
Function Name - msRecSearchDate

Function Purpose - 1. This function performs a recursive search for file(s) within a given directoty and teh associated sub directories based on the date range (msBeginDate, msEndDate).

2. This function takes arguments such as root directory path, date range which indicates begin date and end date sent from client.

3. Then fucntion proceeds with the iteration through the directory entries while omiiting '.' as well as '..' entries.

4. For all the entries, it checks if it is either a directory or a file.

5. In case, it is a 'directory', the function will be recusively perform search operation on that directory.

6. In case, it is a 'file' and its fetches the last modified/update date of the file and compares with the particular date range as per client request.

7. If in case the file's modification time falls with the particular date range then the file's path is saved to a temporay file list.

8. Then the list accumulates the all the files that are within the date criteria.

9. In the end, the function closes - directory, temporary file lists and returns the total count of the files that matches the date range criteria.
*/
int msRecSearchDate(char *msDirectoryName, time_t msBeginDate, time_t msEndDate, int *msFileTotalCount)
{
	DIR *msDir;
    struct dirent *msDirEntry;
    char msBuffer[MAXIMUM_BUFFER_SIZE_ALLOWED];
	FILE *msFilePtr;
	
	// Firstly, the command buffer is fully cleared.
	memset(msBuffer, 0, sizeof(msBuffer));
	
	DEBUG_LOG("Beginning the recursive search process by date limit...\n");
	
    // Displaying message in case there is an issue during the opening of directory.
    if ((msDir = opendir(msDirectoryName)) == NULL) {
        DEBUG_LOG("Issue Encountered: Error while opening the directory.\n");
        return 1;
    }
	
	// Opening the "TEMPORARY_FILE_LIST" in the append mode.
	msFilePtr = fopen(TEMPORARY_FILE_LIST, "a");
    if (msFilePtr == NULL) {
        fprintf(stderr, "Issue Encountered: Error while opening the file.\n");
        return 1;
    }

    
    while ((msDirEntry = readdir(msDir)) != NULL)
    {
        // Omiting the Directories with single dot(.) and two consecutive dots(..)
        if (strcmp(msDirEntry->d_name, ".") == 0 || strcmp(msDirEntry->d_name, "..") == 0)
        {
            continue;
        }
		
		DEBUG_LOG("Directory read information -> %s\n", msDirEntry->d_name);
		
		if (msDirectoryName[strlen(msDirectoryName) - 1] == '/') {
				sprintf(msBuffer, "%s%s", msDirectoryName, msDirEntry->d_name);
		} else {
			sprintf(msBuffer, "%s/%s", msDirectoryName, msDirEntry->d_name);
		}
		
		// In case it is a directory then recursively searching using "msRecSearchDate" function.
		if (msDirEntry->d_type == DT_DIR)
        {
            msRecSearchDate(msBuffer, msBeginDate, msEndDate, msFileTotalCount);
        }
		
        // In case it is a file then performing a check if it is within the specified date range and then print the file name.
        else{
			
			// Fetching the stats of File(s) as well as Directories.
			struct stat msStatBuffer;
			if (stat(msBuffer, &msStatBuffer) == -1)
			{
				continue;
			}
			
			time_t msFileLastUpdateTime;
			#ifdef __APPLE__
				msFileLastUpdateTime = msStatBuffer.st_birthtime;
			#else
				msFileLastUpdateTime = msStatBuffer.st_mtime;
			#endif
			
			DEBUG_LOG("The file name -> %s having last modification date -> %ld\n", msDirEntry->d_name, msFileLastUpdateTime);
            if (msFileLastUpdateTime >= msBeginDate && msFileLastUpdateTime <= msEndDate)
            {	

				if (msDirectoryName[strlen(msDirectoryName) - 1] == '/') {
					sprintf(msBuffer, "%s%s", msDirectoryName, msDirEntry->d_name);
				} else {
					sprintf(msBuffer, "%s/%s", msDirectoryName, msDirEntry->d_name);
				}
				fprintf(msFilePtr, "%s\n", msBuffer);
				DEBUG_LOG("File(s) saved to %s: %s \n", TEMPORARY_FILE_LIST, msBuffer);
                *msFileTotalCount+=1;
            }
        }
    }
    closedir(msDir);
	fclose(msFilePtr);
	
	return 0;
}

/*
Function Name - getdirf

Function Purpose - 1. This function will search for a file(s) based on the (msBeginDate, msEndDate) and sends that to the client in a compressed format.

2. It will take 2 arguments - Client Socket Descriptor(msFileDesOfClientSock), date range arguments(msArgs) as input.

3. First, it will start with the conversion of date strings to UNIX timestamps.

4. Then it proceeds with the setting of root path as the user's home directory and intializes the variables.

5. Then it will initialize a search operation for files within the date range using the function 'msRecSearchDate'.

6. Then it checks the total count of the matching files.

7. In case the file(s) are found then it will create command to compress the file(s) using 'tar' -> Execute that command -> Send the compressed archive to the client in case of success.

8. During the above process, various error conditions are handled and these condition return error codes in case any issue is encountered.

9. Once transfer is complete, deletion of temporary files will take place.

10. Lastly,it returns 0 for success and in case no file(s) matches with the date range then it proceeds with sending a message to the client and return 0.
*/
int getdirf(int msFileDesOfClientSock, char **msArgs)
{
    time_t msBeginDate = msConvertTimeToUnixFromDate(msArgs[0], 1);
    time_t msEndDate = msConvertTimeToUnixFromDate(msArgs[1], 2);
    char *msRootPath = USER_HOME_DIRECTORY;	
    char msBuffer[MAXIMUM_BUFFER_SIZE_ALLOWED];
	char *msTarCmdFormat = "tar -czf %s -T %s"; // Tar Command Format.
	
	DEBUG_LOG("Beginning the - getdirf process...\n");

	int msStatus, msFileTotalCount = 0;
	
	// Fetching the files.
    msRecSearchDate(msRootPath, msBeginDate, msEndDate, &msFileTotalCount);
	DEBUG_LOG("Total number of files that were found -> %d\n",msFileTotalCount);
    sprintf(msBuffer, msTarCmdFormat, TEMPORARY_TAR_ARCH, TEMPORARY_FILE_LIST);

    if (msFileTotalCount > 0) {
		DEBUG_LOG("Performing Operation -> %s\n", msBuffer);
        int msStatus = system(msBuffer);
        if (msStatus == 0) {
            DEBUG_LOG("The execution of command is successfully done.\n");
            
			// Sending the file(compressed form) from Server -> Client.
            FILE *msFilePtr;
            char *msBuffer;
            long msFileSize;
            long int msFileSizeFgets;

            msFilePtr = fopen(TEMPORARY_TAR_ARCH, "rb"); // Performing the Open(binary mode) operation on the file.

            if (msFilePtr == NULL){
                DEBUG_LOG("Issue Encounterd: Error during the opening of file.\n");
                return 1;
            }

            fseek(msFilePtr, 0, SEEK_END);
            msFileSize = ftell(msFilePtr);
            fseek(msFilePtr, 0, SEEK_SET);

            msBuffer = (char *)malloc((msFileSize + 1) * sizeof(char)); // For the data of file, memory allocation will be done.
            fread(msBuffer, msFileSize, 1, msFilePtr);                   // Performing read operation on the file to store in buffer.

            // Sending the file size.
            send(msFileDesOfClientSock, &msFileSize, sizeof(msFileSize), 0);
            
			// Saving the response during the transferring the file.
            int msFileTransferStatus = msResponseForSendingFile(msFileDesOfClientSock, TEMPORARY_TAR_ARCH);
            if (msFileTransferStatus != 0){
                DEBUG_LOG("Issue Encountered: Error while transferring the file(s).\n");
            } else{
                DEBUG_LOG("The operation of transfer on File(s) is successfully completed.\n");
            }

            fclose(msFilePtr); 
        } else {
            DEBUG_LOG("Issue Encountered: Error while executing the command.\n");
			return 1;
        }
    } else {
        // In case no file is found - sending a message.
		DEBUG_LOG("Sorry, There were no files within your specified args.\n");
		msResponseForSendingText(msFileDesOfClientSock, "Sorry, there were no files within your specified args.");
		return 0;
    }
	
	// Now, deleting the file named - "temporary_file_list.txt"
    msStatus = remove(TEMPORARY_FILE_LIST);
    if (msStatus != 0) {
        DEBUG_LOG("Issue Encountered: Error while deleting the file - temporary_file_list.txt\n");
    }
    
    // Now, deleting the file named - "temp.tar.gz"
    msStatus = remove(TEMPORARY_TAR_ARCH);
    if (msStatus != 0) {
        DEBUG_LOG("Issue Encountered: Error while deleting the file - temp.tar.gz\n");
    }

    return 0;
}

/*
Function Name - msRemoveLineBreakFromBuff

Function Purpose - 1. This function trims the trailing new characters('\n') from the provided charcter array('msBuffer').

2. It checks if the array ends with a newline then it replaces that with a null terminator('\0') in order to remove the line break.

3. Its mainly used to handle strings without the unwanted line breaks in case it exists which often comes during I/P processing.
*/
void msRemoveLineBreakFromBuff(char msBuffer[]) {
    int msBufferLength = strlen(msBuffer);
    if (msBufferLength > 0 && msBuffer[msBufferLength - 1] == '\n') {
        msBuffer[msBufferLength - 1] = '\0';
    }
}

/*
Function Name - msConvertTimeToUnixFromDate

Function Purpose - 1. This function converts a date string of format: (YYYY-MM-DD) into a Unix timestamp. 

2. It allows the timestamp wheather it should represent either the start or end of the day. 

3. Then the function proceeds with the parsing of the date, adjusting the year and the month, 

4. Based on Point number 3, it sets the time based on the type, also handles the DST(daylight saving time), 

5. In the end, it returns the Unix timestamp. 

6. As a result, if successful, the function will then return the timestamp;
   otherwise, it returns the value -1.
*/
time_t msConvertTimeToUnixFromDate(const char *msTimeStr, int msDateType)
{
    struct tm msConvertedTime;
    int msYear, msMonth, msDay;
    if (sscanf(msTimeStr, "%d-%d-%d", &msYear, &msMonth, &msDay) != 3)
    {
        return (time_t)-1;
    }
    msConvertedTime.tm_year = msYear - 1900;  // Doing adjustment for the year if begins from 1900.
    msConvertedTime.tm_mon = msMonth - 1;     // Doing adjustment for the month, in case begins from 0.
    msConvertedTime.tm_mday = msDay;		  // No adjustment required for date.
	if(msDateType == 1){
		msConvertedTime.tm_hour = 0;
		msConvertedTime.tm_min = 0;
		msConvertedTime.tm_sec = 0;
	}
	else{
		msConvertedTime.tm_hour = 23;
		msConvertedTime.tm_min = 59;
		msConvertedTime.tm_sec = 59;
	}
    msConvertedTime.tm_isdst = -1;  // Setting DST to -1
    time_t t = mktime(&msConvertedTime);
	time_t tempTimeVar = (time_t)-1;
    if (t == tempTimeVar)
    {
        return tempTimeVar;
    }
    return t;
}
