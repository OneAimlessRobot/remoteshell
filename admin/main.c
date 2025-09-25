#include "Includes/preprocessor.h"
socklen_t socklenvar[2]= {sizeof(struct sockaddr),sizeof(struct sockaddr_in)};

static const char* ping= "gimmemore!";
static int enable_tty_mode=0;
static u_int64_t alive=1;
//static u_int64_t exiting=0;
static const u_int64_t android_comp_mode_on=0;
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
static pthread_mutex_t exitMtx= PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t exitCond= PTHREAD_COND_INITIALIZER;
//static pthread_mutex_t exitMtx2= PTHREAD_MUTEX_INITIALIZER;
//static pthread_cond_t exitCond2= PTHREAD_COND_INITIALIZER;
//static pthread_mutex_t exitMtx3= PTHREAD_MUTEX_INITIALIZER;
//static pthread_cond_t exitCond3= PTHREAD_COND_INITIALIZER;
static pthread_t connectionChecker;
static pthread_t outputWritter;
static pthread_t errWritter;
static pthread_t commandPrompt;
int outpipe[2];
int errpipe[2];
int inpipe[2];

int master_fd=-1;
int slave_fd=-1;
int pid=0;
int32_t server_socket,client_socket,output_socket,lifeline_socket;
int err_socket;
char line[LINESIZE]={0};
char  outbuff[DATASIZE]={0};
char  errbuff[DATASIZE]={0};
struct sockaddr_in server_address;

static void print_addr_aux(char* prompt,struct sockaddr_in* addr){
         printf("%s\nEndereço: \n%s Porta: %u\n",prompt,inet_ntoa(addr->sin_addr),ntohs(addr->sin_port));

}

static void print_sock_addr(int socket){

        struct sockaddr_in addr={0};

        getsockname(socket,(struct sockaddr*)(&addr),&socklenvar[0]);

        print_addr_aux("O endereço desta socket é:\n",&addr);

}
static void init_addr(struct sockaddr_in* addr, char* hostname_str,uint16_t port){

         addr->sin_family=AF_INET;
        struct addrinfo *addr_info_struct=NULL;
        int error=0;
         if((error=getaddrinfo(hostname_str, NULL, NULL, &addr_info_struct))){
                printf("Erro a obter address a partir de hostname!!\nErro: %s\n",gai_strerror(error));
                if(addr_info_struct){
                        freeaddrinfo(addr_info_struct);
                }
        }

        memcpy(addr,(struct sockaddr_in*)addr_info_struct->ai_addr,sizeof(struct sockaddr_in));

        addr->sin_port= htons(port);

        print_addr_aux("ip address: ",addr);

        if(addr_info_struct){
                freeaddrinfo(addr_info_struct);
        }
}

	
static void sigint_handler(int signal){

	acessVarMtx(&varMtx,&alive,0,0*signal);
	pthread_cond_signal(&exitCond);
}

static void sigpipe_handler(int signal){

	printf("sigpipe was called in server!!\n");
	raise(SIGINT+signal*0);
}

static void initpipes(void){

if(enable_tty_mode){

	return;

}
if(pipe(outpipe)){
perror("Erro a criar pipe de stdout!!!\n");
raise(SIGINT);
}
dup2(outpipe[1],1);
long flags=0;
if(!android_comp_mode_on){

	flags=fcntl(outpipe[0],F_GETFL);
	if(flags<0){

	perror("Erro a criar pipe de stdout!!!\nFlags were not retrieved\n");
	raise(SIGINT);

	}

	flags |=O_NONBLOCK;
	fcntl(outpipe[0],F_SETFL,flags);

	if(flags<0){

	perror("Erro a criar pipe de stdout!!!\nFlags were not set!!!!\n");
	raise(SIGINT);

	}


	if(pipe(inpipe)){
	perror("Erro a criar pipe de stdout!!!\n");
	raise(SIGINT);
}
}
dup2(inpipe[1],0);
if(!android_comp_mode_on){
	long flags=fcntl(inpipe[0],F_GETFL);
	if(flags<0){

	perror("Erro a criar pipe de stdout!!!\nFlags were not retrieved\n");
	raise(SIGINT);

	}


	flags |=O_NONBLOCK;
	fcntl(inpipe[0],F_SETFL,flags);

	if(flags<0){

	perror("Erro a criar pipe de stdout!!!\nFlags were not set!!!!\n");
	raise(SIGINT);

}
}

if(pipe(errpipe)){

perror("Erro a criar pipe de stderr!!!\n");
raise(SIGINT);
}
dup2(errpipe[1],2);
if(!android_comp_mode_on){

	flags=fcntl(errpipe[0],F_GETFL);
	flags |=O_NONBLOCK;
	fcntl(errpipe[0],F_SETFL,flags);
}

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
		printf("No client connected!!!!\n");
		return;
	}
	printf("Coneccao de %s!!!!!!\n",inet_ntoa(server_address.sin_addr));
	
}
static void initServer(char* address,int port){


	
	server_socket= socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(server_socket==-1){
		perror("Nao foi possivel criar server socket!!!\n");
		raise(SIGINT);
	}
	int ptr=1;
	struct linger the_linger={0,30};
        if(setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR,(char*)&ptr,sizeof(ptr))){
		perror("Erro a meter SO_REUSEADDR  na socket (setsockopt)\n");
		raise(SIGINT);
	}
	if(setsockopt(server_socket,SOL_SOCKET,SO_LINGER,(char*)&the_linger,sizeof(struct linger))){
		perror("Erro a meter SO_LINGER na socket (setsockopt)\n");
		raise(SIGINT);
	}
	signal(SIGINT,sigint_handler);
	signal(SIGPIPE,sigpipe_handler);
	if(android_comp_mode_on){
		fcntl(server_socket,F_SETFL,O_NONBLOCK);
	}
	init_addr(&server_address,address,port);
	if(bind(server_socket,(struct sockaddr*) &server_address,sizeof(server_address))){

		perror("Nao foi possivel dar bind!!!\n");
		raise(SIGINT);
	}
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
static void* areYouStillThere(void* args){
	
        int pingLength=strlen(ping);
	char buff[strlen(ping)];
        memset(buff,0,pingLength);
	while(acessVarMtx(&varMtx,&alive,0,-1)){
		char buff[strlen(ping)];
		int numread=1;
		while(acessVarMtx(&varMtx,&alive,0,-1)&&((numread=timedRead(lifeline_socket,buff,pingLength,MAXTIMEOUTPING,MAXTIMEOUTUPING))>0)){
		timedSend(lifeline_socket,(char*)ping,strlen(ping),MAXTIMEOUTPING,MAXTIMEOUTUPING);
		}
		raise(SIGINT);
		break;

	}
	printf("checker out!!!\n");
	return args;

}

static void* writeOutput(void* args){

	
	while(acessVarMtx(&varMtx,&alive,0,-1)){
		int numread=1;
		while(acessVarMtx(&varMtx,&alive,0,-1)&&((numread=timedRead(enable_tty_mode?master_fd:outpipe[0],outbuff,DATASIZE,MAXTIMEOUTSECS,MAXTIMEOUTUSECS))>=0)){
		if(!numread){
			continue;
		}
		timedSend(output_socket,outbuff,DATASIZE,MAXTIMEOUTSECS,MAXTIMEOUTUSECS);
		memset(outbuff,0,DATASIZE);
		}
		break;

	}
	printf("output writter out!!!\n");
	return args;



}

static void* writeErr(void* args){
	while(acessVarMtx(&varMtx,&alive,0,-1)){
		int numread=1;
		while(acessVarMtx(&varMtx,&alive,0,-1)&&((numread=timedRead(errpipe[0],errbuff,DATASIZE,MAXTIMEOUTSECS,MAXTIMEOUTUSECS))>=0)){
		if(!numread){
			continue;
		}
		timedSend(err_socket,errbuff,DATASIZE,MAXTIMEOUTSECS,MAXTIMEOUTUSECS);
		memset(outbuff,0,DATASIZE);
		}
		break;

	}
	printf("error writter out!!!\n");
	return args;



}
void setupConnections(void){


	acceptConnection(&client_socket);

	acceptConnection(&lifeline_socket);

	acceptConnection(&output_socket);
	long flags=0;
	if(!android_comp_mode_on){
		flags= fcntl(client_socket,F_GETFL);
		flags |= O_NONBLOCK|O_ASYNC;
	        fcntl(client_socket,F_SETFD,flags);

		flags= fcntl(lifeline_socket,F_GETFL);
		flags |= O_NONBLOCK;
	        fcntl(lifeline_socket,F_SETFD,flags);

		flags= fcntl(output_socket,F_GETFL);
		flags |= O_NONBLOCK;
	        fcntl(output_socket,F_SETFD,flags);
	}
 	char sizes[10]={0};
	snprintf(sizes,10,"%d %d",DATASIZE,enable_tty_mode);
	send(client_socket,sizes,10,0);
	char buff[strlen(ping)];
	memset(buff,0,strlen(ping));
	receiveClientInput(client_socket,buff,strlen(ping),MAXTIMEOUTCONS,MAXTIMEOUTUCONS);

	if(!enable_tty_mode){
		acceptConnection(&err_socket);
		if(!android_comp_mode_on){
			flags= fcntl(err_socket,F_GETFL);
			flags |= O_NONBLOCK;
	        	fcntl(err_socket,F_SETFD,flags);
		}
	}

}
static void* command_prompt_thread(void* args){

	char buff[strlen(ping)+1];
	while(acessVarMtx(&varMtx,&alive,0,-1)){
	memset(line,0,sizeof(line));
	while(acessVarMtx(&varMtx,&alive,0,-1)&&(receiveClientInput(client_socket,line,LINESIZE,MAXTIMEOUTCMD,MAXTIMEOUTUCMD)>=0)){

	
	memset(buff,0,strlen(ping)+1);
	memcpy(buff,ping,strlen(ping));
	if(!strncmp(line, "exit",strlen(line)-enable_tty_mode)&&((strlen(line)-enable_tty_mode)==strlen("exit"))){

		write(enable_tty_mode?master_fd:inpipe[1],"exit",strlen(line));
		printf("we got orders to exit!\n");
		kill(pid,SIGTERM);
		print_sock_addr(client_socket);
		waitpid(-1,NULL,0);
		raise(SIGINT);

	}
	write(enable_tty_mode?master_fd:inpipe[1],line,strlen(line));
	if(!enable_tty_mode){
		fflush(stdout);
		fflush(stderr);
	}
	memset(line,0,sizeof(line));
	//send(client_socket,buff,sizeof(buff),0);
	}
	}
	printf("Command prompt about to exit!\n");
	return args;

}
int main(int argc, char ** argv){
	
	if(argc!=5){

		printf("arg1: address\narg2: porta do server\narg3: shell to use\narg4: 0/1 = (dis)/(en)able tty mode (experimental)\n");
		exit(-1);
	}
	enable_tty_mode=atoi(argv[4]);
	if((enable_tty_mode<0)||(enable_tty_mode>1)){

		printf("arg4 so pode ser 0 ou 1!\n");
		exit(-1);
	}
	
	initServer(argv[1],atoi(argv[2]));
	printf("Shell name: %s\n",argv[3]);
	setupConnections();


	initpipes();

	pthread_create(&connectionChecker,NULL,areYouStillThere,NULL);

	pthread_create(&outputWritter,NULL,writeOutput,NULL);

	if(!enable_tty_mode){
		pthread_create(&errWritter,NULL,writeErr,NULL);
		char *ptrs[3];
		char arg0[LINESIZE]={0};
		char arg1[LINESIZE]={0};
		pid= fork();
		switch(pid){
			case -1:
				exit(-1);
			case 0:
				dup2(inpipe[0], STDIN_FILENO);
			        close(inpipe[0]);
			        close(inpipe[1]);
				snprintf(arg0,LINESIZE-1,"/bin/%s",argv[3]);
				ptrs[0]=arg0;
				snprintf(arg1,LINESIZE-1,"-i");
				ptrs[1]=arg1;
				ptrs[2]=NULL;
				execvp(ptrs[0], ptrs);
				exit(-1);
			default:
				close(inpipe[0]);
				break;
		}
	}
	else{
		char* ptrs[3];
		char pty_name[128]={0};
		openpty(&master_fd,&slave_fd,pty_name,NULL,NULL);
		printf("Pty name: %s\n",pty_name);
		long flags= 0;
		if(!android_comp_mode_on){
			flags=fcntl(master_fd,F_GETFL);
			flags |= O_NONBLOCK|O_SYNC;
	        	fcntl(master_fd,F_SETFD,flags);
		}
		pid=fork();
		switch(pid){
			case -1:
				exit(-1);
			case 0:
				close(master_fd);
				setsid();
				ioctl(slave_fd,TIOCSCTTY,0);
				dup2(slave_fd,0);
				dup2(slave_fd,1);
				dup2(slave_fd,2);
				close(slave_fd);
				char arg0[LINESIZE]={0};
				snprintf(arg0,LINESIZE-1,"/bin/%s",argv[3]);
				ptrs[0]=arg0;
				char arg1[LINESIZE]={0};
				snprintf(arg1,LINESIZE-1,"-i");
				ptrs[1]=arg1;
				ptrs[2]=NULL;
				login_tty(slave_fd);
				execvp(ptrs[0], ptrs);
				exit(-1);
			default:
				close(slave_fd);
				break;
		}
	}
	pthread_create(&commandPrompt,NULL,command_prompt_thread,NULL);
	pthread_mutex_lock(&exitMtx);
	while(acessVarMtx(&varMtx,&alive,0,-1)){

		pthread_cond_wait(&exitCond,&exitMtx);

	}
	pthread_mutex_unlock(&exitMtx);
	printf("sigint was called in server!!\n");
	close(client_socket);
        if(enable_tty_mode){
		close(master_fd);
	}
	else{
		close(inpipe[0]);
		close(inpipe[1]);
	}
	pthread_join(commandPrompt,NULL);
	if(!enable_tty_mode){
		close(err_socket);
		close(errpipe[0]);
		close(errpipe[1]);
		fflush(stderr);
		pthread_join(errWritter,NULL);
	}
        if(!enable_tty_mode){
		close(master_fd);
	}
	close(output_socket);
	if(!enable_tty_mode){

		close(outpipe[0]);
		close(outpipe[1]);
		fflush(stdout);
	}
	pthread_join(outputWritter,NULL);
	close(lifeline_socket);
	pthread_join(connectionChecker,NULL);
        close(server_socket);
	
	close(0);
	close(1);
	close(2);
	return 0;
}

