#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <errno.h>

int i;
int globErr=0;
void helpMe(){
	printf("Type 'put', then your message when prompted to put a message in the current box\n");
	printf("Type 'next' to get next message from current open box\n");
	printf("Type 'open', then box name when prompted to open an existing box\n");
	printf("Type 'close', then box name when prompted to close an existing box\n");
	printf("Type 'create', then box name when prompted to create a new box\n");
	printf("Type 'delete', then box name when prompted to delete a box\n");
	printf("Type 'quit' to end server connection\n");
}
char *itoa(int num){
	char str[12];
	//just fills it with terminating chars just to be safe
	for(i=0;i<12;i++){
		str[i]='\0';
	}
    sprintf(str, "%d", num);//puts the string rep of num into str
	for(i=0;str[i]!='\0';i++);//literally just used to find first terminating char
	//i know i said we'd be lazy but saw a malloc
	int numStrLen = i+1;
	char*numStr = malloc(numStrLen*sizeof(char));
	for(i=0;str[i]!='\0';i++){
		numStr[i]=str[i];
	}
	numStr[i]='\0';

    return numStr;
}
//sends the command
void sendCmd(char* cmd, int socfd){
	printf("Command to be sent is: %s", cmd);
	int i = 0;
	while (i < strlen(cmd)){
		int numBytes = write(socfd, cmd, strlen(cmd)+1);
		if (numBytes == -1){
			printf("WRITE ERROR in sendCmd()\n");
			printf("error: '%d'\n", errno);
			break;
		}
		else{
			printf("send '%d' bytes successfully\n", numBytes);
			i += numBytes;
		}
	}
}
//reads the command
char* readCmd(int socfd){
	printf("  made it into read cmd\n");
	char *input=malloc(1*sizeof(char));
	char buff[1];
	size_t inputlen = 0;
	do{
		int bytesRead = 0;
		bytesRead = read(socfd,buff,1);
		if (bytesRead == 0){ //server disconected socket
			printf("The server has disconnected\n");
			exit(0);
		}
		input = realloc(input, inputlen+1);
		strcpy(input+inputlen,buff);
		inputlen += 1;
	} while(buff[0]!='\0');	
	printf("   Server sent: %s, %d bytes, %c\n",input, strlen(input)+1,input[strlen(input)-1]);
	return input;
}
//creates one string for the whole of user's input
char* createInput(){
	char* input = NULL;
	char buff[5];
	size_t inputlen = 0, templen = 0;
	do{
		fgets(buff,5,stdin);
		templen = strlen(buff);
		input = realloc(input, inputlen+templen+1);
		strcpy(input+inputlen,buff);
		inputlen += templen;
	} while(templen ==4&&buff[3]!='\n');
	input[strlen(input)-1]='\0';
	printf("created input: %s length %d\n", input, strlen(input));
	return input;
}
//used for literally one case
int startsWith(char* string, char*refString){
	if(strncmp(string,refString, strlen(refString))==0){
		return 0;
	}
	return 1;
}
//finds out what to give user for the error
char* errorCheck(char*string){
	// NOOPN EXIST OPEND NEXIST NOTMT EMPTY WHAT?
	if(strcmp(string, "ER:NOOPN")==0){
		return "No box currently open\n";
	}
	if(strcmp(string, "ER:EXIST")==0){
		return "Same type already exists\n";
	}
	if(strcmp(string, "ER:OPEND")==0){
		return "Target is currently open by another user\n";
	}
	if(strcmp(string, "ER:NOTMT")==0){
		return "Cannot delete non-empty box\n";
	}
	if(strcmp(string, "ER:EMPTY")==0){
		return "Currennt box is empty\n";
	}
	if(strcmp(string, "ER:NEXST")==0){
		return "Target does not exist\n";
	}
	else{
		return "Your command was malformed in some way,\n type 'help' for a list of valid commands\n";
	}
}


int main (int argc, char* argv[]){
	if (argc < 3){ //no hostname/IP address, missing port, or missing both
		printf("MISSING ARGUMENTS\n");
	}
	//create socket
	int socketfd = socket(AF_INET, SOCK_STREAM, 0); //not sure if correct parameters
	if (socketfd < 0){
		printf("SOCKET COULD NOT BE CREATED\n");
	}
	int portno = atoi(argv[2]);
	struct hostent* serverIPAddress = gethostbyname(argv[1]);
	struct sockaddr_in serverAddressInfo;
	bzero((char *)&serverAddressInfo, sizeof(serverAddressInfo));
	serverAddressInfo.sin_family = AF_INET;
	serverAddressInfo.sin_port = htons(portno);
	bcopy((char*)serverIPAddress->h_addr, (char*)&serverAddressInfo.sin_addr.s_addr, serverIPAddress->h_length);
	
	i = 0;
	char *cmd;
	while (i<3){
		if (connect(socketfd, (struct sockaddr*)&serverAddressInfo, sizeof(serverAddressInfo)) == 0){
			sendCmd("HELLO", socketfd);
			cmd = readCmd(socketfd);
			if(strcmp(cmd,"HELLO DUMBv0 ready!")!=0){
				printf("something went horribly wrong, dear God\n");
				return 0;
			}
			free(cmd);
			break;
		}
		printf("Connection failed '%d' times", i);
		i++;
	}
	//all the stuff for being connected with the server
	//should only break after a quit
	while(1){
		printf(">");
		cmd = createInput();
		char *newCmd;
		char*serverMessage;
		if(strcmp(cmd, "help")==0){
			helpMe();
			free(cmd);
			continue;
		}
		if(strcmp(cmd, "quit")==0){
			free(cmd);
			sendCmd("GDBYE", socketfd);
			//this probably needs to be changed
			if (connect(socketfd, &serverAddressInfo, sizeof(serverAddressInfo)) != 0){
				//need to send GDBYE prompt to server
				printf("Succesfully diconnected \n");
				break;
			}
		}
		if(strcmp(cmd, "put")==0){
			free(cmd);
			printf("Okay, put what message?\n");
			printf("put:>  ");
			
			cmd = createInput();//now the user' message
			//message to be sent to server is PUT!'[int]'!msg
			int messageSize = strlen(cmd)+1;//length of message
			int lengthOfmessageSize = strlen(itoa(messageSize));//length of ^ as a string
			int newSize = (lengthOfmessageSize+1) + strlen(cmd)+7;//size of the message to be sent
			serverMessage = malloc(newSize*sizeof(char));

			//fill in the array properly
			strcpy(serverMessage,"PUTMG!");
			char*itoaStr=itoa(messageSize);
            strcpy(serverMessage+6,itoaStr);
            free(itoaStr);
            strcpy(serverMessage+6+lengthOfmessageSize,"!");
            strcpy(serverMessage+7+lengthOfmessageSize,cmd);
			free(cmd);
			printf("  serverMessage to be sent: %s\n",serverMessage);
			sendCmd(serverMessage, socketfd);
			free(serverMessage);

			cmd = readCmd(socketfd);//now the server's response
			if(strcmp(cmd, "OK!")!=0){
				printf("%s", errorCheck(cmd));
				continue;
			}
			free(cmd);
			continue;
		}
		if(strcmp(cmd,"open")==0){
			free(cmd);
			printf("Okay, open which message box?\n");
			printf("open:>  ");

			cmd = createInput();//now the user's box name
			serverMessage= malloc((7+strlen(cmd))*sizeof(char));
			strcpy(serverMessage, "OPNBX ");
			strcpy(serverMessage+6,cmd);
			free(cmd);
			printf("  serverMessage to be sent: %s\n",serverMessage);
			sendCmd(serverMessage, socketfd);
			free(serverMessage);

			cmd = readCmd(socketfd);//now the server's response
			if(strcmp(cmd, "OK!")!=0){
				printf("%s", errorCheck(cmd));
				continue;
			}
			free(cmd);
			continue;
		}
		if (strcmp(cmd, "close")==0){
			free(cmd);
			printf("Okay, close which message box?\n");
			printf("close:>  ");

			cmd = createInput();//user's box name
			char *serverMessage= malloc((7+strlen(cmd))*sizeof(char));
			strcpy(serverMessage, "CLSBX ");
			strcpy(serverMessage+6,cmd);
			free(cmd);
			printf("  serverMessage to be sent: %s\n",serverMessage);
			sendCmd(serverMessage, socketfd);
			free(serverMessage);
			
			cmd = readCmd(socketfd);//server's response
			if(strcmp(cmd, "OK!")!=0){
				printf("%s", errorCheck(cmd));
				continue;
			}
			free(cmd);
			continue;
		}
		if (strcmp(cmd, "delete")==0){
			free(cmd);
			printf("Okay, delete which message box?\n");
			printf("delete:>  ");

			cmd = createInput();//user's delete name
			serverMessage= malloc((7+strlen(cmd))*sizeof(char));
			strcpy(serverMessage, "DELBX ");
			strcpy(serverMessage+6,cmd);
			free(cmd);
			printf("  serverMessage to be sent: %s\n",serverMessage);
			sendCmd(serverMessage, socketfd);
			free(serverMessage);

			cmd = readCmd(socketfd);//server's response
			if(strcmp(cmd, "OK!")!=0){
				printf("%s", errorCheck(cmd));
				continue;
			}
			free(cmd);
			continue;
		}
		if (strcmp(cmd, "create")==0){
			free(cmd);
			printf("Okay, what would you like the name of the message box to be?\n");
			printf("create:>  ");
   
			cmd = createInput();
			printf("cmd length %d\n", strlen(cmd));
			serverMessage= malloc((7+strlen(cmd))*sizeof(char));
			strcpy(serverMessage, "CREAT ");
			strcpy(serverMessage+6,cmd);
			free(cmd);
			printf("  serverMessage to be sent: %s\n",serverMessage);
			sendCmd(serverMessage, socketfd);
			free(serverMessage);

			cmd = readCmd(socketfd);//servers reponse
			if(strcmp(cmd, "OK!")!=0){
				printf("%s", errorCheck(cmd));
				continue;
			}
			free(cmd);
			continue;
		}
		if (strcmp(cmd, "next")==0){
			free(cmd);

			cmd = malloc(6*sizeof(char));
			cmd = "NXTMG";
			cmd[6]='\0';
			sendCmd(cmd, socketfd);
			free(cmd);

			cmd=readCmd(socketfd);//server's response
			if(startsWith(cmd, "OK!")!=0){
				printf("%s", errorCheck(cmd));
				continue;
			}
			//now we gotta interpret it somehow
			else{
				int first=3, second=0;
				i=0;
				//literally just getting the number of bytes
				//will stop either at the end or if second is changed
				while(cmd[i]!='\0'||second==0){
					if(cmd[i]=='!'&&i!=3){
						second = i;
					}
					i++;
				}
				serverMessage = malloc((second-first+1)*sizeof(char));
				i=first;
				int j = 0;
				while (cmd[i]!=cmd[second+1]){
					serverMessage[j]=cmd[i];
					i++;
					j++;
				}
				serverMessage[j+1]='\0';
				printf("  serverMessage size is(as a string): %s\n",serverMessage);
				int messageSize = atoi(serverMessage);
				free(serverMessage);

				//ok now lets get the actual message
				serverMessage = malloc(messageSize*sizeof(char));
				j=0;
				for(i = second+1; cmd[i]!='\0';i++){
					serverMessage[j]=cmd[i];
					j++;
				}
				free(cmd);
				serverMessage[j+1]='\0';
				printf("  serverMessage recieved is: %s\n",serverMessage);
				free(serverMessage);
				continue;
			}
			continue;
		}	
		else{
			//any message that is most likely incorrect
			printf("  serverMessage to be sent: %s\n",cmd);
			sendCmd(cmd, socketfd);
			free(cmd);

			cmd=readCmd(socketfd);
			
			if(strcmp(cmd, "OK!")!=0){
				printf("%s", errorCheck(cmd));
			}
			free(cmd);
			continue;
		}
	}
}// end of main
