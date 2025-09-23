#include "Includes/preprocessor.h"
static const char* ping= "gimmemore!";

static u_int64_t alive=1;
#define MAXNUMBEROFTRIES 10

#define MAXTIMEOUTSECS 1
#define MAXTIMEOUTUSECS 0

#define MAXTIMEOUTCMD (60*5)
#define MAXTIMEOUTUCMD 0

#define MAXTIMEOUTCONS (60*5)
#define MAXTIMEOUTUCONS 0

#define MAXTIMEOUTPING 1
#define MAXTIMEOUTUPING 0

#define LINESIZE 1024
#define ARGVMAX 100

#define DATASIZE 1024

static pthread_mutex_t varMtx= PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t outMtx= PTHREAD_MUTEX_INITIALIZER;
static pthread_t connectionChecker;
static pthread_t outputWritter;
static pthread_t errWritter;
int outpipe[2],errpipe[2];
int32_t server_socket,client_socket,output_socket,lifeline_socket,err_socket;
char line[LINESIZE]={0};
char  outbuff[DATASIZE]={0};
char  errbuff[DATASIZE]={0};
struct sockaddr_in server_address;
	
static void sigint_handler(int signal){

        acessVarMtx(&varMtx,&alive,0,0);
        pthread_join(connectionChecker,NULL);
	pthread_join(outputWritter,NULL);
	pthread_join(errWritter,NULL);
	close(server_socket);
	close(client_socket);
        close(lifeline_socket);
        close(output_socket);
	close(err_socket);
	close(outpipe[0]);
	close(outpipe[1]);
	close(errpipe[0]);
	close(errpipe[1]);
	fflush(stdout);
	fflush(stderr);
	close(1);
	close(2);
	exit(-1);
}

static void sigpipe_handler(int signal){

	raise(SIGINT);
}


static void initpipes(void){

if(pipe(outpipe)){

perror("Erro a criar pipe de stdout!!!\n");
raise(SIGINT);
}
dup2(outpipe[1],1);
long flags=fcntl(outpipe[0],F_GETFL);
flags |=O_NONBLOCK;
fcntl(outpipe[0],F_SETFL,flags);

if(pipe(errpipe)){

perror("Erro a criar pipe de stderr!!!\n");
raise(SIGINT);
}
dup2(errpipe[1],2);
flags=fcntl(errpipe[0],F_GETFL);
flags |=O_NONBLOCK;
fcntl(errpipe[0],F_SETFL,flags);

}
static void acceptConnection(int* socket){

		int iResult;
                struct timeval tv;
                fd_set rfds;
                FD_ZERO(&rfds);
                FD_SET(server_socket,&rfds);
                tv.tv_sec=MAXTIMEOUTCONS;
                tv.tv_usec=MAXTIMEOUTUCONS;
                iResult=select(server_socket+1,&rfds,(fd_set*)0,(fd_set*)0,&tv);
        if(iResult>0){
 
		*socket=accept(server_socket,NULL,NULL);
	}
	else{
		raise(SIGINT);
	}
	printf("Coneccao de %s!!!!!!\n",inet_ntoa(server_address.sin_addr));
	
}
static void initServer(int port){


	
	server_socket= socket(AF_INET,SOCK_STREAM,0);
	if(server_socket==-1){
		raise(SIGINT);
	}
	signal(SIGINT,sigint_handler);
	signal(SIGPIPE,sigpipe_handler);
	fcntl(server_socket,F_SETFL,O_NONBLOCK);
	server_address.sin_family=AF_INET;
	server_address.sin_port= htons(port);
	server_address.sin_addr.s_addr = INADDR_ANY;
	socklen_t len;
	
	bind(server_socket,(struct sockaddr*) &server_address,sizeof(server_address));

        listen(server_socket,3);

}
static int64_t receiveClientInput(int socket,char buff[],u_int64_t size,int secwait,int usecwait){
                int iResult;
                struct timeval tv;
                fd_set rfds;
   	        FD_ZERO(&rfds);
                FD_SET(socket,&rfds);
                tv.tv_sec=secwait;
                tv.tv_usec=usecwait;
		iResult=select(socket+1,&rfds,(fd_set*)0,(fd_set*)0,&tv);
                if(iResult>0){
                return recv(socket,buff,size,0);
                }
                return -1;
}
static int64_t timedRead(int fd,char buff[],u_int64_t size,int secwait,int usecwait){
                int iResult;
                struct timeval tv;
                fd_set rfds;
   	        FD_ZERO(&rfds);
                FD_SET(fd,&rfds);
                tv.tv_sec=secwait;
                tv.tv_usec=usecwait;
		iResult=select(fd+1,&rfds,(fd_set*)0,(fd_set*)0,&tv);
                if(iResult>0){
                return read(fd,buff,size);
                }
                return -1;
}

static int makeargv(char *s, char *argv[ARGVMAX]) {
    int ntokens = 0;

    if (s == NULL || argv == NULL || ARGVMAX == 0)
        return -1;
    argv[ntokens] = strtok(s, " \t\n");
    while ((argv[ntokens] != NULL) && (ntokens < ARGVMAX)) {
        ntokens++;
        argv[ntokens] = strtok(NULL, " \t\n");
    }
    argv[ntokens] = NULL; // it must terminate with NULL
    return ntokens;
}

static void* areYouStillThere(void* args){
	
        int pingLength=strlen(ping);
	char buff[strlen(ping)];
        memset(buff,0,pingLength);
	
	receiveClientInput(lifeline_socket,buff,pingLength,MAXTIMEOUTPING,MAXTIMEOUTUPING);
	while(acessVarMtx(&varMtx,&alive,0,-1)){

		send(lifeline_socket,ping,strlen(ping),0);
		int status=receiveClientInput(lifeline_socket,buff,strlen(ping),MAXTIMEOUTPING,MAXTIMEOUTUPING);
                if(status<0){
                        raise(SIGINT);
                }
		memset(buff,0,pingLength);



	}
	return NULL;

}
static void* writeOutput(void* args){
	while(acessVarMtx(&varMtx,&alive,0,-1)){
		char buff[strlen(ping)];
		int numread=1;
		while((numread=timedRead(outpipe[0],outbuff,DATASIZE,MAXTIMEOUTSECS,MAXTIMEOUTUSECS))>0){
		send(output_socket,outbuff,DATASIZE,0);
		receiveClientInput(output_socket,buff,sizeof(buff),MAXTIMEOUTSECS,MAXTIMEOUTUSECS);
		memset(outbuff,0,DATASIZE);
		}
		
		

	}
	return NULL;



}
static void* writeErr(void* args){
	while(acessVarMtx(&varMtx,&alive,0,-1)){
		char buff[strlen(ping)];
		int numread=1;
		
		while((numread=timedRead(errpipe[0],errbuff,DATASIZE,MAXTIMEOUTSECS,MAXTIMEOUTUSECS))>0){
		send(err_socket,errbuff,DATASIZE,0);
		receiveClientInput(err_socket,buff,sizeof(buff),MAXTIMEOUTSECS,MAXTIMEOUTUSECS);
		memset(errbuff,0,DATASIZE);
		}
		

	}
	return NULL;



}
void setupConnections(void){


	acceptConnection(&client_socket);
 
	acceptConnection(&lifeline_socket);
	
	acceptConnection(&output_socket);
 	
	acceptConnection(&err_socket);
 	long flags= fcntl(client_socket,F_GETFL);
	flags |= O_NONBLOCK|O_ASYNC;
        fcntl(client_socket,F_SETFD,flags);
	
	flags= fcntl(lifeline_socket,F_GETFL);
	flags |= O_NONBLOCK;
        fcntl(lifeline_socket,F_SETFD,flags);
	
	flags= fcntl(output_socket,F_GETFL);
	flags |= O_NONBLOCK;
        fcntl(output_socket,F_SETFD,flags);

	flags= fcntl(err_socket,F_GETFL);
	flags |= O_NONBLOCK;
        fcntl(err_socket,F_SETFD,flags);
	
	char sizes[10]={0};
	snprintf(sizes,10,"%hu",DATASIZE);
	send(client_socket,sizes,10,0);
	char buff[strlen(ping)];
	memset(buff,0,strlen(ping));
	receiveClientInput(client_socket,buff,strlen(ping),MAXTIMEOUTCONS,MAXTIMEOUTUCONS);
	

}
int main(int argc, char ** argv){
	
	if(argc!=2){

		printf("arg1: porta do server\n");
		exit(-1);
	}

	initServer(atoi(argv[1]));
	
	setupConnections();

	initpipes();
	
	pthread_create(&connectionChecker,NULL,areYouStillThere,NULL);
	
	pthread_create(&outputWritter,NULL,writeOutput,NULL);

	pthread_create(&errWritter,NULL,writeErr,NULL);

	//receber e armazenar dados recebidos
        
	
	
	while(acessVarMtx(&varMtx,&alive,0,-1)){
	//perror("Gormitis de fogo\n");
	while(receiveClientInput(client_socket,line,LINESIZE,MAXTIMEOUTCMD,MAXTIMEOUTUCMD)>0){
	char buff[strlen(ping)+1];
	memset(buff,0,strlen(ping)+1);
	memcpy(buff,ping,strlen(ping));
	
	char* args[ARGVMAX]={0};
	makeargv(line,args);
	fflush(stdout);
	fflush(stderr);
	int pid= fork();
	switch(pid){
		case -1:
			exit(-1);
		case 0:
			execvp(args[0], args);
			exit(-1);
		default:
			while(!wait(NULL));
		break;

	}
	send(client_socket,buff,sizeof(buff),0);	
	}
	}
	raise(SIGINT);
	return 0;
}

