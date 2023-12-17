#ifndef SERVER_H
#define SERVER_H


#define LOGMSGLENGTH 1000
#define MAXTIMEOUTSECS 1
#define MAXTIMEOUTUSECS 0
#define MAXTIMEOUTPING 1
#define DELAYUSECS 1
#define GRAPHICSDELAY 10000
#define MAXLOGSIZE 20
#define MAXIDLECLIENT 5


typedef struct serverState{
	
	char logBuff[LOGMSGLENGTH*100];
	char addressContainer[INET_ADDRSTRLEN];
	u_int64_t serverRunning;
	u_int64_t idle;
	DListW* listOfClients;
	stackList* logs;
	int server_socket;
	struct sockaddr_in server_address;
	u_int64_t totalSent;
	u_int64_t timeActive;
	double trafficRate;
	stackList* kickedClientPtrs;
	u_int64_t dataSize;
	u_int16_t pingSize,maxNumOfClients;

}serverState;

typedef struct clientStruct{
	struct	sockaddr_in clientAddress;
	int client_socket;
	u_int32_t addrLength;
	u_int64_t numOfBytesSent;

}clientStruct;

void initEverything(u_int16_t port,char* ipaddr,u_int16_t startPingSize,u_int64_t startDataSize,u_int16_t startMaxNumOfClients);

void sigint_handler(int signal);

void cleanup(void);

#endif

