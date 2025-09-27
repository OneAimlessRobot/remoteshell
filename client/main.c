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
#define MAXNUMBEROFTRIES 10

#define MAXTIMEOUTSECS 1
#define MAXTIMEOUTUSECS 0

#define MAXTIMEOUTCMD 1
#define MAXTIMEOUTUCMD 0

#define MAXTIMEOUTCONS (60*5)
//#define MAXTIMEOUTCONS 1
#define MAXTIMEOUTUCONS 0

#define MAXTIMEOUTPING 1
#define MAXTIMEOUTUPING 0

#define LINESIZE 1024
#define DATASIZE 1024


static pthread_t commandPrompt;
static pthread_t cleanupCrew;
static pthread_t connectionChecker;
static pthread_t outputPrinter;
static pthread_t errPrinter;
char outbuff[LINESIZE*10]={0};
char errbuff[LINESIZE*10]={0};
char line[LINESIZE]={0};
char raw_line[LINESIZE]={0};

static struct sockaddr_in server_address;
static struct sockaddr_in server_ack_address;
static struct sockaddr_in client_ack_address;

u_int16_t server_ack_port=-1;
u_int16_t client_ack_port=-1;

//on example_pipe[2]...
//write to example_pipe[1]
//read from example_pipe[0]


int err_safety_pipe[2];
int out_safety_pipe[2];
int cmd_safety_pipe[2];
int lifeline_safety_pipe[2];



int client_socket,lifeline_socket, output_socket;
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

if(pipe2(lifeline_safety_pipe,O_NONBLOCK)){

	perror("Safety pipe on lifeline message channel on client failed to open! Aborting");
	exit(-1);
}

if((lifeline_safety_pipe[0]=dup(lifeline_safety_pipe[0]))<0){

	perror("Safety pipe on lifeline message channel on client failed to be duped on read end!!\nAborting");
	exit(-1);


}
if((lifeline_safety_pipe[1]=dup(lifeline_safety_pipe[1]))<0){

	perror("Safety pipe on lifeline message channel on client failed to be duped on read end!!\nAborting");
	exit(-1);


}
printf("just created lifeline message channel safety pipe in client\n");

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

static void safety_close(int safety_fd_write){


	write(safety_fd_write,"x",1);


}
static void safety_close(char* prompt_to_show,int safety_fd_write){

        printf("We are trying to close the fd of the following description safely:\n%s\n",prompt_to_show);
        write(safety_fd_write,"x",1);

}

static void build_ack_addresses(void){


	getpeername(client_socket,(struct sockaddr*)&server_ack_address,&socklenvar[0]);
	getpeername(client_socket,(struct sockaddr*)&client_ack_address,&socklenvar[0]);
	server_ack_address.sin_port=server_ack_port;
	client_ack_address.sin_port=client_ack_port;

        if(bind(lifeline_socket,(struct sockaddr*) &client_ack_address,sizeof(client_ack_address))){

	        perror("Nao foi possivel dar bind!!!\n");
	        close(lifeline_socket);
	        close(client_socket);
	        exit(-1);
	}
	print_addr_aux("Address de acks de client (bind): ",&client_ack_address);
	print_addr_aux("Address de acks de server (peer): ",&server_ack_address);
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

                        printf("reading from safety fd in the client's timedSendUDP function!!! %s\n",drain_buff);
                        close(safety_fd);
                        close(fd);
                        return -1;
                }
                //printf("just read from pipe or not\n");
                return sendto(fd,buff,size,0,&server_ack_address,socklenvar[1]);
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

                        printf("reading from safety fd in the client's timedReadUDP function!!! %s\n",drain_buff);
                        close(safety_fd);
                        close(fd);
                        return -1;
                }
                //printf("just read from pipe or not\n");
                return recvfrom(fd,buff,size,0,&server_ack_address,&socklenvar[1]);
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
	lifeline_socket= socket(AF_INET,SOCK_DGRAM,0);
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
	int numread=1;
	memset(buff,0,pingLength);
	
	while(acessVarMtx32(&varMtx,&ping_alive,0,-1)&&acessVarMtx32(&varMtx,&all_alive,0,-1)){
		while(acessVarMtx32(&varMtx,&ping_alive,0,-1)&&acessVarMtx32(&varMtx,&ping_alive,0,-1)&&((numread=timedSendUDP(lifeline_socket,lifeline_safety_pipe[0],buff,pingLength,MAXTIMEOUTPING,MAXTIMEOUTUPING))>0)){
			if(timedReadUDP(lifeline_socket,lifeline_safety_pipe[0],buff,pingLength,MAXTIMEOUTPING,MAXTIMEOUTUPING)<=0){
				acessVarMtx32(&varMtx,&all_alive,0,0);
				break;
			}
		}
		acessVarMtx32(&varMtx,&all_alive,0,0);
		break;
	}
	acessVarMtx32(&varMtx,&ping_alive,0,0);
	printf("Client's ping channel thread exiting!\n");
	sigint_handler(0);
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
		//if(!strncmp(line, "exit",strlen(line)-enable_tty_mode)&&((strlen(line)-enable_tty_mode)==strlen("exit"))){
		if(!strncmp(line, "exit",strlen("exit"))&&((strlen(raw_line))==strlen("exit"))){
			acessVarMtx32(&varMtx,&all_alive,0,0);
			break;
		}
		timedSend(client_socket,cmd_safety_pipe[0],line,LINESIZE,MAXTIMEOUTCMD,MAXTIMEOUTUCMD);
	}
	
	acessVarMtx32(&varMtx,&cmd_alive,0,0);
	printf("Client's command sending channel thread exiting!\n");
	raise(SIGINT);
	


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
	        if(!enable_tty_mode){
                safety_close(err_safety_pipe[1]);
                fflush(stderr);
        }

        printf("Cleanup crew called in client. Closing file descriptors and sockets\n");
        fflush(stdout);
        safety_close(cmd_safety_pipe[1]);
	safety_close(lifeline_safety_pipe[1]);
        safety_close(out_safety_pipe[1]);
	
	printf("Finished cleanup in client.\n");
        pthread_mutex_unlock(&cleanupMtx);
        return args;

}

int main(int argc, char ** argv){

	if(argc!=4){

		printf("Utilizacao correta: arg1: ip de server a connectar.\narg2: porta de server\narg3: porta de client para acks udp\n");
		exit(-1);
	}

	init_safety_pipes();
	client_ack_port=atoi(argv[3]);
	char buff2[DATASIZE]={0};
	initClient(atoi(argv[1]),argv[2]);
	tryConnect(&client_socket);
	timedRead(client_socket,cmd_safety_pipe[0],buff2,DATASIZE,MAXTIMEOUTCONS,MAXTIMEOUTUCONS);
	sscanf(buff2,"%d %hu",&enable_tty_mode,&server_ack_port);
	server_ack_port=ntohs(server_ack_port);
	printf("Enable tty mode: %d\nServer ack port: %hu\n",enable_tty_mode,server_ack_port);
	char buff3[DATASIZE]={0};
	snprintf(buff3,sizeof(buff3)-1,"%d",htons(client_ack_port));
	//snprintf(buff3,sizeof(buff3)-1,"%d",client_ack_port);
	send(client_socket,buff3,DATASIZE,0);
	printf("Send de ping feito!\nPorta enviada: %hu\nhtonl de porta enviada! %hu\nntohl de porta enviada %hu\n",client_ack_port,htons(client_ack_port),ntohs(client_ack_port));

	build_ack_addresses();

	//especificar socket;
	pthread_create(&connectionChecker,NULL,areYouStillThere,NULL);
	pthread_setname_np(connectionChecker,"connectionChecker_remote_shell_client");
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
	
	pthread_create(&commandPrompt,NULL,command_line_thread,NULL);
        pthread_setname_np(commandPrompt,"commandPrompt_remote_shell_client");
	pthread_create(&cleanupCrew,NULL,cleanup_crew,NULL);
	pthread_setname_np(cleanupCrew,"cleanupCrew_remote_shell_client");

        acessVarMtx32(&varMtx,&ping_alive,1,0);
        pthread_cond_signal(&pingCond);
	printf("waiting for session end to reap cleanup crew thread!!\n");
        pthread_join(cleanupCrew,NULL);
	printf("ending client.\n");
	return 0;
}

