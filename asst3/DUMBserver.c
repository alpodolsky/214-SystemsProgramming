#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>

int boxCount = 0;
struct input {
	char* cmd;
	char* arg;
	int argLength;
};
struct msgBox {
	int isOpen;
	int hasMessages;
	char* boxName;
	//need pointer to queue containing messages
	struct queue* msgQueue;
	struct msgBox* nxtBox; // pointer to next msgBox
	
};

struct threadInfo{
	int hasBoxOpen; //1 = has box open, 0 = does not have box open
	struct msgBox* openedBox;
};

struct qNode{
	char* msg;
	struct qNode* next;
};

struct queue{
	struct qNode *front, *rear;
};
struct qNode* newNode(char* toAdd){
	struct qNode* temp = (struct qNode*)malloc(sizeof(struct qNode));
	temp->msg = toAdd;
	temp->next = NULL;
	return temp;
}

struct queue* createQueue(){
	struct queue* q = (struct queue*)malloc(sizeof(struct queue));
	q->front = NULL;
	q->rear = NULL;
	return q;
}

void enQueue(struct queue* q, char* toAdd){
	struct qNode* temp = newNode(toAdd);
	//if queue is empty
	if (q->rear == NULL){
		q->front = temp;
		q->rear = temp;
		return;
	}
	//not emptu
	q->rear->next = temp;
	q->rear = temp;
}

struct qNode* deQueue(struct queue* q){
	if (q->front == NULL){ //queue is empty
		return NULL;
	}
	
	struct qNode* temp = q->front;
	free(temp);
	q->front = q->front->next;

	if (q->front == NULL){
		q->rear = NULL;
	}
	return temp;
}

struct msgBox* head=NULL,*tail=NULL;
struct msgBox* nullBox;//this was going to eventually be a problem the way it was made before
void printArray(char* arr, int size){
	int i = 0;
	for (i = 0; i < size; i++){
		printf("%c",arr[i]);
	}
	printf("'\n");
}

char* itoa(int num){
	char str[12];
	int i;
	for (i=0; i < 12; i++){
		str[i] = '\0';
	}
	sprintf(str, "%d", num);
	for(i = 0; str[i] != '\0'; i++);
	int numStrLen = i+1;
	char* numStr = malloc(numStrLen*sizeof(char));
	for(i=0; str[i]!='\0';i++){
		numStr[i]=str[i];
	}
	numStr[i] = '\0';
	return numStr;
}
char null[]="\n";
//handles any related errors itself, dynamically allocated
void createMsgBox(int socfd, char* name){
	char okmsg[] = "OK!";
	//check if box with that name already exists
	int i = 0;
	struct msgBox* ptr =head;
	if(strlen(name)<4&&isdigit(name[0])!=0){
		sendMsg(socfd,"ER:WHAT?",9);
		return;
	}
	if(head == NULL){
		//means list is empty so create the box
		head = (struct msgBox*)malloc(sizeof(struct msgBox));
		head->isOpen = 0;
		head->boxName = name;
		head->nxtBox = tail;
		head->msgQueue = NULL;
		boxCount++;
		sendMsg(socfd, "OK!", 4);
		return;
	}
	//if its not null then there is at least one box in there
	while(ptr->nxtBox!=tail){//this stops on tail keep that in mind
		//check if head has same name
		//if it does send back EXIST error
		if(strcmp(ptr->boxName,name)==0){
			sendMsg(socfd, "ER:EXIST",9);
			return;
		}
		ptr=ptr->nxtBox;
	}
	if(strcmp(ptr->boxName,name)==0){
		sendMsg(socfd, "ER:EXIST",9);
		return;
	}
	//we are on tail then we hit null(i think?)
	//so create the new box
	struct msgBox* temp = (struct msgBox*)malloc(sizeof(struct msgBox));
	temp->isOpen = 0;
	temp->boxName = name;
	temp->nxtBox = tail;
	temp->msgQueue = NULL;
	//inserts temp between head and tail 
	ptr->nxtBox = temp;
	boxCount++;
	sendMsg(socfd, "OK!", 4);
	return;
}
//just finds the box or returns a 'nullbox', error handled by calling function
struct msgBox* getMsgBox(char* name){
	int i = 0;
	struct msgBox* ptr =head;
	if (ptr == NULL){
		//may change this to just return NEXST
		printf("there are no boxes\n");
		return nullBox;
	}
	while(ptr->nxtBox!=tail){
		if(strcmp(ptr->boxName,name)==0){
			return ptr;
		}
		ptr = ptr->nxtBox;
	}
	if(strcmp(ptr->boxName,name)==0){
		return ptr;
	}
	else{
		printf("there are no boxes at end\n");
		return nullBox;
	}
}
//will return either NULL or the opened box, takes care of sending message
struct msgBox openBox(int socfd, char* name){ //need this to return the box it just opened
	//check that box exists
	struct msgBox boxToOpen = *(getMsgBox(name));
	char cmpTo[] = "!";
	if (strcmp(boxToOpen.boxName, cmpTo) == 0){
		char err[] = "ER:NEXST";
		sendMsg(socfd, err, sizeof(err));
		struct msgBox nullBox;
		nullBox.boxName = "!";
		return nullBox;
	}
	//check if box is already open
	else if (boxToOpen.isOpen == 1){
		printf("in er:opend if\n");
		char err[] = "ER:OPEND";
		sendMsg(socfd, err, sizeof(err));
		struct msgBox nullBox;
		nullBox.boxName = "!"; 
		return nullBox;
	}
	//open box
	else{
		boxToOpen.isOpen = 1;
		char okmsg[]  = "OK!";
		sendMsg(socfd, okmsg, sizeof(okmsg));
		return boxToOpen;
	}
	
}
//does not handle errors, only deletes appropriate box
void deleteMsgBox(struct msgBox* boxToDelete){
	//need to find its prev box because we already have built in access to next
	struct msgBox* temp;
	temp = head;
	struct msgBox*prev = NULL;
	while(strcmp(temp->boxName,boxToDelete->boxName)!=0){
		prev = temp;
		temp = temp->nxtBox;
	}
	//special case of its the only one in the list
	if(prev==NULL&&temp->nxtBox==NULL){
		head =NULL;
		free(temp);
		boxCount--;
		return;
	}
	//kin of the case above but not rly
	if(prev==NULL&&temp->nxtBox!=NULL){
		head = temp->nxtBox;
		free(temp);
		boxCount--;
		return;
	}
	//point the box before boxToBeDelete to the box after it
	struct msgBox*next = temp->nxtBox;
	prev->nxtBox=next;

	boxCount--;
	free(temp);
	return;
}
//i think you have to pass the thisInput arg to here inplace of messg
//probably wont fix the error but meh idk anymore
void putMsg(int socfd, char* messg, struct msgBox* openBox){
	printf("in putmsg\n");
	openBox->msgQueue = createQueue();
	enQueue(openBox->msgQueue, messg);
	char ok[] = "OK!"; //change this
	sendMsg(socfd, ok, sizeof(ok));
	return;
}

void nxtMsg(int socfd, struct msgBox* openBox){
	//need to send client the message and then remove the message from queue
	struct qNode* q = (struct qNode*)malloc(sizeof(struct qNode));
	q  = deQueue(openBox->msgQueue);
	char* msg = q->msg;
	printf("%s\n", msg);
	//send msg back
	char*messageToSend = malloc((4+strlen(msg)+sizeof(string(msg))*sizeof(char)));
	int messageSizeAsString = itoa(msg);
	strcpy(messageToSend, "OK!");
	strcpy(messageToSend+3, messageSizeAsString);
	strcpy(messageToSend+3+sizeof(messageSizeAsString)+1, "!");
	strcpy(messageToSend+3+sizeof(messageSizeAsString)+2,msg);
	
	sendMsg(socfd, messageToSend, sizeof(messageToSend));
	
}


int sendMsg(int socfd, char* msg, int msgLength){
	printf("message to be sent is: ");
	printArray(msg, msgLength);
	int i = 0;
	while (i < msgLength){
		int bytesSent = write(socfd, msg+i, msgLength-i);
		if (bytesSent == -1){
			printf("WRITE ERROR\n");
			return -1;
		}
		printf("was able to write '%d' bytes\n", bytesSent);
		i += bytesSent;
		printf("have sent a total of '%d' bytes\n", i);
	}
	return 0;
}
struct input readInput(int socfd){
	//read first 6 bytes
	struct input thisInput;
	int bufLength = 6;
	char* input = (char*)malloc(bufLength*sizeof(char));
	int i = 0;
	printf("trying to read\n");
	while (i < bufLength){
		int bytesRead = read(socfd, input+i, bufLength-i);
		if (bytesRead == -1){
			printf("READ ERROR\n");
			break;
		}
		else if (bytesRead == 0){ //client has disconnected, call quit
			printf("client disconnected 1\n");
			char* quit = "GDBYE";
			thisInput.cmd = quit;
			return thisInput;
		}
		printf("was able to read '%d' bytes\n", bytesRead);
		i += bytesRead;
		printf("have read a total of '%d' bytes\n", i);
	}
	//remove 6th byte from cmd
	int p = 0;
	char* finalCmd = (char*)malloc((bufLength)*sizeof(char));
	while(p < bufLength){
		finalCmd[p] = input[p];
		p++;
	}
	printf("command was: ");
	finalCmd[bufLength-1]='\0';
	printArray(finalCmd, bufLength);
	thisInput.cmd = finalCmd;
	printf("%d'", i);
	printf("'%c'\n", input[i]);
	if (input[i-1] == ' '||input[i-1]=='\0'){
		printf("in if\n");
		bufLength = 32;
		char* withArg = (char*)realloc(input, bufLength);
		//read 1 char at a time until null term or greater than 32
		while (i < bufLength && withArg[i-1] != '\0'){
			int bytesRead = read(socfd, withArg+i, 1);
			if (bytesRead == -1){
				printf("READ ERROR\n");
				break;
			}
			else if (bytesRead == 0){ //client has disconnected, we need to stop reading -> quit
				printf("client disconnected\n");
				char* quit = "GDBYE";
				thisInput.cmd = quit;
				return thisInput;
			}
			printf("was able to read '%d' bytes \n", bytesRead);
			i += bytesRead;
			printf("have read a total of '%d' bytes\n", i);

			if (i == 32 && withArg[i-1] != '\0'){
				printf("Argument is too long\n"); //change to error
				char err[] = "ER:WHAT?";
				printf("size of msg: '%d'\n", sizeof(err));
				sendMsg(socfd, err, sizeof(err));
			}
		}
		thisInput.argLength = i;
		int k = 0;
		int j = 6;
		char* arg = (char*)malloc(i*sizeof(char));
		for (k = 0; j < i; k++){
			arg[k] = withArg[j];
			j++;
		}
		thisInput.arg = arg;
		printArray(withArg, i);
	}
	else if (input[i-1] == '!'){
		//read until next '!' to get length of message
		printf("In '!' if statement \n");
		char* withMsgLeng;
		int firstIndex = i;
		int currLength = bufLength;
		do {
			withMsgLeng = (char*)realloc(input, currLength);
			int bytesRead = read(socfd, withMsgLeng+i, 1);
			if (bytesRead == -1){
				printf("READ ERROR\n");
				break;	
			}
			else if (bytesRead == 0){ //client disconnect
				char* quit = "GDBYE";
				thisInput.cmd = quit;
				printf("client has disconnected\n");
				return thisInput;
			}
			printf("Was able to read '%d' bytes\n", bytesRead);
			i += bytesRead;
			printf("Have read a total of '%d' bytes\n", i);
			currLength++;
		}while(withMsgLeng[i-1] != '!');
		int secondIndex = i-1;
		int length = secondIndex-firstIndex;
		char* msgLength = (char*)malloc(length*sizeof(char));
		int k = 0;
		while (firstIndex < secondIndex){
			msgLength[k] = withMsgLeng[firstIndex];
			firstIndex++;
			k++; 
		}	
		
		printf("Array containing msg length: ");
		printArray(msgLength, length);
		int msgLengAsInt = atoi(msgLength);
		printf("message length: '%d'\n", msgLengAsInt);
		thisInput.argLength = msgLengAsInt;
		
		//continue reading to get message
		char* wholeInput = (char*)realloc(withMsgLeng, currLength+thisInput.argLength);
		int msgStart = i;
		while(i < (currLength+thisInput.argLength)){
			int bytesRead = read(socfd, wholeInput+i, (currLength+thisInput.argLength)-i);
			if (bytesRead == -1){
				printf("READ ERROR\n");
			}
			else if (bytesRead == 0){
				printf("client disconnected\n");
				char* quit = "GDBYE";
				thisInput.cmd = quit;
				return thisInput;
			}
			printf("was able to read '%d' bytes\n", bytesRead);
			i += bytesRead;
			printf("have read a total of '%d' bytes\n", i);
		}
		printArray(wholeInput, currLength+thisInput.argLength);
		char* message = (char*)malloc(thisInput.argLength*sizeof(char));
		k = 0;
		while (k < i){
			message[k] = wholeInput[msgStart];
			msgStart++;
			k++;
		}
		printf("Message is: ");
		printArray(message, thisInput.argLength);
		thisInput.arg = message;	
	}
	else{
		//is unknown command
		printf("client supplied unknown command, setting cmd appropriately\n");
		char err[] = "ER:WHAT?";
		thisInput.cmd = err;
	}
	return thisInput;
}

void quit(int socfd){
	//close any open message box
	close(socfd);
	pthread_exit(NULL);
}

int interpretCmd(struct input thisInput, int socfd, struct threadInfo* thisThreadInfo){
	char* goodbye = "GDBYE";
	char* hello = "HELLO";
	char* create = "CREAT";
	char* openbox = "OPNBX";
	char* nextmsg = "NXTMG";
	char* putmsg = "PUTMG";
	char* deletebox = "DELBX";
	char* closebox = "CLSBX";

	char* cmd = thisInput.cmd;
	if (strcmp(cmd, goodbye) == 0){
		printf("client called the 'quit' command\n");
		//call quit fnction
		quit(socfd);
		return 1;
	}
	else if (strcmp(cmd, hello) == 0){
		printf("client called the 'hello' command\n");
		char helloResponse[] = "HELLO DUMBv0 ready!";
		sendMsg(socfd, helloResponse, sizeof(helloResponse));
		return 0;
	}
	else if (strcmp(cmd, create) == 0){
		printf("client called the 'create' command\n");
		createMsgBox(socfd, thisInput.arg);
		return 0;
	}
	else if (strcmp(cmd, openbox) == 0){
		printf("client called the 'openbox' command\n");
		printf("current values of thread info struct: \n");
		printf("has open: '%d'\n", thisThreadInfo->hasBoxOpen);
		//check if user has box open prior to calling openbox()
		if(thisThreadInfo->hasBoxOpen == 1){
			//if they do, error
			printf("In EC error\n");
			sendMsg(socfd, "ER:EXTR1", 9);
			return 0;
		}
		else{
			struct msgBox openReturn;
			openReturn = openBox(socfd, thisInput.arg);
			if(strcmp(openReturn.boxName, "!") == 0){
				return 0;
			}
			else{
				//upon successful open box, change struct to reflect that they have a box open
				printf("setting threadinfo stats upon successful open\n");
				thisThreadInfo->openedBox = &openReturn;
				thisThreadInfo->hasBoxOpen = 1;
				printf("has open is now: '%d'\n", thisThreadInfo->hasBoxOpen);
				return 0;	
			}	
		}
	}
	else if (strcmp(cmd, nextmsg) == 0){
		printf("client called the 'nextmsg' command\n");
		if (thisThreadInfo->hasBoxOpen == 0){
			char err[] = "ER:NOOPN";
			sendMsg(socfd, err, sizeof(err));
			return 0;
		}
		else{
			nxtMsg(socfd, thisThreadInfo->openedBox);
		}
	}
	else if (strcmp(cmd, putmsg) == 0){
		printf("client called the 'putmsg' command\n");
		//need to create a method for this
		if (thisThreadInfo->hasBoxOpen == 0){
			//send not open err
			char err[] = "ER:NOOPN";
			sendMsg(socfd, err, sizeof(err));
			return 0;
		}
		else{
			putMsg(socfd, thisInput.arg, thisThreadInfo->openedBox);
			return 0;
		}
	}
	else if (strcmp(cmd, deletebox) == 0){
		printf("client called the 'deletebox' command\n");
		struct msgBox* boxToDelete=getMsgBox(thisInput.arg);
		if(strcmp(boxToDelete->boxName,"!")==0){
			sendMsg(socfd, "ER:NEXST",9);
			return 0;
		}
		if(boxToDelete->isOpen ==1){
			sendMsg(socfd,"ER:OPEND",9);
			return 0;
		}
		if(boxToDelete->hasMessages==1){
			sendMsg(socfd,"ER:NOTMT",9);
			return 0;
		}
		else{
			deleteMsgBox(boxToDelete);
			sendMsg(socfd,"OK!",4);
			return 0;
		}
	}
	else if (strcmp(cmd, closebox) == 0){
		printf("client called the 'closebox' command\n");
		//also check if a box is even open
		if(thisThreadInfo->hasBoxOpen==0){
			sendMsg(socfd, "ER:NOOPN",9);
			return 0;
		}
		//should also check the correct box is being closed, else errr
		if(strcmp(thisThreadInfo->openedBox->boxName,thisInput.arg)!=0){
			sendMsg(socfd, "ER:EXTR2",9);
			return 0;
		}
		//need to change status of the box first
		//upon close need to change thread to no box open
		else{
			thisThreadInfo->openedBox->isOpen = 0;
			thisThreadInfo->hasBoxOpen = 0;
			thisThreadInfo->openedBox =NULL;
			sendMsg(socfd,"OK!",4);
			return 0;
		}	
	}
	else{ //don't know the command, send back WHAT?
		printf("client sent an unknown command\n");
		char err[] = "ER:WHAT?";
		sendMsg(socfd, err, sizeof(err));
		return;
	}	
}

void* communicate(void* socfd){
    struct threadInfo thisThreadInfo;
    thisThreadInfo.hasBoxOpen = 0;
    thisThreadInfo.openedBox = NULL;
        int* clientfd = (int*)socfd;
        int client = *clientfd;
    while(1){
       	struct input thisIn = readInput(client);
	int i;
        i = interpretCmd(thisIn, client, &thisThreadInfo);
		if (i == 1){
			break;
		}
    }

}

int main (int argc, char* argv[]){
	if (argc < 2){ //no port number given
		printf("PORT ERROR\n"); //change to what error franny wants
	}
	nullBox=(struct msgBox*)malloc(sizeof(struct msgBox*));
	char nullBoxName[] = "!";
	nullBox->boxName = nullBoxName;
	//create socket
	int socketfd = socket(AF_INET, SOCK_STREAM, 0); //not sure if these are the right parameters
	if (socketfd < 0) { //socket could not be created
		printf("SOCKET COULD NOT BE CREATED\n"); //change to what franny wants
	}
	int portno = atoi(argv[1]);
	printf("portno: '%d'\n", portno);
	
	struct sockaddr_in serverAddressInfo, clientAddressInfo;
	bzero((char*)&serverAddressInfo, sizeof(serverAddressInfo));
	serverAddressInfo.sin_family = AF_INET;
	serverAddressInfo.sin_addr.s_addr = INADDR_ANY;
	serverAddressInfo.sin_port = htons(portno);

	//bind address to socket
	if (bind(socketfd,(struct sockaddr*)&serverAddressInfo, sizeof(serverAddressInfo)) < 0){
		printf("BINDING ERROR\n");
	}
	while(1){
		//listen
		if (listen(socketfd, 5) != 0){
			printf("LISTEN FAILED\n"); //change to whatever franny wants
		}
		else{
			printf("server listening\n");
		}	

		int clientlen = sizeof(clientAddressInfo);
		int clientfd = accept(socketfd,(struct sockaddr*)&clientAddressInfo, &clientlen);
		if (clientfd < 0){
			printf("ACCEPT ERROR\n");
		}
		else{
			printf("server accepted client\n");
		}
	
		//make new thread
		pthread_t thread_id;
		int t = pthread_create(&thread_id, NULL, communicate, &clientfd);
		if (t != 0){
			printf("error creating thread\n");
		}
	}
		
}//end of main