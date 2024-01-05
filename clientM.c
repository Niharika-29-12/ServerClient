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

// Including the required set of header files which is essential for the execution of the program(client program file).
#include <sys/wait.h>
#include <time.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/signal.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdbool.h>
#include <netdb.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <ctype.h>

// Declaring some global macros/constants that will be used throughout the execution of the program(client program file).
// Declared global macro/cosntant for debug mode.
#define STATUS_OF_DEBUG_MODE 0

// Declared some global macros/constants for maximum limits.
#define MAXIMUM_BUFFER_SIZE_ALLOWED 1024
#define MAXIMUM_ARGUMENTS_ALLOWED 10

// Declared some global macros/constants for response type for - Structure, file, text.
#define RESP_TYPE_FILE 3
#define RESP_TYPE_STRUCT 2
#define RESP_TYPE_TEXT 1

// Declared global macro/constant for port number reference.
#define NUM_OF_SERVER_PORT 56789

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

// Defined 'hs_AddressInformation' of type struct to bundle together - IP & Port no. for transfer.
typedef struct {
    char hs_IPAddr[INET_ADDRSTRLEN];
    int hs_PortNum;
} hs_AddressInformation; 

// Declared some flag variables to indicate whether to perform few actions such as Quit and unzip.
static int hs_shouldUnzip = 0;
int hs_shouldQuit = 0;

// Declared variable to indicate in case file was found or not.
int hs_fileFound = 0;  

// Declared function('hs_validateInputCommand') to validate the input command from the user
int hs_validateInputCommand(char* userInput);

// Declared function('hs_checkForUnzipOption') to check if the Unzip option is selected
void hs_checkForUnzipOption(char userInput[]);

// Declared function('hs_removeLineBreaks') to remove line breaks from input
void hs_removeLineBreaks(char userInput[]);

// Declared function('hs_validateDateInput') to validate an input date
int hs_validateDateInput(char date[]);

// Declared function('hs_areAllDigits') to check if all characters in a string are digits
int hs_areAllDigits(char* str);

// Declared function('hs_generateFileTransmissionProgressBar') to generate a progress bar for file transmission
void hs_generateFileTransmissionProgressBar(int hs_totalSize, int hs_bytesReceived, time_t hs_transmissionStartTime);

/*
Function Name - main()

Function Purpose - Logic execution of the main function for Cient-Mirror-Server Connection(in the client program file).

Function Handles - It will handle the following - 
1. network related variables, 
2. establishes connection to either server or mirror based on the conditions defined, 
3. enters a loop to handle user I/P and O/P from server/mirror response,
4. validates the user I/P commands, 
5. manages file transmission, and 
6. responds to various user input.

Function Working - 1. Declared variables - socket descriptor, addresses, and buffers.

2. Checks if a server IP is provided as a commnad line argument. If missing then print usage.

3. Then proceed with the creation of socket and configuting the server address.

4. Connecting to a server and in case it is unsuccessful, it prints an appropriate error message.

5. Then receieves an intial repsonse type from server and log that.

6. Handles 2 intial response type:
   If 'text' then print a connection message.
   If 'structure', then redirection will take place i.e. switch to mirror server.

7. Then the process in main loop where interaction will take place -

7.1 Asks for a command → Remove line breaks is exists → Then Checks for the unzip option.
    ↓↓↓↓↓↓↓↓↓
7.2 Then command validation will take place and continue only if it is valid date input.
	↓↓↓↓↓↓↓↓↓
7.3 The the command will be sent to the server/mirror.
	↓↓↓↓↓↓↓↓↓
7.4 Once the command is received by Server/Mirror then processing of response headers will take place.
	↓↓↓↓↓↓↓↓↓
7.5 Handles 2 types of reponse type
	If response type - 'text',then print the received text message.
	If response type - 'file',then receive and save the file data and show the progress.
	↓↓↓↓↓↓↓↓↓
7.6 In case server disconnection is detected and then break from the loop.
	↓↓↓↓↓↓↓↓↓
7.7 Also, handles the 'QUIT' command by printing a message and breaking from the loop.
	↓↓↓↓↓↓↓↓↓
7.8 In case the unzip flag is set, unzip of the received file will take place.

8. In the end, the connection is closed, and print a connection closed message, and return 0 in the end.
*/
int main(int argumentCount, char *argumentValues[]) {
    int hs_socketDescriptor = 0, hs_bytesRead = 0, hs_fileFound = 0, hs_shouldUnzip = 0;
    struct sockaddr_in hs_serverAddress;
	struct sockaddr_in hs_mirrorAddress;
    char hs_commandBuffer[MAXIMUM_BUFFER_SIZE_ALLOWED] = {0};
    char hs_responseText[MAXIMUM_BUFFER_SIZE_ALLOWED];
    char hs_responseFileData[MAXIMUM_BUFFER_SIZE_ALLOWED];
    char hs_inputValueBuffer[MAXIMUM_BUFFER_SIZE_ALLOWED];
    char hs_serverIPAddress[16];
    char *hs_archiveFilename = "temp1.tar.gz";

    if (argumentCount < 2) {
        printf("Suggestion -----> Correct Command Usage ---> %s <server_ip> \n", argumentValues[0]);
        return 1;
    }
    strcpy(hs_serverIPAddress, argumentValues[1]);

    if ((hs_socketDescriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Warning  -----> Some issues found: There was a issue while creating socket!! \n");
        return 1;
    }

    //reading ip address and port number
	hs_serverAddress.sin_family = AF_INET;
    hs_serverAddress.sin_port = htons(NUM_OF_SERVER_PORT);

    if (inet_pton(AF_INET, hs_serverIPAddress, &hs_serverAddress.sin_addr) <= 0) {
        printf("Warning  -----> Some issues found: Address entered is either invalid/unsupported. Please check it!! \n");
        return 1;
    }

    if (connect(hs_socketDescriptor, (struct sockaddr *)&hs_serverAddress, sizeof(hs_serverAddress)) < 0) {
        printf("Warning  -----> Some issues found: Connection request is failed. Please check it!! \n");
        return 1;
    }

    long initialResponseType = 0;
    recv(hs_socketDescriptor, &initialResponseType, sizeof(initialResponseType), 0);

    DEBUG_LOG("Hint -----> Printing initial responses type ---> %ld\n", initialResponseType);

	// During normal conection server sending text 
	if (initialResponseType == RESP_TYPE_TEXT) {
        printf("Success -----> Success Message: Connection established to the server. Now, perform some operations!!! \n");

        // Greet message from srver
		memset(hs_responseText, 0, sizeof(hs_responseText));//setting memory
        read(hs_socketDescriptor, hs_responseText, sizeof(hs_responseText));//reading
        printf("Greet Message -----> From the server -----> %s\n", hs_responseText);
    } 
	
	// In case of redirection, server sending structure to mirror
	else if (initialResponseType == RESP_TYPE_STRUCT) 
	{
        printf("Alert -----> Redirection Message -----> Due to condition Server is redirecting to Mirror. Please check mirror terminal for the next messages and logs!! \n");

        hs_AddressInformation mirrorInfo;
		
		//Strucutre reading mirror ip adress and port number
        recv(hs_socketDescriptor, &mirrorInfo, sizeof(hs_AddressInformation), 0);

        // Closng connection to Main Server
		close(hs_socketDescriptor);

        // log message printing
		DEBUG_LOG("The Mirror IP which was sent from server is -----> %s\n", mirrorInfo.hs_IPAddr);
        DEBUG_LOG("The Mirror Port which was sent from server is -----> %d\n", mirrorInfo.hs_PortNum);

        // Creating a new socket for next operations
		hs_socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);
        if (hs_socketDescriptor < 0) {
            printf("Warning  -----> Some issues found: The creation of Socket is failed !!!\n");
            return 1;
        }

        //reading ip address and port number
		hs_mirrorAddress.sin_family = AF_INET;
        hs_mirrorAddress.sin_port = htons(mirrorInfo.hs_PortNum);
        if (inet_pton(AF_INET, mirrorInfo.hs_IPAddr, &hs_mirrorAddress.sin_addr) <= 0) {
            printf("Warning  -----> Some issues found: The Mirror address is NOT valid. Please check it!!!\n");
            return 1;
        }

        // Connecting to the Mirror Server
		if (connect(hs_socketDescriptor, (struct sockaddr *)&hs_mirrorAddress, sizeof(hs_mirrorAddress)) < 0) {
            printf("Warning  -----> Some issues found: Client failed to make a connection to mirror. Please check it!!!\n");
            return 1;
        }

        printf("Success Message: Connection established to the mirror server successfully.\n");
        // Reading greet message from Mirror Server
		memset(hs_responseText, 0, sizeof(hs_responseText));//memeroy set
        read(hs_socketDescriptor, hs_responseText, sizeof(hs_responseText));
        printf("Success -----> Greet message from Mirror Connection -----> %s\n", hs_responseText);
    }

    // the Main process to interact with Server or Mirror
	while (1) 
	{
        // Clearng the command buffer
		memset(hs_commandBuffer, 0, sizeof(hs_commandBuffer));
        hs_fileFound = 0;

        printf("\nPlease enter your command: ");
        fgets(hs_commandBuffer, MAXIMUM_BUFFER_SIZE_ALLOWED, stdin);
		
		// Cleaning linebreak from input stream
        hs_removeLineBreaks(hs_commandBuffer);

		// Checking for unzip option
        int userInputLength = strlen(hs_commandBuffer);
        // Check if the input ends with " -u" indicating unzip option
        if (userInputLength >= 3 && strcmp(hs_commandBuffer + userInputLength - 3, " -u") == 0) {
            hs_shouldUnzip = 1;  // Set the flag to indicate unzip option
            hs_commandBuffer[userInputLength - 3] = '\0'; // Remove the " -u" from the input
        }

        // Validating input commands
		strcpy(hs_inputValueBuffer, hs_commandBuffer);
        if (hs_validateInputCommand(hs_inputValueBuffer)) {
            continue;
        }

        // sending commands to server to execute opeartions
		send(hs_socketDescriptor, hs_commandBuffer, strlen(hs_commandBuffer), 0);

        // Receiving header to identify response type for the commands
		long responseHeader;
        hs_bytesRead = read(hs_socketDescriptor, &responseHeader, sizeof(responseHeader));

        if (hs_bytesRead > 0) {
			// Texts response from mirror/server
            if (responseHeader == RESP_TYPE_TEXT) {
                memset(hs_responseText, 0, sizeof(hs_responseText));
				
				// Reading and delivering output text response
                read(hs_socketDescriptor, hs_responseText, sizeof(hs_responseText));
                printf("The responses in text form are -----> %s\n", hs_responseText);
            } 
			// File Response
			else {
                hs_fileFound = 1;
				// Obtaining file size from the header
                long fileSize = responseHeader;

                // Clear the response file buffer
				memset(hs_responseFileData, 0, sizeof(hs_responseFileData));

                FILE *filePointer = fopen(hs_archiveFilename, "wb");
                if (filePointer == NULL) {
                    printf("Warning  -----> Some issues found: There are some error during the creation of physical file. Please check it!!!\n");
                    return 1;
                }

                //starttime declaration
				time_t startTime;
                time(&startTime);

                // Receiving data from the server
				long totalBytesReceived = 0;
                printf("Size of received file is -----> %ld Bytes\n", fileSize);
                while (totalBytesReceived < fileSize) {
                    hs_generateFileTransmissionProgressBar(fileSize, totalBytesReceived, startTime);
                    int bytesToReceive = MAXIMUM_BUFFER_SIZE_ALLOWED;
                    if (totalBytesReceived + MAXIMUM_BUFFER_SIZE_ALLOWED > fileSize) {
                        bytesToReceive = fileSize - totalBytesReceived;
                    }
                    int receivedBytes = recv(hs_socketDescriptor, hs_responseFileData, bytesToReceive, 0);
                    if (receivedBytes < 0) {
                        printf("Warning  -----> Some issues found: There is a issue while receiving file data. Please check the file!!!");
                        return 1;
                    }
					//writing file size
                    fwrite(hs_responseFileData, sizeof(char), receivedBytes, filePointer);
                    totalBytesReceived += receivedBytes;
                    if (totalBytesReceived >= fileSize) {
                        break;
                    }
                }
				//progressbar generation
                //hs_generateFileTransmissionProgressBar(fileSize, totalBytesReceived, startTime);
                printf("\n Received file name is -----> %s\n", hs_archiveFilename);
                fclose(filePointer);
            }
        } 
		else 
		{
            // Read 0 bytes from server ie. disconnected!!
			printf("Alert -----> Message: Server is disconnected now.\n");
            break;
        }
		// Handling command quit
		if(hs_shouldQuit)
		{
			printf("Quit command has been executed successfully.So quitting!!!!!!!");
			printf("Success: -----> QUIT comand has been executed.");
			break;
		}


        // Handleing unzip option <-u> on client side
		if (hs_shouldUnzip && hs_fileFound) 
		{
			char cmd[MAXIMUM_BUFFER_SIZE_ALLOWED];
			snprintf(cmd, MAXIMUM_BUFFER_SIZE_ALLOWED, "tar -xzf %s", hs_archiveFilename);
			system(cmd);
			printf("Success Message -----> Unziping of file has been done successfully...!!!\n");
			hs_shouldUnzip = 0;
		}
		}

	// Closeing connection to Server / Mirror
	close(hs_socketDescriptor);
    printf("Alert -----> Connection is now closed...!!!\n");
    return 0;
}

/*
Function Name - hs_generateFileTransmissionProgressBar

Function Purpose - This function used to create progress bar dynamically for mainly displaying the progress of a file transfer.

Function Working - 1. It takes arguments such as file size, number of bytes received so far, start time of transfer.

2. It calculates the progress %, converts that to filled and empty blocks for visualzing, computes the elapse time since the start time of transfer, and calculates the transfer rate in Kilobytes/second.

3. Then it will print the progress bar using '#' for filled blocks and spaces for empty blocks. It will also print the progress % and the transfer rate. 

4. Then the function updates the display which provides uses with update of transfer process.
*/
void hs_generateFileTransmissionProgressBar(int hs_totalSize, int hs_bytesReceived, time_t hs_transmissionStartTime) {
    // Calculate the percentage of progress
    int hs_progressPercentage = (int)((float)hs_bytesReceived / (float)hs_totalSize * 100);

    // Calculate the number of filled and empty blocks in the progress bar
    int hs_numFilledBlocks = hs_progressPercentage / 5;
    int hs_numEmptyBlocks = 24 - hs_numFilledBlocks;

    // Get the current time to calculate time elapsed since the transfer began
    time_t hs_currentTime;
    time(&hs_currentTime);
    double hs_timeElapsed = difftime(hs_currentTime, hs_transmissionStartTime);

    // Calculate the transfer rate in KB/s
    double hs_transferRate = (double)hs_bytesReceived / hs_timeElapsed;

    // Print the progress bar
    printf("[");
    for (int filledBlock = 0; filledBlock < hs_numFilledBlocks; filledBlock++) {
        printf("##");  // Display filled block
    }
    for (int emptyBlock = 0; emptyBlock < hs_numEmptyBlocks; emptyBlock++) {
        printf(" ");  // Display empty block
    }

    // Print progress percentage and transfer rate
    printf("] %d%% %.2f KB/s", hs_progressPercentage, hs_transferRate / 1024);

    fflush(stdout); // Flush the output buffer to update the display
    printf("\r");   // Move the cursor to the beginning of the line to update the progress
}


/*
Function Name - hs_removeLineBreaks

Function Purpose/Working - 1. This function trims the trailing new characters('\n') from the provided charcter array.

2. It checks if the array ends with a newline then it replaces that with a null terminator('\0') in order to remove the line break.

3. Its mainly used to handle strings without the unwanted line breaks in case it exists which often comes during I/P processing.
*/
void hs_removeLineBreaks(char userInput[]) {
    int textLength = strlen(userInput);

    // Check if the string has content and ends with a line break
    if (textLength > 0 && userInput[textLength - 1] == '\n') {
        userInput[textLength - 1] = '\0';  // Remove the line break by null-terminating the string
    }
}

// Function to validate an input date
/*
Function Name - hs_validateDateInput

Function Purpose/Working - 1. It validates the format of a date string in the format 'YYYY-MM-DD'.

2. It parses Year, Month, and Day from the format.

3. Check is performed  based on the rules of Gregorian calendar on the year range, month range, day range, days in months, February month.

4. Then it returns the validation result
   - In case all checks are passed then the date is valid can be concluded as per specified conditions and funstion returns 1.
   - In case any 1 check is failed then the date is invalid can be concluded as per specified conditons and function returns 0.
*/

int hs_validateDateInput(char dateString[]) {
    int hs_year, hs_month, hs_day;

    // Parse the year, month, and day components from the input string
    if (sscanf(dateString, "%d-%d-%d", &hs_year, &hs_month, &hs_day) != 3) {
        // Parsing failed, invalid format
        return 0;
    }

    // Check if the year is within valid range (between 1000 and 9999)
    if (hs_year < 1000 || hs_year > 9999) {
        return 0;
    }

    // Check if the month is within valid range (between 1 and 12)
    if (hs_month < 1 || hs_month > 12) {
        return 0;
    }

    // Check if the day is within valid range (between 1 and 31)
    if (hs_day < 1 || hs_day > 31) {
        return 0;
    }

    // Check for months with 30 days
    if ((hs_month == 4 || hs_month == 6 || hs_month == 9 || hs_month == 11) && hs_day > 30) {
        return 0;
    }

    // Check for February
    if (hs_month == 2) {
        // Leap year
        if ((hs_year % 4 == 0 && hs_year % 100 != 0) || hs_year % 400 == 0) {
            if (hs_day > 29) {
                return 0;
            }
        }
        // Non-leap year
        else {
            if (hs_day > 28) {
                return 0;
            }
        }
    }

    // If all checks pass, the date is valid
    return 1;
}

/*
Function Name - hs_areAllDigits

Function Purpose/Working - 1. It will check wheather every character in the provided string is numeric digit or not.

2. To check for all digits if numeric, it iterates over the string and utilizes 'isdigit' function to check if each character is a digit or not.

3. In case any single character is not a digit, the function returns 0
   otherwise 1 is returned in case of all characters are digits.   
*/
int hs_areAllDigits(char* string) {
    for (int index = 0; string[index] != '\0'; index++) {
        if (!isdigit(string[index])) {
            return 0;  // Not all characters are digits
        }
    }
    return 1;  // All characters are digits
}

// Function to validate the input command from the user
/*
Function Name - hs_validateInputCommand

Function Purpose - Mainatining the correctness of all the user interactions during the program execution by allowing only allowed command format and argument requirements only.

Function Working - 1. The function validates the user I/P commands and the passed arguments in command by the user.

2. Then tokenize the I/P -> checks the validity of command -> ensure right number of arguments and perform validations for I/P commands.

3. It also handles unzip flag and the quit command from user I/P.

4. In case all the validations are passed the function returns 0
   Otherwise in case of any 1 validation failure, it returns 0.
   It basically checks if I/P is valid or not.   
*/

int hs_validateInputCommand(char* userInput) {
  // Tokenize the input string into arguments
  char* tokens[MAXIMUM_ARGUMENTS_ALLOWED]; 
  int numTokens = 0;

  // Use strtok to split input by space delimiter
  char* token = strtok(userInput, " ");
  
  while (token != NULL) {
    
    // Save each token 
    tokens[numTokens++] = token;
    
    // Get next token
    token = strtok(NULL, " "); 
  }

  // Add null at end
  tokens[numTokens] = NULL;

  // Check if command entered is valid
  if (strcmp(tokens[0], "fgets") != 0 &&  //check command in client
      strcmp(tokens[0], "tarfgetz") != 0 &&
	  strcmp(tokens[0], "filesrch") != 0 &&
      strcmp(tokens[0], "targzf") != 0 &&
      strcmp(tokens[0], "getdirf") != 0 &&
      strcmp(tokens[0], "quit") != 0) {
      
    printf("Warning  -----> The command entered is not valid. Please try again with a valid command.\n");
    return 1;
  }
  
    
  // Checking the provided command as a arguments
  if(numTokens < 2 && strcmp(tokens[0], "quit") != 0){
    printf("Suggestion -----> Please provide the required arguments for the command. PleaseTry again.\n");
    return 1; 
  }

  // Checkng max arguments not exceeded 
  else if(numTokens > MAXIMUM_ARGUMENTS_ALLOWED){
    printf("Alert -----> Too many arguments. Please check and try again.\n");
    return 1;
  }

  // Validating args for filesrch  
  if(strcmp(tokens[0], "filesrch") == 0 && numTokens > 2){
    printf("The filesrch command accepts only 1 argument. Please try again.\n");
    return 1;
  }

// Checking the command arguments as per requirements and display the appropriate messages.
    if( strcmp(tokens[0], "targzf") == 0 && (numTokens > 5 || numTokens < 2)){
        printf("Issue Encountered - targzf command allows minimum 1 extension and maximum 4 extensions.\n");
        return 1;
    }

 

    //verify the command arguments in case of 'fgets' command
    if( strcmp(tokens[0], "fgets") == 0 && (numTokens > 5 || numTokens < 2)){
        printf("Issue Encountered - fgets command allows minimum 1 file names and maximum 4 file names.\n");
        return 1;
    }

  // Validateing args for tarfgetz
  if(strcmp(tokens[0], "tarfgetz") == 0){
    
    // Checking bounds
    if(numTokens > 3){
      printf("Alert -----> Too many arguments for tarfgetz. Please check the arguments!\n");
      printf("Usage Suggestion -----> tarfgetz requires 2 size arguments and an optional -u flag.\n");
      return 1;
    }
    
    // Validate size arguments
    if(!hs_areAllDigits(tokens[1]) || !hs_areAllDigits(tokens[2])){
      printf("Alert -----> Invalid size values entered. Size arguments must be valid integers. Please try again.\n");
      printf("Usage Suggestion ----->  tarfgetz requires 2 size arguments and an optional -u flag.\n");
      return 1;
    }
    
    // Compareing size bounds
    if(atoi(tokens[1]) > atoi(tokens[2])){
      printf("Alert -----> Size 1 cannot be greater than Size 2. Please specify valid size bounds.\n");
      return 1;
    }
  }

  // Validating args for getdirf
  if(strcmp(tokens[0], "getdirf") == 0) {
    // Argument bounds check
    if(numTokens > 3){
      printf("Alert -----> Too many arguments for getdirf. Please try again.\n");    
      printf("Usage Suggestion ----->  getdirf requires 2 date arguments and an optional -u flag.\n");
      return 1;
    }

    // Validateing date formats
    if(!hs_validateDateInput(tokens[1]) || !hs_validateDateInput(tokens[2])) {
      printf("Alert -----> Invalid date arguments. Dates must be in valid format YYYY-MM-DD. Please try again.\n");
      printf("Usage Suggestion ----->  getdirf requires 2 date arguments and an optional -u flag.\n");  
      return 1;
    }

    struct tm timm_1 = { 0 };
	int hs_year, hs_month, hs_day;
	if (sscanf(tokens[1], "%d-%d-%d", &hs_year, &hs_month, &hs_day) == 3) {
	timm_1.tm_year = hs_year - 1900;
	timm_1.tm_mon = hs_month - 1;
	timm_1.tm_mday = hs_day;
	}

	//date1
	time_t date001 = mktime(&timm_1);
	struct tm timm_2 = { 0 };
	if (sscanf(tokens[2], "%d-%d-%d", &hs_year, &hs_month, &hs_day) == 3) {
	timm_2.tm_year = hs_year - 1900;
	timm_2.tm_mon = hs_month - 1;
	timm_2.tm_mday = hs_day;
	}

	//date 2
	time_t date002 = mktime(&timm_2);

	// Compare dates    
    if(date001 > date002){
      printf("Warning -----> Date 1 cannot be later than Date 2. Please specify valid date bounds.\n");
      return 1;
    }
  }

  // Preventing unzip with invalid commands
  if((strcmp(tokens[0], "filesrch") == 0 || 
       strcmp(tokens[0], "quit") == 0) &&
       hs_shouldUnzip == 1) {
    hs_shouldUnzip = 0;
    hs_shouldQuit = 0;
    printf("Alert -----> The unzip flag <-u> cannot be used with filesrch or quit commands.\n");
    return 1;
  }

  // Set quit flag
  if(strcmp(tokens[0], "quit") == 0){
    printf("Alert -----> The Quit command has been entered. Now, Client will exit.!!!\n");
	hs_shouldQuit = 1; 
  }

  return 0; 
}
