#include "Includes/preprocessor.h"

static const char* ping= "gimmemore!";
static int enable_tty_mode=0;
//static int32_t all_ready=0;
static int32_t all_alive=0;
static int32_t err_alive=0;
static int32_t out_alive=0;
static int32_t cmd_alive=0;
static int32_t ping_alive=0;
static pthread_mutex_t cleanupMtx= PTHREAD_MUTEX_INITIALIZER;
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
static const u_int32_t one_for_detach_zero_for_join=1;
#define MAXNUMBEROFTRIES 10


#define MAXTIMEOUTCONS 1
#define MAXTIMEOUTUCONS 0


#define MAXTIMEOUTSECS (60*5)
#define MAXTIMEOUTUSECS 0

#define MAXTIMEOUTCMD (60*5)
#define MAXTIMEOUTUCMD 0

#define MAXTIMEOUTPING (60*5)
#define MAXTIMEOUTUPING 0

#define LINESIZE 1024

#define DATASIZE 1024


static pthread_t connectionChecker;
static pthread_t outputWritter;
static pthread_t errWritter;
static pthread_t commandPrompt;
static pthread_t cleanupCrew;
int outpipe[2];
int errpipe[2];
int inpipe[2];

struct sockaddr_in server_address;
struct sockaddr_in server_ack_address;
struct sockaddr_in client_ack_address;

u_int16_t server_ack_port=-1;
u_int16_t client_ack_port=-1;



//on example_pipe[2]...
//write to example_pipe[1]
//read from example_pipe[0]


int err_safety_pipe[2];
int out_safety_pipe[2];
int cmd_safety_pipe[2];
int master_fd_safety_pipe[2];
int lifeline_safety_pipe[2];

int master_fd=-1;
int slave_fd=-1;
int pid=0;
int32_t server_socket,client_socket,output_socket,lifeline_socket;
int err_socket;
char line[LINESIZE]={0};
char raw_line[LINESIZE]={0};
char  outbuff[DATASIZE]={0};
char  errbuff[DATASIZE]={0};

static void sigint_handler(int signal){
	
	acessVarMtx32(&varMtx,&all_alive,0,0);
	printf("sigint was called in server!!\nThis is the signal parameter\nBeing used in a function\nSo that it doesnt complain: %d\n",signal);
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
	struct sockaddr addr_con={0};
	
	getpeername(*socket,&addr_con,&socklenvar[1]);
	print_addr_aux("Coneccao de: ",(struct sockaddr_in*)&addr_con);

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
	print_addr_aux("server inicializado no address: ",&server_address);
        listen(server_socket,3);

	lifeline_socket= socket(AF_INET,SOCK_DGRAM,0);
	if(lifeline_socket<0){
		raise(SIGINT);
		return;

	}

}

void setupConnections(void){


	acceptConnection(&output_socket);
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
 	
	if(!enable_tty_mode){
		acceptConnection(&err_socket);
		if(!android_comp_mode_on){
			flags= fcntl(err_socket,F_GETFL);
			flags |= O_NONBLOCK;
	        	fcntl(err_socket,F_SETFD,flags);
		}
	}

}
static void init_safety_pipes(void){

if(!enable_tty_mode){

        if(pipe2(err_safety_pipe,O_NONBLOCK)){

                perror("Safety pipe on error message channel on server failed to open! Aborting");
                exit(-1);
        }
        if((err_safety_pipe[0]=dup(err_safety_pipe[0]))<0){

                perror("Safety pipe on error message channel on server failed to be duped on read end!!\nAborting");
                exit(-1);


        }
        if((err_safety_pipe[1]=dup(err_safety_pipe[1]))<0){

                perror("Safety pipe on error message channel on server failed to be duped on read end!!\nAborting");
                exit(-1);


        }
        printf("just created error message channel safety pipe in server\n");
}

if(pipe2(out_safety_pipe,O_NONBLOCK)){

        perror("Safety pipe on output message channel on server failed to be created!\nAborting");
        exit(-1);
}
if((out_safety_pipe[0]=dup(out_safety_pipe[0]))<0){

        perror("Safety pipe on output message channel on server failed to be duped on read end!!\nAborting");
        exit(-1);


}
if((out_safety_pipe[1]=dup(out_safety_pipe[1]))<0){

        perror("Safety pipe on output message channel on server failed to be duped on read end!!\nAborting");
        exit(-1);


}
printf("just created output message channel safety pipe in server\n");

if(pipe2(lifeline_safety_pipe,O_NONBLOCK)){

        perror("Safety pipe on lifeline message channel on server failed to open! Aborting");
        exit(-1);
}

if((lifeline_safety_pipe[0]=dup(lifeline_safety_pipe[0]))<0){

        perror("Safety pipe on lifeline message channel on server failed to be duped on read end!!\nAborting");
        exit(-1);

}
if((lifeline_safety_pipe[1]=dup(lifeline_safety_pipe[1]))<0){

        perror("Safety pipe on lifeline message channel on server failed to be duped on read end!!\nAborting");
        exit(-1);


}
printf("just created lifeline message channel safety pipe in server\n");

if(pipe2(cmd_safety_pipe,O_NONBLOCK)){

        perror("Safety pipe on cmd sending channel on server failed to open! Aborting");
        exit(-1);
}

if((cmd_safety_pipe[0]=dup(cmd_safety_pipe[0]))<0){

        perror("Safety pipe on cmd sending channel on server failed to be duped on read end!!\nAborting");
        exit(-1);


}
if((cmd_safety_pipe[1]=dup(cmd_safety_pipe[1]))<0){

        perror("Safety pipe on cmd sending channel on server failed to be duped on read end!!\nAborting");
        exit(-1);


}
printf("just created master_fd channel safety pipe in server\n");
if(pipe2(master_fd_safety_pipe,O_NONBLOCK)){

        perror("Safety pipe on master_fd channel on server failed to open! Aborting");
        exit(-1);
}

if((master_fd_safety_pipe[0]=dup(master_fd_safety_pipe[0]))<0){

        perror("Safety pipe on cmd sending channel on server failed to be duped on read end!!\nAborting");
        exit(-1);


}
if((master_fd_safety_pipe[1]=dup(master_fd_safety_pipe[1]))<0){

        perror("Safety pipe on cmd sending channel on server failed to be duped on read end!!\nAborting");
        exit(-1);


}
printf("just created command sending channel safety pipe in server\n");
}


static void safety_close(char* prompt_to_show,int safety_fd_write){

	printf("We are trying to close the fd of the following description safely:\nWe are in the server\n%s\n",prompt_to_show);
        write(safety_fd_write,"x",1);
	printf("Okay! We wrote the closing byte into the safety socket of the following description:\nWe are in the server\n%s\n",prompt_to_show);


}
static void build_ack_addresses(void){


	getpeername(client_socket,(struct sockaddr*)&server_ack_address,&socklenvar[0]);
	getpeername(client_socket,(struct sockaddr*)&client_ack_address,&socklenvar[0]);

	server_ack_address.sin_port=server_ack_port;
        client_ack_address.sin_port=client_ack_port;

        if(bind(lifeline_socket,(struct sockaddr*) &server_ack_address,sizeof(server_ack_address))){

                perror("Nao foi possivel dar bind!!!\n");
                close(lifeline_socket);
                close(client_socket);
                exit(-1);
        }
        print_addr_aux("Address de acks de server (bind): ",&server_ack_address);
        print_addr_aux("Address de acks de client (peer): ",&client_ack_address);



}


static int64_t timedSend(int fd,int safety_fd,char buff[],u_int64_t size,int secwait,int usecwait){
                int iResult;
                struct timeval tv;
                fd_set wrfds;
                FD_ZERO(&wrfds);
                FD_SET(fd,&wrfds);
                fd_set rfds;
                char drain_buff[LINESIZE]={0};
                FD_ZERO(&rfds);
                FD_SET(safety_fd,&rfds);
                tv.tv_sec=secwait;
                tv.tv_usec=usecwait;
                //printf("just read from pipe or not??\nSafety fd: %d\n\n",safety_fd);
                iResult=select(MAX(safety_fd,fd)+1,&rfds,&wrfds,(fd_set*)0,&tv);
                if(iResult>0){
                while(read(safety_fd,drain_buff,1)>0){

                        printf("reading from safety fd in the server's timedSend function!!! %s\n",drain_buff);
                	close(safety_fd);
			close(fd);
			return -1;
                }
                //printf("just read from pipe or not\n");
	        return write(fd,buff,size);
                }
                else if(iResult){
                return -1;
                }
                else{
                return 0;
                }

}

static int64_t timedRead(int fd,int safety_fd,char buff[],u_int64_t size,int secwait,int usecwait){
                int iResult;
                struct timeval tv;
                fd_set rfds;
                char drain_buff[LINESIZE]={0};
                FD_ZERO(&rfds);
                FD_SET(fd,&rfds);
                FD_SET(safety_fd,&rfds);
                tv.tv_sec=secwait;
                tv.tv_usec=usecwait;
                //printf("just read from pipe or not??\nSafety fd: %d\n\n",safety_fd);
                iResult=select(MAX(safety_fd,fd)+1,&rfds,(fd_set*)0,(fd_set*)0,&tv);
                if(iResult>0){
                while(read(safety_fd,drain_buff,1)>0){

                        printf("reading from safety fd in the server's timedRead function!!! %s\n",drain_buff);
                	close(safety_fd);
			close(fd);
			return -1;
		}
                //printf("just read from pipe or not\n");
	        return read(fd,buff,size);
                }
                else if(iResult){
                return -1;
                }
                else{
                return 0;
                }


}
static int64_t timedSendUDP(int fd,int safety_fd,char buff[],u_int64_t size,int secwait,int usecwait){
                int iResult;
                struct timeval tv;
                fd_set wrfds;
                FD_ZERO(&wrfds);
                FD_SET(fd,&wrfds);
                fd_set rfds;
                char drain_buff[LINESIZE]={0};
                FD_ZERO(&rfds);
                FD_SET(safety_fd,&rfds);
                tv.tv_sec=secwait;
                tv.tv_usec=usecwait;
                //printf("just read from pipe or not??\nSafety fd: %d\n\n",safety_fd);
                iResult=select(MAX(safety_fd,fd)+1,&rfds,&wrfds,(fd_set*)0,&tv);
                if(iResult>0){
                while(read(safety_fd,drain_buff,1)>0){

                        printf("reading from safety fd in the server's timedSendUDP function!!! %s\n",drain_buff);
                	close(safety_fd);
			close(fd);
			return -1;
                }
                //printf("just read from pipe or not\n");
	        return sendto(fd,buff,size,0,&client_ack_address,socklenvar[1]);
                }
                else if(iResult){
                return -1;
                }
                else{
                return 0;
                }

}
static int64_t timedReadUDP(int fd,int safety_fd,char buff[],u_int64_t size,int secwait,int usecwait){
                int iResult;
                struct timeval tv;
                fd_set rfds;
                char drain_buff[LINESIZE]={0};
                FD_ZERO(&rfds);
                FD_SET(fd,&rfds);
                FD_SET(safety_fd,&rfds);
                tv.tv_sec=secwait;
                tv.tv_usec=usecwait;
                //printf("just read from pipe or not??\nSafety fd: %d\n\n",safety_fd);
                iResult=select(MAX(safety_fd,fd)+1,&rfds,(fd_set*)0,(fd_set*)0,&tv);
                if(iResult>0){
                while(read(safety_fd,drain_buff,1)>0){

                       printf("reading from safety fd in the server's timedReadUDP function!!! %s\n",drain_buff);
                	close(safety_fd);
			close(fd);
			return -1;
		}
                //printf("just read from pipe or not\n");
		socklen_t tmp_var=0;
	        return recvfrom(fd,buff,size,0,&client_ack_address,&tmp_var);
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

	printf("Server's ping channel thread online!!!\n");


	acessVarMtx32(&varMtx,&out_alive,1,0);
	acessVarMtx32(&varMtx,&err_alive,!enable_tty_mode,0);

	pthread_cond_signal(&outCond);
	pthread_cond_signal(&errCond);

	int pingLength=strlen(ping);
	char buff[strlen(ping)];
        int numread=1;
	memset(buff,0,pingLength);
	acessVarMtx32(&varMtx,&ping_alive,1,0);
	acessVarMtx32(&varMtx,&all_alive,1,0);
	while(acessVarMtx32(&varMtx,&all_alive,0,-1)&&acessVarMtx32(&varMtx,&ping_alive,0,-1)){
		while(acessVarMtx32(&varMtx,&all_alive,0,-1)&&acessVarMtx32(&varMtx,&ping_alive,0,-1)&&((numread=timedReadUDP(lifeline_socket,lifeline_safety_pipe[0],buff,pingLength,MAXTIMEOUTPING,MAXTIMEOUTUPING))>0)){
			if(timedSendUDP(lifeline_socket,lifeline_safety_pipe[0],(char*)ping,strlen(ping),MAXTIMEOUTPING,MAXTIMEOUTUPING)<=0){
				acessVarMtx32(&varMtx,&all_alive,0,0);
				break;
			}
		}
		acessVarMtx32(&varMtx,&all_alive,0,0);
		break;

	}
	acessVarMtx32(&varMtx,&ping_alive,0,0);
	printf("Server's ping channel thread out!!!\n");
	return args;

}

static void* writeOutput(void* args){

	pthread_mutex_lock(&outMtx);
	while(!acessVarMtx32(&varMtx,&out_alive,0,-1)&&acessVarMtx32(&varMtx,&all_alive,0,-1)){

		pthread_cond_wait(&outCond,&outMtx);

	}
	pthread_mutex_unlock(&outMtx);
	printf("Server's output message channel thread online!!!\n");
	acessVarMtx32(&varMtx,&cmd_alive,1,0);

	pthread_cond_signal(&cmdCond);
	while(acessVarMtx32(&varMtx,&all_alive,0,-1)&&acessVarMtx32(&varMtx,&out_alive,0,-1)){
		int numread=1;
		while(acessVarMtx32(&varMtx,&out_alive,0,-1)&&acessVarMtx32(&varMtx,&all_alive,0,-1)&&((numread=timedRead(enable_tty_mode?master_fd:outpipe[0],enable_tty_mode?master_fd_safety_pipe[0]:out_safety_pipe[0],outbuff,DATASIZE,MAXTIMEOUTSECS,MAXTIMEOUTUSECS))>=0)){
		if(!numread){
			continue;
		}
		if(timedSend(output_socket,out_safety_pipe[0],outbuff,DATASIZE,MAXTIMEOUTSECS,MAXTIMEOUTUSECS)<=0){
			break;
		}
		memset(outbuff,0,DATASIZE);
		}

	}
	acessVarMtx32(&varMtx,&out_alive,0,0);
	printf("Server's output message channel thread out!!!\n");
	return args;



}
static void* writeErr(void* args){
	pthread_mutex_lock(&errMtx);
	while(!acessVarMtx32(&varMtx,&err_alive,0,-1)&&acessVarMtx32(&varMtx,&all_alive,0,-1)){

		pthread_cond_wait(&errCond,&errMtx);

	}
	pthread_mutex_unlock(&errMtx);
	printf("Server's error message channel thread online!!!\n");
	acessVarMtx32(&varMtx,&cmd_alive,1,0);

	pthread_cond_signal(&cmdCond);

	while(acessVarMtx32(&varMtx,&all_alive,0,-1)&&acessVarMtx32(&varMtx,&err_alive,0,-1)){
		int numread=1;
		while(acessVarMtx32(&varMtx,&all_alive,0,-1)&&acessVarMtx32(&varMtx,&err_alive,0,-1)&&((numread=timedRead(errpipe[0],err_safety_pipe[0],errbuff,DATASIZE,MAXTIMEOUTSECS,MAXTIMEOUTUSECS))>=0)){
		if(!numread){
			continue;
		}
		if(timedSend(err_socket,err_safety_pipe[0],errbuff,DATASIZE,MAXTIMEOUTSECS,MAXTIMEOUTUSECS)<=0){
			break;
		}
		memset(errbuff,0,DATASIZE);
		}

	}
	acessVarMtx32(&varMtx,&err_alive,0,0);
	printf("Server's error message channel thread out!!!\n");
	return args;



}
static void* command_prompt_thread(void* args){
	
	pthread_mutex_lock(&cmdMtx);
	while(!acessVarMtx32(&varMtx,&cmd_alive,0,-1)&&acessVarMtx32(&varMtx,&all_alive,0,-1)){

		pthread_cond_wait(&cmdCond,&cmdMtx);

	}
	pthread_mutex_unlock(&cmdMtx);

	printf("Server's command receiving channel thread about to start!\n");
	char buff[strlen(ping)+1];
	
	while(acessVarMtx32(&varMtx,&cmd_alive,0,-1)&&acessVarMtx32(&varMtx,&all_alive,0,-1)){
	memset(raw_line,0,sizeof(raw_line));
	memset(line,0,sizeof(line));
	while(acessVarMtx32(&varMtx,&cmd_alive,0,-1)&&acessVarMtx32(&varMtx,&all_alive,0,-1)&&(timedRead(client_socket,cmd_safety_pipe[0],raw_line,LINESIZE,MAXTIMEOUTCMD,MAXTIMEOUTUCMD)>0)){

	
	memset(buff,0,strlen(ping)+1);
	memcpy(buff,ping,strlen(ping));
	memcpy(line,raw_line,strlen(raw_line)-1);
	write(enable_tty_mode?master_fd:inpipe[1],raw_line,strlen(raw_line));
	printf("Command received: raw line: |%s|\nActual line: |%s|\n",raw_line,line);
	
	printf("O valor deste caracter é: %d\n",line[(strlen(line))-1]);
	if(!strncmp(line, "exit",strlen("exit"))&&((strlen(line))==strlen("exit"))){

		printf("The server got orders to exit!\n");
		acessVarMtx32(&varMtx,&all_alive,0,0);
		break;

	}
	if(!enable_tty_mode){
		fflush(stdout);
		fflush(stderr);
	}
	memset(line,0,sizeof(line));
	}
	}
	acessVarMtx32(&varMtx,&cmd_alive,0,0);
	printf("Server's command receiving channel thread about to exit!\n");
	return args;

}

void* cleanup_crew(void*args){

	pthread_mutex_lock(&exitMtx);
	while(acessVarMtx32(&varMtx,&all_alive,0,-1)){

		pthread_cond_wait(&exitCond,&exitMtx);

	}
	pthread_mutex_unlock(&exitMtx);
	printf("Cleanup crew called in server\n");
	printf("The server got orders to exit!\n");
	kill(pid,SIGTERM);
	print_sock_addr(client_socket);
	waitpid(pid,NULL,0);

	printf("Cleanup crew called in server. About to join threads which are online\n");
	if(!acessVarMtx32(&varMtx,&cmd_alive,0,-1)||!acessVarMtx32(&varMtx,&all_alive,0,-1)){
		printf("reaping server cmdline thread!!\n");
		if(!one_for_detach_zero_for_join){
			pthread_join(commandPrompt,NULL);
		}
		else{
			pthread_detach(commandPrompt);
		}
	}
	if(!acessVarMtx32(&varMtx,&out_alive,0,-1)||!acessVarMtx32(&varMtx,&all_alive,0,-1)){
		printf("reaping server output writter thread!!\n");
		if(!one_for_detach_zero_for_join){
			pthread_join(outputWritter,NULL);
		}
		else{
			pthread_detach(outputWritter);
		}
	}
	if(!enable_tty_mode){

		if(!acessVarMtx32(&varMtx,&err_alive,0,-1)||!acessVarMtx32(&varMtx,&all_alive,0,-1)){
			printf("reaping server error printer thread!!\n");
			if(!one_for_detach_zero_for_join){
				pthread_join(errWritter,NULL);
			}
			else{
				pthread_detach(errWritter);
			}
		}
	}
	if(!acessVarMtx32(&varMtx,&ping_alive,0,-1)||!acessVarMtx32(&varMtx,&all_alive,0,-1)){
		printf("reaping server connection checker thread!!\n");
		if(!one_for_detach_zero_for_join){
			pthread_join(connectionChecker,NULL);
        	}
		else{
			pthread_detach(connectionChecker);
		}
	}
	printf("Cleanup crew called in server. Closing file descriptors and sockets\n");
	if(enable_tty_mode){
		close(master_fd);
	}
	else{
		close(inpipe[0]);
		close(inpipe[1]);
		close(errpipe[0]);
		close(errpipe[1]);
		fflush(stderr);
		close(outpipe[0]);
		close(outpipe[1]);
		fflush(stdout);
	}

	close(server_socket);
	safety_close("Lifeline ack exchanger socket",lifeline_safety_pipe[1]);
	printf("Finished cleanup in server.\n");
	return args;

}


int main(int argc, char ** argv){
	
	if(argc!=6){

		printf("arg1: address\narg2: porta do server\narg3: porta do server para acks\narg4: shell to use\narg5: 0/1 = (dis)/(en)able tty mode (experimental)\n");
		exit(-1);
	}
	enable_tty_mode=atoi(argv[5]);
	if((enable_tty_mode<0)||(enable_tty_mode>1)){

		printf("arg4 so pode ser 0 ou 1!\n");
		exit(-1);
	}
	all_alive=1;
	init_safety_pipes();
	initServer(argv[1],atoi(argv[2]));
	acceptConnection(&client_socket);
	char sizes[DATASIZE]={0};
	server_ack_port=atoi(argv[3]);
	snprintf(sizes,10,"%d %hu",enable_tty_mode,htons(server_ack_port));
	//snprintf(sizes,10,"%d %d",enable_tty_mode,server_ack_port);
	send(client_socket,sizes,DATASIZE,0);
	char buff[DATASIZE]={0};
	timedRead(client_socket,cmd_safety_pipe[1],buff,DATASIZE,MAXTIMEOUTCONS,MAXTIMEOUTUCONS);
	sscanf(buff,"%hu",&client_ack_port);
	client_ack_port=ntohs(client_ack_port);
	printf("Send de ping feito!\nPorta enviada: %hu\nhtonl de porta enviada! %hu\nntohl de porta enviada %hu\n",server_ack_port,htons(server_ack_port),ntohs(server_ack_port));
	build_ack_addresses();
	pthread_create(&connectionChecker,NULL,areYouStillThere,NULL);
	pthread_setname_np(connectionChecker,"connectionChecker_remote_shell_server");
	pthread_create(&outputWritter,NULL,writeOutput,NULL);
	pthread_setname_np(outputWritter,"outputWritter_remote_shell_server");
	if(!enable_tty_mode){
		pthread_create(&errWritter,NULL,writeErr,NULL);
		pthread_setname_np(errWritter,"errWritter_remote_shell_server");
	}


	printf("Shell name: %s\n",argv[4]);
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
				snprintf(arg0,LINESIZE-1,"/bin/%s",argv[4]);
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
				snprintf(arg0,LINESIZE-1,"/bin/%s",argv[4]);
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
	pthread_setname_np(commandPrompt,"commandPrompt_remote_shell_server");
	pthread_create(&cleanupCrew,NULL,cleanup_crew,NULL);
	pthread_setname_np(cleanupCrew,"cleanupCrew_remote_shell_server");

	signal(SIGINT,sigint_handler);
	signal(SIGPIPE,sigpipe_handler);
	acessVarMtx32(&varMtx,&ping_alive,1,0);
	pthread_cond_signal(&pingCond);
	printf("waiting for session end to reap cleanup crew thread!!\n");
	pthread_join(cleanupCrew,NULL);
        printf("ending server.\n");
	return 0;
}
