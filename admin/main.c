#include "Includes/preprocessor.h"

static const char* ping= "gimmemore!";
static int enable_tty_mode=0;
//static int32_t all_ready=0;
static int32_t all_alive=0;
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

static pthread_t connectionChecker;
static pthread_t outputWritter;
static pthread_t errWritter;
static pthread_t commandPrompt;
static pthread_t cleanupCrew;
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


	
static void sigint_handler(int signal){

	printf("sigint was called in server!!\n");
	acessVarMtx32(&varMtx,&all_alive,0,0*signal);
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
		exit(-1);
	}
	int ptr=1;
	struct linger the_linger={0,30};
        if(setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR,(char*)&ptr,sizeof(ptr))){
		perror("Erro a meter SO_REUSEADDR  na socket (setsockopt)\n");
		close(server_socket);
		exit(-1);
	}
	if(setsockopt(server_socket,SOL_SOCKET,SO_LINGER,(char*)&the_linger,sizeof(struct linger))){
		perror("Erro a meter SO_LINGER na socket (setsockopt)\n");
		close(server_socket);
		exit(-1);
	}
	if(android_comp_mode_on){
		fcntl(server_socket,F_SETFL,O_NONBLOCK);
	}
	init_addr(&server_address,address,port);
	if(bind(server_socket,(struct sockaddr*) &server_address,sizeof(server_address))){

		perror("Nao foi possivel dar bind!!!\n");
		close(server_socket);
		exit(-1);
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
void setupConnections(void){


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
 	
	if(!enable_tty_mode){
		acceptConnection(&err_socket);
		if(!android_comp_mode_on){
			flags= fcntl(err_socket,F_GETFL);
			flags |= O_NONBLOCK;
	        	fcntl(err_socket,F_SETFD,flags);
		}
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
	
        pthread_mutex_lock(&pingMtx);
	while(!acessVarMtx32(&varMtx,&ping_alive,0,-1)&&acessVarMtx32(&varMtx,&all_alive,0,-1)){

		pthread_cond_wait(&pingCond,&pingMtx);

	}
	pthread_mutex_unlock(&pingMtx);

	printf("checker online!!!\n");


	acessVarMtx32(&varMtx,&out_alive,1,0);
	acessVarMtx32(&varMtx,&err_alive,!enable_tty_mode,0);

	pthread_cond_signal(&outCond);
	pthread_cond_signal(&errCond);

	int pingLength=strlen(ping);
	char buff[strlen(ping)];
        memset(buff,0,pingLength);
	acessVarMtx32(&varMtx,&ping_alive,1,0);
	acessVarMtx32(&varMtx,&all_alive,1,0);
	while(acessVarMtx32(&varMtx,&all_alive,0,-1)){
		char buff[strlen(ping)];
		int numread=1;
		while(acessVarMtx32(&varMtx,&all_alive,0,-1)&&((numread=timedRead(lifeline_socket,buff,pingLength,MAXTIMEOUTPING,MAXTIMEOUTUPING))>0)){
		timedSend(lifeline_socket,(char*)ping,strlen(ping),MAXTIMEOUTPING,MAXTIMEOUTUPING);
		}
		raise(SIGINT);
		break;

	}
	acessVarMtx32(&varMtx,&ping_alive,0,0);
	printf("checker out!!!\n");
	return args;

}

static void* writeOutput(void* args){
	
	pthread_mutex_lock(&outMtx);
	while(!acessVarMtx32(&varMtx,&out_alive,0,-1)&&acessVarMtx32(&varMtx,&all_alive,0,-1)){

		pthread_cond_wait(&outCond,&outMtx);

	}
	pthread_mutex_unlock(&outMtx);
	printf("output writter online!!!\n");
	acessVarMtx32(&varMtx,&cmd_alive,1,0);

	pthread_cond_signal(&cmdCond);
	while(acessVarMtx32(&varMtx,&all_alive,0,-1)){
		int numread=1;
		while(acessVarMtx32(&varMtx,&all_alive,0,-1)&&((numread=timedRead(enable_tty_mode?master_fd:outpipe[0],outbuff,DATASIZE,MAXTIMEOUTSECS,MAXTIMEOUTUSECS))>=0)){
		if(!numread){
			continue;
		}
		timedSend(output_socket,outbuff,DATASIZE,MAXTIMEOUTSECS,MAXTIMEOUTUSECS);
		memset(outbuff,0,DATASIZE);
		}

	}
	acessVarMtx32(&varMtx,&out_alive,0,0);
	printf("output writter out!!!\n");
	return args;



}
static void* writeErr(void* args){
	pthread_mutex_lock(&errMtx);
	while(!acessVarMtx32(&varMtx,&err_alive,0,-1)&&acessVarMtx32(&varMtx,&all_alive,0,-1)){

		pthread_cond_wait(&errCond,&errMtx);

	}
	pthread_mutex_unlock(&errMtx);
	printf("error writter online!!!\n");
	acessVarMtx32(&varMtx,&cmd_alive,1,0);

	pthread_cond_signal(&cmdCond);

	while(acessVarMtx32(&varMtx,&all_alive,0,-1)){
		int numread=1;
		while(acessVarMtx32(&varMtx,&all_alive,0,-1)&&((numread=timedRead(errpipe[0],errbuff,DATASIZE,MAXTIMEOUTSECS,MAXTIMEOUTUSECS))>=0)){
		if(!numread){
			continue;
		}
		timedSend(err_socket,errbuff,DATASIZE,MAXTIMEOUTSECS,MAXTIMEOUTUSECS);
		memset(errbuff,0,DATASIZE);
		}

	}
	acessVarMtx32(&varMtx,&err_alive,0,0);
	printf("error writter out!!!\n");
	return args;



}
static void* command_prompt_thread(void* args){
	
	pthread_mutex_lock(&cmdMtx);
	while(!acessVarMtx32(&varMtx,&cmd_alive,0,-1)&&acessVarMtx32(&varMtx,&all_alive,0,-1)){

		pthread_cond_wait(&cmdCond,&cmdMtx);

	}
	pthread_mutex_unlock(&cmdMtx);

	printf("Command prompt about to start!\n");
	char buff[strlen(ping)+1];
	
	while(acessVarMtx32(&varMtx,&all_alive,0,-1)){
	memset(line,0,sizeof(line));
	while(acessVarMtx32(&varMtx,&all_alive,0,-1)&&(receiveClientInput(client_socket,line,LINESIZE,MAXTIMEOUTCMD,MAXTIMEOUTUCMD)>=0)){

	
	memset(buff,0,strlen(ping)+1);
	memcpy(buff,ping,strlen(ping));
	write(enable_tty_mode?master_fd:inpipe[1],line,strlen(line));
	printf("Command received: %s\n",line);
	
	printf("O valor deste caracter é: %d\n",line[(strlen(line))-1]);
	if(!strncmp(line, "exit",strlen(line)-1)&&((strlen(line)-1)==strlen("exit"))){

		printf("we got orders to exit!\n");
		print_sock_addr(client_socket);
		waitpid(-1,NULL,0);
		raise(SIGINT);
		break;

	}
	if(!enable_tty_mode){
		fflush(stdout);
		fflush(stderr);
	}
	memset(line,0,sizeof(line));
	}
	}
	acessVarMtx32(&varMtx,&all_alive,0,0);
	acessVarMtx32(&varMtx,&cmd_alive,0,0);
	printf("Command prompt about to exit!\n");
	return args;

}

void* cleanup_crew(void*args){

	pthread_mutex_lock(&exitMtx);
	while(acessVarMtx32(&varMtx,&all_alive,0,-1)){

		pthread_cond_wait(&exitCond,&exitMtx);

	}
	pthread_mutex_unlock(&exitMtx);
	printf("Cleanup crew called in server\n");
	if(enable_tty_mode){
		close(master_fd);
	}
	else{
		close(inpipe[0]);
		close(inpipe[1]);
		close(err_socket);
		close(errpipe[0]);
		close(errpipe[1]);
		fflush(stderr);
		close(outpipe[0]);
		close(outpipe[1]);
		fflush(stdout);
	}

	close(server_socket);
	close(client_socket);
	close(lifeline_socket);
	close(output_socket);

	printf("Cleanup crew called in server. About to join threads which are online\n");
	if(!acessVarMtx32(&varMtx,&cmd_alive,0,-1)){
		printf("reaping server cmdline thread!!\n");
		pthread_join(commandPrompt,NULL);
	}
	if(!acessVarMtx32(&varMtx,&out_alive,0,-1)){
		printf("reaping server output writter thread!!\n");
		pthread_join(outputWritter,NULL);
	}
	if(!enable_tty_mode){

		if(!acessVarMtx32(&varMtx,&err_alive,0,-1)){
			printf("reaping server error printer thread!!\n");
			pthread_join(errWritter,NULL);
		}
	}
	if(!acessVarMtx32(&varMtx,&ping_alive,0,-1)){
		printf("reaping server connection checker thread!!\n");
		pthread_join(connectionChecker,NULL);
        }
	printf("Finished cleanup in server.\n");
	close(0);
	close(1);
	close(2);
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
	all_alive=1;
	initServer(argv[1],atoi(argv[2]));
	acceptConnection(&client_socket);
	char sizes[10]={0};
	snprintf(sizes,10,"%d %d",DATASIZE,enable_tty_mode);
	send(client_socket,sizes,10,0);
	char buff[strlen(ping)];
	memset(buff,0,strlen(ping));
	receiveClientInput(client_socket,buff,strlen(ping),MAXTIMEOUTCONS,MAXTIMEOUTUCONS);

	pthread_create(&connectionChecker,NULL,areYouStillThere,NULL);

	pthread_create(&outputWritter,NULL,writeOutput,NULL);
	if(!enable_tty_mode){
		pthread_create(&errWritter,NULL,writeErr,NULL);
	}


	printf("Shell name: %s\n",argv[3]);
	setupConnections();

	initpipes();

	if(!enable_tty_mode){
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
	pthread_create(&cleanupCrew,NULL,cleanup_crew,NULL);

	signal(SIGINT,sigint_handler);
	signal(SIGPIPE,sigpipe_handler);
	ping_alive=1;
	pthread_cond_signal(&pingCond);
	pthread_join(cleanupCrew,NULL);
	return 0;
}
