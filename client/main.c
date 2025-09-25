#include "Includes/preprocessor.h"
static const char* ping= "gimmemore!";

static int enable_tty_mode=0;
static const u_int64_t android_comp_mode_on=0;
static u_int64_t alive=1;
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


static pthread_mutex_t varMtx= PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t readyCond= PTHREAD_COND_INITIALIZER;
static pthread_mutex_t readyMtx= PTHREAD_MUTEX_INITIALIZER;
static pthread_t connectionChecker;
static pthread_t outputPrinter;
static pthread_t errPrinter;
char* outbuff=NULL;
char* errbuff=NULL;

u_int64_t goPlease=0;
int client_socket,lifeline_socket, output_socket;
int err_socket;
static struct sockaddr_in server_address;
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

static void sigint_handler(int signal){
	char line[LINESIZE];
	memset(line,0,LINESIZE);
	strncat(line,"exit",min(LINESIZE-1,strlen("exit")));
	timedSend(client_socket,line,LINESIZE,MAXTIMEOUTCMD,MAXTIMEOUTUCMD);
	acessVarMtx(&varMtx,&alive,0,0);
	acessVarMtx(&varMtx,&goPlease,0,1);
	pthread_cond_signal(&readyCond);
	pthread_join(connectionChecker,NULL);
	pthread_join(outputPrinter,NULL);
	if(!enable_tty_mode){
		pthread_join(errPrinter,NULL);
	}
	if(outbuff){
		free(outbuff);
	}
	if(errbuff&&!enable_tty_mode){
		free(errbuff);
	}
	close(client_socket);
	close(lifeline_socket);
	close(output_socket);
	if(!enable_tty_mode){
		close(err_socket);
	}
	printf("cliente a fechar!!!\n");
	exit(-1+signal*0);

}
static void sigpipe_handler(int signal){
	printf("SIG PIPE!!!\n");
	raise(SIGINT+0*signal);
	
}

static void* getOutput(void* args){

	outbuff=malloc(dataSize);
	memset(outbuff,0,dataSize);
	acessVarMtx(&varMtx,&goPlease,1,0);
	pthread_cond_signal(&readyCond);
	while(acessVarMtx(&varMtx,&alive,0,-1)){
	int numread=1;
	
	while(acessVarMtx(&varMtx,&alive,0,-1)&&((numread=timedRead(output_socket,outbuff,dataSize,MAXTIMEOUTCMD,MAXTIMEOUTUCMD))>0)){
		if(!numread){
			continue;
		}
		printf("%s",outbuff);
		memset(outbuff,0,dataSize);
	}
	}
	return args;

}
static void* getErr(void* args){
	errbuff=malloc(dataSize);
	memset(errbuff,0,dataSize);
	acessVarMtx(&varMtx,&goPlease,1,0);
 	while(acessVarMtx(&varMtx,&alive,0,-1)){
	int numread=1;
	while(acessVarMtx(&varMtx,&alive,0,-1)&&((numread=timedRead(err_socket,errbuff,dataSize,MAXTIMEOUTCMD,MAXTIMEOUTUCMD))>0)){
		if(!numread){
			continue;
		}
		fprintf(stderr,"%s",errbuff);
		memset(errbuff,0,dataSize);
	}
	}
	return args;

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
                printf("Tentando conectar a %s (Tentativa %d)!!!!!!\n",inet_ntoa(server_address.sin_addr),-numOfTries+MAXNUMBEROFTRIES+1);		
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
static void* areYouStillThere(void* args){
	int pingLength=strlen(ping);
	char buff[pingLength];
	memset(buff,0,pingLength);
	
	while(acessVarMtx(&varMtx,&alive,0,-1)){
	int numread=1;
	while(acessVarMtx(&varMtx,&alive,0,-1)&&((numread=timedSend(lifeline_socket,buff,pingLength,MAXTIMEOUTPING,MAXTIMEOUTUPING))>0)){
		timedRead(lifeline_socket,buff,pingLength,MAXTIMEOUTPING,MAXTIMEOUTUPING);
		memset(buff,0,sizeof(buff));
	}
	}
	raise(SIGINT);
	return args;

}

int main(int argc, char ** argv){

	if(argc!=3){

		printf("Utilizacao correta: arg1: ip do server.\narg2: porta\n");
		exit(-1);
	}
	char buff2[10]={0};
	initClient(atoi(argv[1]),argv[2]);
	tryConnect(&client_socket);
	tryConnect(&lifeline_socket);
	tryConnect(&output_socket);

	receiveServerOutput(client_socket,buff2,10,MAXTIMEOUTCONS,MAXTIMEOUTUCONS);
	sscanf(buff2,"%hu %d",&dataSize,&enable_tty_mode);
	printf("Datasize: %hu\nEnable tty mode: %d\n",dataSize,enable_tty_mode);
	send(client_socket,ping,strlen(ping),0);
	printf("Send de ping feito! %s\n",ping);
	
	if(!enable_tty_mode){
		tryConnect(&err_socket);
	}
	//especificar socket;
	pthread_create(&connectionChecker,NULL,areYouStillThere,NULL);
	pthread_create(&outputPrinter,NULL,getOutput,NULL);
	if(!enable_tty_mode){
		pthread_create(&errPrinter,NULL,getErr,NULL);
	}
		pthread_mutex_lock(&readyMtx);
		while(!acessVarMtx(&varMtx,&goPlease,0,-1)&&acessVarMtx(&varMtx,&alive,0,-1)){
			pthread_cond_wait(&readyCond,&readyMtx);

		}
		
		pthread_mutex_unlock(&readyMtx);
		char line[LINESIZE]={0};
		char buff[strlen(ping)+1];
		memset(line,0,LINESIZE);
		while(acessVarMtx(&varMtx,&alive,0,-1)){

			memset(line,0,LINESIZE);
			memset(buff,0,strlen(ping)+1);
			fgets(line,LINESIZE,stdin);
			if(!strncmp(line, "exit",strlen(line)-enable_tty_mode)&&((strlen(line)-enable_tty_mode)==strlen("exit"))){
				raise(SIGINT);
			}
			line[strlen(line)]=enable_tty_mode?'\n':0;
			timedSend(client_socket,line,LINESIZE,MAXTIMEOUTCMD,MAXTIMEOUTUCMD);
			//receiveServerOutput(client_socket,buff,sizeof(buff),MAXTIMEOUTCMD,MAXTIMEOUTUCMD);
		}
	        raise(SIGINT);

}

