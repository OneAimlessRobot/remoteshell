#include "Includes/preprocessor.h"
static const char* ping= "gimmemore!";
static char* module_name= "CLIENT";
static int enable_tty_mode=0;
static const u_int64_t android_comp_mode_on=0;
static const u_int32_t one_for_detach_zero_for_join=0;
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


static pthread_t commandPrompt;
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

        printf("sigint was called in client!!\n");
        acessVarMtx32(&varMtx,&all_alive,0,0*signal);
        pthread_cond_signal(&exitCond);
}

static void sigpipe_handler(int signal){

        printf("sigpipe was called in server!!\n");
        raise(SIGINT+signal*0);
}
static void init_safety_pipes(void){

if(!enable_tty_mode){
        create_safety_pipe(err_safety_pipe,"Error  safety pipe",module_name,O_NONBLOCK);
}

create_safety_pipe(out_safety_pipe,"Output printing safety pipe",module_name,O_NONBLOCK);
create_safety_pipe(cmd_safety_pipe,"command receiving safety pipe",module_name,O_NONBLOCK);
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
	        //flags |= O_NONBLOCK;
	        fcntl(client_socket,F_SETFD,flags);

		flags= fcntl(output_socket,F_GETFL);
	        //flags |= O_NONBLOCK;
	        fcntl(output_socket,F_SETFD,flags);
	}

	
	if(!enable_tty_mode&&!android_comp_mode_on){
		flags= fcntl(err_socket,F_GETFL);
	        //flags |= O_NONBLOCK;
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

void cleanup_crew(void){

        pthread_mutex_lock(&exitMtx);
        while(acessVarMtx32(&varMtx,&all_alive,0,-1)){

                pthread_cond_wait(&exitCond,&exitMtx);

        }
        pthread_mutex_unlock(&exitMtx);
        printf("Cleanup crew called in client\n");
        printf("Cleanup crew called in client. Closing file descriptors and sockets\n");
        fflush(stdout);
        if(!enable_tty_mode){
                fflush(stderr);
        }
	printf("Cleanup crew called in client. About to join threads which are online\n");
        if(!acessVarMtx32(&varMtx,&cmd_alive,0,-1)||!acessVarMtx32(&varMtx,&all_alive,0,-1)){
                printf("Reaping client cmdline thread!!\n");
		if(!one_for_detach_zero_for_join){
	                safety_close("Command sending socket",cmd_safety_pipe[1]);
			pthread_join(commandPrompt,NULL);
        		close(client_socket);
		}
		else{

		        pthread_detach(commandPrompt);
        	}

	}
        if(!acessVarMtx32(&varMtx,&out_alive,0,-1)||!acessVarMtx32(&varMtx,&all_alive,0,-1)){
                printf("Reaping client output writter thread!!\n");
                if(!one_for_detach_zero_for_join){
	                safety_close("Output message receiving socket",out_safety_pipe[1]);
			pthread_join(outputPrinter,NULL);
        		close(output_socket);
		}
		else{
		        pthread_detach(outputPrinter);
        	}
	}
        if(!enable_tty_mode){

		if(!acessVarMtx32(&varMtx,&err_alive,0,-1)||!acessVarMtx32(&varMtx,&all_alive,0,-1)){
                	printf("Reaping client error printer thread!!\n");
                	if(!one_for_detach_zero_for_join){
	                	safety_close("Error sending socket",err_safety_pipe[1]);
				pthread_join(errPrinter,NULL);
        			close(err_socket);
			}
			else{
				pthread_detach(errPrinter);
			}
		}
	}


	printf("Finished cleanup in client.\n");
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
	sscanf(buff2,"%d",&enable_tty_mode);
	printf("enable_tty_mode: %d\n",enable_tty_mode);
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

	cleanup_crew();
	printf("ending client.\n");
	return 0;
}

