#include "Includes/preprocessor.h"
static const char* ping= "gimmemore!";

static int enable_tty_mode=0;
static const u_int64_t android_comp_mode_on=0;
static const u_int32_t one_for_detach_zero_for_join=1;
//static int32_t all_ready=1;
static int32_t all_alive=1;
static int32_t err_alive=0;
static int32_t out_alive=0;
static int32_t cmd_alive=0;
static pthread_mutex_t varMtx= PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t exitCond= PTHREAD_COND_INITIALIZER;
static pthread_mutex_t exitMtx= PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cmdCond= PTHREAD_COND_INITIALIZER;
static pthread_mutex_t cmdMtx= PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t errCond= PTHREAD_COND_INITIALIZER;
static pthread_mutex_t errMtx= PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t outCond= PTHREAD_COND_INITIALIZER;
static pthread_mutex_t outMtx= PTHREAD_MUTEX_INITIALIZER;
#define MAXNUMBEROFTRIES 10


#define MAXTIMEOUTCONS 1
//#define MAXTIMEOUTCONS 1
#define MAXTIMEOUTUCONS 0

#define MAXTIMEOUTSECS (60*5)
#define MAXTIMEOUTUSECS 0

#define MAXTIMEOUTCMD (60*5)
#define MAXTIMEOUTUCMD 0


#define LINESIZE 1024
#define DATASIZE 1024


static pthread_t commandPrompt;
static pthread_t cleanupCrew;
static pthread_t outputPrinter;
static pthread_t errPrinter;
char outbuff[LINESIZE*10]={0};
char errbuff[LINESIZE*10]={0};
char line[LINESIZE]={0};
char raw_line[LINESIZE]={0};

static struct sockaddr_in server_address;
//on example_pipe[2]...
//write to example_pipe[1]
//read from example_pipe[0]


int err_safety_pipe[2];
int out_safety_pipe[2];
int cmd_safety_pipe[2];



int client_socket,output_socket;
int err_socket;

static void sigint_handler(int signal){

        printf("sigint was called in server!!\n");
        acessVarMtx32(&varMtx,&all_alive,0,0*signal);
        pthread_cond_signal(&exitCond);
}

static void sigpipe_handler(int signal){

        printf("sigpipe was called in server!!\n");
        raise(SIGINT+signal*0);
}

static void init_safety_pipes(void){


if(!enable_tty_mode){

	if(pipe2(err_safety_pipe,O_NONBLOCK)){

		perror("Safety pipe on error message channel on client failed to open! Aborting");
		exit(-1);
	}
	if((err_safety_pipe[0]=dup(err_safety_pipe[0]))<0){

		perror("Safety pipe on error message channel on client failed to be duped on read end!!\nAborting");
		exit(-1);


	}
	if((err_safety_pipe[1]=dup(err_safety_pipe[1]))<0){

		perror("Safety pipe on error message channel on client failed to be duped on read end!!\nAborting");
		exit(-1);


	}
	printf("just created error message channel safety pipe in client\n");
}

if(pipe2(out_safety_pipe,O_NONBLOCK)){

	perror("Safety pipe on output message channel on client failed to be created!\nAborting");
	exit(-1);
}
if((out_safety_pipe[0]=dup(out_safety_pipe[0]))<0){

	perror("Safety pipe on output message channel on client failed to be duped on read end!!\nAborting");
	exit(-1);


}
if((out_safety_pipe[1]=dup(out_safety_pipe[1]))<0){

	perror("Safety pipe on output message channel on client failed to be duped on read end!!\nAborting");
	exit(-1);


}
printf("just created output message channel safety pipe in client\n");


if(pipe2(cmd_safety_pipe,O_NONBLOCK)){

	perror("Safety pipe on cmd sending channel on client failed to open! Aborting");
	exit(-1);
}

if((cmd_safety_pipe[0]=dup(cmd_safety_pipe[0]))<0){

	perror("Safety pipe on cmd sending channel on client failed to be duped on read end!!\nAborting");
	exit(-1);


}
if((cmd_safety_pipe[1]=dup(cmd_safety_pipe[1]))<0){

	perror("Safety pipe on cmd sending channel on client failed to be duped on read end!!\nAborting");
	exit(-1);


}
printf("just created command sending channel safety pipe in client\nSetting flags");

}

static void safety_close(char* prompt_to_show,int safety_fd_write){

        printf("We are trying to close the fd of the following description safely:\nWe are in the client\n%s\n",prompt_to_show);
        write(safety_fd_write,"x",1);
	printf("Okay! We wrote the closing byte into the safety socket of the following description:\nWe are in the client\n%s\n",prompt_to_show);
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

			printf("reading from safety fd in the client's timedSend function!!! %s\n",drain_buff);
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

			printf("reading from safety fd in the client's timedRead function!!! %s\n",drain_buff);
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

static void initClient(int port, char* addr){

	client_socket= socket(AF_INET,SOCK_STREAM,0);
	if(client_socket==-1){
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
	memset(outbuff,0,DATASIZE);
	acessVarMtx32(&varMtx,&cmd_alive,1,0);
	
	pthread_cond_signal(&cmdCond);
	while(acessVarMtx32(&varMtx,&out_alive,0,-1)&&acessVarMtx32(&varMtx,&all_alive,0,-1)){
	int numread=1;
	
	while(acessVarMtx32(&varMtx,&out_alive,0,-1)&&acessVarMtx32(&varMtx,&all_alive,0,-1)&&((numread=timedRead(output_socket,out_safety_pipe[0],outbuff,DATASIZE,MAXTIMEOUTSECS,MAXTIMEOUTUSECS))>0)){
		if(!numread){
			continue;
		}
		printf("%s",outbuff);
		memset(outbuff,0,DATASIZE);
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
	memset(errbuff,0,DATASIZE);
	acessVarMtx32(&varMtx,&cmd_alive,1,0);
	
	pthread_cond_signal(&cmdCond);
 	while(acessVarMtx32(&varMtx,&err_alive,0,-1)&&acessVarMtx32(&varMtx,&all_alive,0,-1)){
	int numread=1;
	while(acessVarMtx32(&varMtx,&err_alive,0,-1)&&acessVarMtx32(&varMtx,&all_alive,0,-1)&&((numread=timedRead(err_socket,err_safety_pipe[0],errbuff,DATASIZE,MAXTIMEOUTCMD,MAXTIMEOUTUCMD))>0)){
		if(!numread){
			continue;
		}
		fprintf(stderr,"%s",errbuff);
		memset(errbuff,0,DATASIZE);
	}
	}
	acessVarMtx32(&varMtx,&err_alive,0,0);
	printf("Client's error printing message channel thread exiting!\n");
	return args;

}
static void* command_line_thread(void* args){
	pthread_mutex_lock(&cmdMtx);
	while(!acessVarMtx32(&varMtx,&cmd_alive,0,-1)&&acessVarMtx32(&varMtx,&all_alive,0,-1)){
		pthread_cond_wait(&cmdCond,&cmdMtx);

	}
	pthread_mutex_unlock(&cmdMtx);
	printf("Client's command sending channel thread alive!\n");
	char buff[strlen(ping)+1];
	memset(line,0,LINESIZE);
	memset(raw_line,0,LINESIZE);
	while(acessVarMtx32(&varMtx,&cmd_alive,0,-1)&&acessVarMtx32(&varMtx,&all_alive,0,-1)){

		memset(raw_line,0,LINESIZE);
		memset(line,0,LINESIZE);
		memset(buff,0,strlen(ping)+1);
		fgets(raw_line,LINESIZE,stdin);
		//line[strlen(line)-1]=enable_tty_mode?'\n':0;
		memcpy(line,raw_line,strlen(raw_line));
		line[strlen(line)-1]='\n';
		printf("The command (clients perspective): |%s|\nThe raw line: |%s|\n",line,raw_line);
		if(!strncmp(line, "exit",strlen("exit"))&&((strlen(raw_line))==strlen("exit"))){
			acessVarMtx32(&varMtx,&all_alive,0,0);
			acessVarMtx32(&varMtx,&cmd_alive,0,0);
			break;
		}
		timedSend(client_socket,cmd_safety_pipe[0],line,LINESIZE,MAXTIMEOUTCMD,MAXTIMEOUTUCMD);
	}
	
	acessVarMtx32(&varMtx,&all_alive,0,0);
	acessVarMtx32(&varMtx,&cmd_alive,0,0);
	printf("Client's command sending channel thread exiting!\n");

	return args;
}

void* cleanup_crew(void*args){

        pthread_mutex_lock(&cleanupMtx);
        pthread_mutex_lock(&exitMtx);
        while(acessVarMtx32(&varMtx,&all_alive,0,-1)){

                pthread_cond_wait(&exitCond,&exitMtx);

        }
        pthread_mutex_unlock(&exitMtx);
        printf("Cleanup crew called in client\n");
        printf("Cleanup crew called in client. Closing file descriptors and sockets\n");
        fflush(stdout);
        safety_close("Command sending socket",cmd_safety_pipe[1]);
	safety_close("Output message receiving socket",out_safety_pipe[1]);
	printf("Cleanup crew called in client. About to join threads which are online\n");
        if(!acessVarMtx32(&varMtx,&cmd_alive,0,-1)||!acessVarMtx32(&varMtx,&all_alive,0,-1)){
                printf("Reaping client cmdline thread!!\n");
		if(!one_for_detach_zero_for_join){
	                pthread_join(commandPrompt,NULL);
        	}
		else{

		        pthread_detach(commandPrompt);
        	}

	}
        if(!acessVarMtx32(&varMtx,&out_alive,0,-1)||!acessVarMtx32(&varMtx,&all_alive,0,-1)){
                printf("Reaping client output writter thread!!\n");
                if(!one_for_detach_zero_for_join){
	                pthread_join(outputPrinter,NULL);
        	}
		else{
		        pthread_detach(outputPrinter);
        	}
	}
        if(!enable_tty_mode){

		if(!acessVarMtx32(&varMtx,&err_alive,0,-1)||!acessVarMtx32(&varMtx,&all_alive,0,-1)){
                	printf("Reaping client error printer thread!!\n");
                	if(!one_for_detach_zero_for_join){
	                	pthread_join(errPrinter,NULL);
        		}
			else{
				pthread_detach(errPrinter);
			}
		}
	}
	        if(!enable_tty_mode){
                safety_close("Error message receiving socket",err_safety_pipe[1]);
                fflush(stderr);
        }


	printf("Finished cleanup in client.\n");
        pthread_mutex_unlock(&cleanupMtx);
        return args;

}

int main(int argc, char ** argv){

	if(argc!=3){

		printf("Utilizacao correta: arg1: ip de server a connectar.\narg2: porta de server\n");
		exit(-1);
	}

	init_safety_pipes();
	char buff2[DATASIZE]={0};
	initClient(atoi(argv[1]),argv[2]);
	tryConnect(&client_socket);
	timedRead(client_socket,cmd_safety_pipe[0],buff2,DATASIZE,MAXTIMEOUTCONS,MAXTIMEOUTUCONS);
	
	//especificar socket;
	pthread_create(&outputPrinter,NULL,getOutput,NULL);
	pthread_setname_np(outputPrinter,"outputPrinter_remote_shell_client");
	if(!enable_tty_mode){
		pthread_create(&errPrinter,NULL,getErr,NULL);
		pthread_setname_np(errPrinter,"errPrinter_remote_shell_client");
	}
	tryConnect(&output_socket);

	
	if(!enable_tty_mode){
		tryConnect(&err_socket);
	}
	
	acessVarMtx32(&varMtx,&err_alive,!enable_tty_mode,0);
        acessVarMtx32(&varMtx,&out_alive,1,0);
        
	if(!enable_tty_mode){
		pthread_cond_signal(&errCond);
	}
	pthread_cond_signal(&outCond);
	
	pthread_create(&commandPrompt,NULL,command_line_thread,NULL);
        pthread_setname_np(commandPrompt,"commandPrompt_remote_shell_client");
	pthread_create(&cleanupCrew,NULL,cleanup_crew,NULL);
	pthread_setname_np(cleanupCrew,"cleanupCrew_remote_shell_client");

        printf("waiting for session end to reap cleanup crew thread!!\n");
        pthread_join(cleanupCrew,NULL);
	printf("ending client.\n");
	return 0;
}

