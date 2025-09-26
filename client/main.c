#include "Includes/preprocessor.h"
static const char* ping= "gimmemore!";

static int enable_tty_mode=0;
static const u_int64_t android_comp_mode_on=0;
//static int32_t all_ready=1;
static int32_t all_alive=1;
static int32_t err_alive=0;
static int32_t out_alive=0;
static int32_t cmd_alive=0;
static int32_t ping_alive=0;
static pthread_mutex_t varMtx= PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t exitCond= PTHREAD_COND_INITIALIZER;
static pthread_mutex_t exitMtx= PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cmdCond= PTHREAD_COND_INITIALIZER;
static pthread_mutex_t cmdMtx= PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t errCond= PTHREAD_COND_INITIALIZER;
static pthread_mutex_t errMtx= PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t outCond= PTHREAD_COND_INITIALIZER;
static pthread_mutex_t outMtx= PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t pingCond= PTHREAD_COND_INITIALIZER;
static pthread_mutex_t pingMtx= PTHREAD_MUTEX_INITIALIZER;
/*static pthread_cond_t readyCond= PTHREAD_COND_INITIALIZER;
static pthread_mutex_t readyMtx= PTHREAD_MUTEX_INITIALIZER;
*/
u_int16_t dataSize;
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


static pthread_t commandPrompt;
static pthread_t cleanupCrew;
static pthread_t connectionChecker;
static pthread_t outputPrinter;
static pthread_t errPrinter;
char outbuff[LINESIZE*10]={0};
char errbuff[LINESIZE*10]={0};

int client_socket,lifeline_socket, output_socket;
int err_socket;
static struct sockaddr_in server_address;

static void sigint_handler(int signal){

        printf("sigint was called in server!!\n");
        acessVarMtx32(&varMtx,&all_alive,0,0*signal);
        pthread_cond_signal(&exitCond);
}

static void sigpipe_handler(int signal){

        printf("sigpipe was called in server!!\n");
        raise(SIGINT+signal*0);
}


static int64_t timedSend(int fd,char buff[],u_int64_t size,int secwait,int usecwait){
                int iResult;
                struct timeval tv;
                fd_set wrfds;
                FD_ZERO(&wrfds);
                FD_SET(fd,&wrfds);
                tv.tv_sec=secwait;
                tv.tv_usec=usecwait;
                iResult=select(fd+1,(fd_set*)0,&wrfds,(fd_set*)0,&tv);
                if(iResult>0){
                return write(fd,buff,size);
                }
                else if(iResult){
		return -1;
		}
		else{
		return 0;
		}

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
		else if(iResult){
		return -1;
		}
		else{
		return 0;
		}


}

static int64_t receiveServerOutput(int socket,char buff[],u_int64_t size,int secwait,int usecwait){
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
                else if(iResult){
		return -1;
		}
		else{
		return 0;
		}

}

static void initClient(int port, char* addr){

	client_socket= socket(AF_INET,SOCK_STREAM,0);
	if(client_socket==-1){
		raise(SIGINT);
	}
	lifeline_socket= socket(AF_INET,SOCK_STREAM,0);
	if(lifeline_socket==-1){
		raise(SIGINT);
	}
	output_socket= socket(AF_INET,SOCK_STREAM,0);
	if(output_socket==-1){
		raise(SIGINT);
	}
	if(!enable_tty_mode){
		err_socket= socket(AF_INET,SOCK_STREAM,0);
		if(err_socket==-1){
			raise(SIGINT);
		}
	}
	long flags=0;
       	if(!android_comp_mode_on){

		flags= fcntl(client_socket,F_GETFL);
	        flags |= O_NONBLOCK;
	        fcntl(client_socket,F_SETFD,flags);

		flags= fcntl(lifeline_socket,F_GETFL);
	        flags |= O_NONBLOCK;
	        fcntl(lifeline_socket,F_SETFD,flags);

		flags= fcntl(output_socket,F_GETFL);
	        flags |= O_NONBLOCK;
	        fcntl(output_socket,F_SETFD,flags);
	}

	
	if(!enable_tty_mode&&!android_comp_mode_on){
		flags= fcntl(err_socket,F_GETFL);
	        flags |= O_NONBLOCK;
	        fcntl(err_socket,F_SETFD,flags);
	}
	signal(SIGINT,sigint_handler);
	signal(SIGPIPE,sigpipe_handler);
	server_address.sin_family=AF_INET;
	server_address.sin_addr.s_addr = inet_addr(addr);
	server_address.sin_port= htons(port);


}

void tryConnect(int* socket){
	int success=-1;
	int numOfTries=MAXNUMBEROFTRIES;


        while(success==-1&& numOfTries){
                print_addr_aux("Tentando conectar a...\n",&server_address);
		success=connect(*socket,(struct sockaddr*)&server_address,sizeof(server_address));
               int sockerr;
		socklen_t slen=sizeof(sockerr);
		getsockopt(*socket,SOL_SOCKET,SO_ERROR,(char*)&sockerr,&slen);
		numOfTries--;
		if(sockerr==EINPROGRESS){
			
			fprintf(stderr,"Erro normal:%s\n Erro Socket%s\nNumero socket: %d\n",strerror(errno),strerror(sockerr),*socket);
			continue;

		}
		fd_set wfds;
		FD_ZERO(&wfds);
		FD_SET((*socket),&wfds);

                struct timeval t;
		t.tv_sec=MAXTIMEOUTCONS;
		t.tv_usec=MAXTIMEOUTUCONS;
		int iResult=select((*socket)+1,0,&wfds,0,&t);

		if(iResult>0&&!success&&((*socket)!=-1)){
			break;

		}
		fprintf(stderr,"Não foi possivel: %s\n",strerror(errno));
        }
        if(!numOfTries){
        printf("Não foi possivel conectar. Numero limite de tentativas (%d) atingido!!!\n",MAXNUMBEROFTRIES);
        raise(SIGINT);
        }

}
static void* getOutput(void* args){
	
	pthread_mutex_lock(&outMtx);
        while(!acessVarMtx32(&varMtx,&out_alive,0,-1)&&acessVarMtx32(&varMtx,&all_alive,0,-1)){

                pthread_cond_wait(&outCond,&outMtx);

        }
        pthread_mutex_unlock(&outMtx);
	printf("Client's output printing message channel thread alive!\n");
	memset(outbuff,0,dataSize);
	acessVarMtx32(&varMtx,&cmd_alive,1,0);
	
	pthread_cond_signal(&cmdCond);
	while(acessVarMtx32(&varMtx,&all_alive,0,-1)){
	int numread=1;
	
	while(acessVarMtx32(&varMtx,&all_alive,0,-1)&&((numread=timedRead(output_socket,outbuff,dataSize,MAXTIMEOUTCMD,MAXTIMEOUTUCMD))>0)){
		if(!numread){
			continue;
		}
		printf("%s",outbuff);
		memset(outbuff,0,dataSize);
	}
	}
	acessVarMtx32(&varMtx,&out_alive,0,0);
	printf("Client's output printing message channel thread exiting!\n");
	return args;

}
static void* getErr(void* args){
	pthread_mutex_lock(&errMtx);
        while(!acessVarMtx32(&varMtx,&err_alive,0,-1)&&acessVarMtx32(&varMtx,&all_alive,0,-1)){

                pthread_cond_wait(&errCond,&errMtx);

        }
        pthread_mutex_unlock(&errMtx);
	printf("Client's error message printing channel thread alive!\n");
	memset(errbuff,0,dataSize);
	acessVarMtx32(&varMtx,&cmd_alive,1,0);
	
	pthread_cond_signal(&cmdCond);
 	while(acessVarMtx32(&varMtx,&all_alive,0,-1)){
	int numread=1;
	while(acessVarMtx32(&varMtx,&all_alive,0,-1)&&((numread=timedRead(err_socket,errbuff,dataSize,MAXTIMEOUTCMD,MAXTIMEOUTUCMD))>0)){
		if(!numread){
			continue;
		}
		fprintf(stderr,"%s",errbuff);
		memset(errbuff,0,dataSize);
	}
	}
	acessVarMtx32(&varMtx,&err_alive,0,0);
	printf("Client's error printing message channel thread exiting!\n");
	return args;

}
static void* areYouStillThere(void* args){
	pthread_mutex_lock(&pingMtx);
	while(!acessVarMtx32(&varMtx,&ping_alive,0,-1)&&acessVarMtx32(&varMtx,&all_alive,0,-1)){
		pthread_cond_wait(&pingCond,&pingMtx);

	}
	pthread_mutex_unlock(&pingMtx);
	printf("Client's ping channel thread alive!\n");


        acessVarMtx32(&varMtx,&out_alive,1,0);
        acessVarMtx32(&varMtx,&err_alive,!enable_tty_mode,0);

        pthread_cond_signal(&outCond);
        pthread_cond_signal(&errCond);

	int pingLength=strlen(ping);
	char buff[pingLength];
	memset(buff,0,pingLength);
	
	while(acessVarMtx32(&varMtx,&all_alive,0,-1)){
	int numread=1;
	while(acessVarMtx32(&varMtx,&ping_alive,0,-1)&&((numread=timedSend(lifeline_socket,buff,pingLength,MAXTIMEOUTPING,MAXTIMEOUTUPING))>0)){
		timedRead(lifeline_socket,buff,pingLength,MAXTIMEOUTPING,MAXTIMEOUTUPING);
		memset(buff,0,sizeof(buff));
	}
	}
	acessVarMtx32(&varMtx,&ping_alive,0,0);
	printf("Client's ping channel thread exiting!\n");
        raise(SIGINT);
	return args;

}
static void* command_line_thread(void* args){
	pthread_mutex_lock(&cmdMtx);
	while(!acessVarMtx32(&varMtx,&cmd_alive,0,-1)&&acessVarMtx32(&varMtx,&all_alive,0,-1)){
		pthread_cond_wait(&cmdCond,&cmdMtx);

	}
	pthread_mutex_unlock(&cmdMtx);
	printf("Client's command sending channel thread alive!\n");
	char line[LINESIZE]={0};
	char buff[strlen(ping)+1];
	memset(line,0,LINESIZE);
	while(acessVarMtx32(&varMtx,&all_alive,0,-1)){

		memset(line,0,LINESIZE);
		memset(buff,0,strlen(ping)+1);
		fgets(line,LINESIZE,stdin);
		//line[strlen(line)-1]=enable_tty_mode?'\n':0;
		line[strlen(line)-1]='\n';
		//if(!strncmp(line, "exit",strlen(line)-enable_tty_mode)&&((strlen(line)-enable_tty_mode)==strlen("exit"))){
		if(!strncmp(line, "exit",strlen(line)-1)&&((strlen(line)-1)==strlen("exit"))){
			raise(SIGINT);
		}
		timedSend(client_socket,line,LINESIZE,MAXTIMEOUTCMD,MAXTIMEOUTUCMD);
	}
	
	acessVarMtx32(&varMtx,&cmd_alive,0,0);
	printf("Client's command sending channel thread exiting!\n");



	return args;
}

void* cleanup_crew(void*args){

        pthread_mutex_lock(&exitMtx);
        while(acessVarMtx32(&varMtx,&all_alive,0,-1)){

                pthread_cond_wait(&exitCond,&exitMtx);

        }
        pthread_mutex_unlock(&exitMtx);
        printf("Cleanup crew called in client\n");
        if(!enable_tty_mode){
                close(err_socket);
                fflush(stderr);
        }

        fflush(stdout);
        close(client_socket);
        close(lifeline_socket);
        close(output_socket);

        printf("Cleanup crew called in client. About to join threads which are online\n");
        if(!acessVarMtx32(&varMtx,&cmd_alive,0,-1)){
                printf("Reaping client cmdline thread!!\n");
                pthread_join(commandPrompt,NULL);
        }
        if(!acessVarMtx32(&varMtx,&out_alive,0,-1)){
                printf("Reaping client output writter thread!!\n");
                pthread_join(outputPrinter,NULL);
        }
        if(!enable_tty_mode){

		if(!acessVarMtx32(&varMtx,&err_alive,0,-1)){
                	printf("Reaping client error printer thread!!\n");
                	pthread_join(errPrinter,NULL);
        	}
	}
        if(!acessVarMtx32(&varMtx,&ping_alive,0,-1)){
                printf("Reaping client connection checker thread!!\n");
                pthread_join(connectionChecker,NULL);
        }
	printf("Finished cleanup in client.\n");
        close(0);
        close(1);
        close(2);
        return args;

}

int main(int argc, char ** argv){

	if(argc!=3){

		printf("Utilizacao correta: arg1: ip de server a connectar.\narg2: porta de server\n");
		exit(-1);
	}
	char buff2[10]={0};
	initClient(atoi(argv[1]),argv[2]);
	tryConnect(&client_socket);
	receiveServerOutput(client_socket,buff2,10,MAXTIMEOUTCONS,MAXTIMEOUTUCONS);
	sscanf(buff2,"%hu %d",&dataSize,&enable_tty_mode);
	printf("Datasize: %hu\nEnable tty mode: %d\n",dataSize,enable_tty_mode);
	send(client_socket,ping,strlen(ping),0);
	printf("Send de ping feito! %s\n",ping);
	
	//especificar socket;
	pthread_create(&connectionChecker,NULL,areYouStillThere,NULL);
	pthread_create(&outputPrinter,NULL,getOutput,NULL);
	if(!enable_tty_mode){
		pthread_create(&errPrinter,NULL,getErr,NULL);
	}
	tryConnect(&lifeline_socket);
	tryConnect(&output_socket);

	
	if(!enable_tty_mode){
		tryConnect(&err_socket);
	}
	
	pthread_create(&commandPrompt,NULL,command_line_thread,NULL);
        pthread_create(&cleanupCrew,NULL,cleanup_crew,NULL);


        ping_alive=1;
        pthread_cond_signal(&pingCond);
        pthread_join(cleanupCrew,NULL);

	return 0;
}

