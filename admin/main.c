#include "../xtrafun/Includes/preprocessor.h"
#include "../xtrafun/Includes/fileshit.h"

struct sigaction sa;

static int32_t all_alive=1;
static int32_t server_alive=1;
static int32_t out_alive=0;
static int32_t cmd_alive=0;
static pthread_mutex_t varMtx= PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t exitCond= PTHREAD_COND_INITIALIZER;
static pthread_mutex_t exitMtx= PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cmdCond= PTHREAD_COND_INITIALIZER;
static pthread_mutex_t cmdMtx= PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t outCond= PTHREAD_COND_INITIALIZER;
static pthread_mutex_t outMtx= PTHREAD_MUTEX_INITIALIZER;


static int_pair srv_con_pair = {SERVER_TIMEOUT_CON_SEC,SERVER_TIMEOUT_CON_USEC};


static int_pair srv_data_pair = {SERVER_TIMEOUT_DATA_SEC,SERVER_TIMEOUT_DATA_USEC};

static pthread_t outputWritter;
static pthread_t commandPrompt;
#define MAXCLIENTS 10000
struct sockaddr_in server_address;


int master_fd=-1;
int slave_fd=-1;
int pid_shell=-1;
int pid_client=-1;
int32_t server_socket=-1;
int32_t client_socket=-1;
char raw_line[DEF_DATASIZE]={0};
char  outbuff[DEF_DATASIZE]={0};

static void sigint_handler_client(int signal){
	printf("sigint was called in server subprocess!!\n");
	acessVarMtx32(&varMtx,&all_alive,0,0*signal);
	pthread_cond_signal(&exitCond);
}

static void sigpipe_handler_client(int signal){

	printf("sigpipe was called in server subprocess!!\n");
	sigint_handler_client(SIGINT+signal*0);
}
static void sigint_handler_server(int signal){

	printf("sigint was called in server process!!\n");
	server_alive=0*signal;
}
static void sigpipe_handler_server(int signal){

	printf("sigpipe was called in server process!!\n");
	sigint_handler_server(SIGINT+signal*0);
}
static void sigact_sigint_handler_server(int signal){

	printf("sigation for sigchild was called in server process!!\n");
	sigint_handler_server(SIGINT+signal*0);
}

static void switch_all_off(void){

	acessVarMtx32(&varMtx,&all_alive,0,0);
	acessVarMtx32(&varMtx,&out_alive,0,0);
	acessVarMtx32(&varMtx,&cmd_alive,0,0);

}

static void initServer(char* address,int port){



	server_socket= socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(server_socket==-1){
		perror("Nao foi possivel criar server socket!!!\n");
		exit(-1);
	}
	set_sock_reuseaddr(&server_socket,1);
	setLinger(&server_socket,0,0);
	set_sock_recvtimeout(&server_socket,30,0);
	set_sock_sendtimeout(&server_socket,30,0);
	setNonBlocking(&server_socket);
	init_addr(&server_address,address,port);
	if(bind(server_socket,(struct sockaddr*) &server_address,sizeof(server_address))){

		perror("Nao foi possivel dar bind!!!\n");
		close(server_socket);
		exit(-1);
	}
	print_addr_aux("server inicializado no address: ",&server_address);
        listen(server_socket,2*MAXCLIENTS);


}

static void* writeOutput(void* args){

	pthread_mutex_lock(&outMtx);
	while(!acessVarMtx32(&varMtx,&out_alive,0,-1)&&acessVarMtx32(&varMtx,&all_alive,0,-1)){

		pthread_cond_wait(&outCond,&outMtx);

	}
	pthread_mutex_unlock(&outMtx);
	printf("Server's output message channel thread online!!!\n");
	int numread=-1;
	int numsent=-1;
	acessVarMtx32(&varMtx,&cmd_alive,1,0);
	pthread_cond_signal(&cmdCond);
	while(acessVarMtx32(&varMtx,&out_alive,0,-1)&&acessVarMtx32(&varMtx,&all_alive,0,-1)){
		memset(outbuff,0,DEF_DATASIZE);
		numread=readsome(master_fd,outbuff,DEF_DATASIZE,srv_data_pair);
		if(numread<=0){
			break;
		}
		numsent=sendsome(client_socket,outbuff,numread,srv_data_pair);
		if(numsent<=0){
			if(numsent==-2){
				continue;
			}
			else{
				perror("Error or interruption in output sending! Exiting...\n");
				break;
			}

		}
	}
	printf("Server's output message channel thread out!!!\n");
	return args;



}
static void* command_prompt_thread(void* args){
	
	pthread_mutex_lock(&cmdMtx);
	while(!acessVarMtx32(&varMtx,&cmd_alive,0,-1)&&acessVarMtx32(&varMtx,&all_alive,0,-1)){

		pthread_cond_wait(&cmdCond,&cmdMtx);

	}
	pthread_mutex_unlock(&cmdMtx);

	printf("Server's command receiving channel thread about to start!\n");
	memset(raw_line,0,sizeof(raw_line));
	int numread=0;
	int numwritten=0;
	while(acessVarMtx32(&varMtx,&cmd_alive,0,-1)&&acessVarMtx32(&varMtx,&all_alive,0,-1)){
	numread=recvsome(client_socket,raw_line,DEF_DATASIZE,srv_data_pair);
	if(numread<0){
		break;
	}
	numwritten=writesome(master_fd,raw_line,strlen(raw_line),srv_data_pair);
	if(numwritten<0){
		break;
	}
	if(!strncmp(raw_line, "exit",strlen("exit"))&&((strlen(raw_line)-1)==strlen("exit"))){

		printf("The server got orders to exit!\n");
		break;

	}
	memset(raw_line,0,sizeof(raw_line));
	}
	printf("Server's command receiving channel thread about to exit!\n");
	raise(SIGINT);
	return args;

}

void cleanup_crew_client(void){

	pthread_mutex_lock(&exitMtx);
	while(acessVarMtx32(&varMtx,&all_alive,0,-1)){

		pthread_cond_wait(&exitCond,&exitMtx);

	}
	pthread_mutex_unlock(&exitMtx);
	printf("Cleanup crew called in server subprocees\n");
	switch_all_off();
	printf("This server subprocess got orders to exit!\n");
	kill(pid_shell,SIGCHLD);
	printf("We tried to kill process number %d!!!!!\n",pid_shell);
	siginfo_t sig_info;
	printf("Waiting for subprocess children!\n");
	int status=-1;
	while(1){
		status=waitid(P_ALL,-1,&sig_info,WEXITED|WNOHANG);
		if(status>0){
			printf("process of pid: %d\nAnd uid %d\nExited!!!\n",sig_info.si_pid,sig_info.si_uid);
		}
		else{
			printf("Yeeey no more childreeen\n");
			break;
		}
	}
	
	printf("Cleanup crew called in server. Closing file descriptors and sockets\n");


	printf("Cleanup crew called in server. About to join threads which are online\n");
	if(!acessVarMtx32(&varMtx,&cmd_alive,0,-1)){
		printf("reaping server comamnd reader thread...\n");
		pthread_join(commandPrompt,NULL);
		printf("reaped server comamnd reader thread!!\n");
	}
	if(!acessVarMtx32(&varMtx,&out_alive,0,-1)){
		printf("reaping server output writter thread...\n");
		pthread_join(outputWritter,NULL);
		printf("reaped server output writter thread!!\n");
	}
	close(master_fd);
	close(server_socket);
	close(client_socket);
	printf("Finished cleanup in server.\n");

}

static void do_connection(char* shell_name){


	printf("Shell name: %s\n",shell_name);

	char* ptrs[3];
	char pty_name[128]={0};
	openpty(&master_fd,&slave_fd,pty_name,NULL,NULL);
	
	printf("Pty name: %s\n",pty_name);
	long flags= 0;
	flags=fcntl(master_fd,F_GETFL);
	flags |= O_NONBLOCK;
        fcntl(master_fd,F_SETFL,flags);
	pid_shell=fork();
	switch(pid_shell){
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
			char arg0[DEF_DATASIZE]={0};
			snprintf(arg0,DEF_DATASIZE-1,"/bin/%s",shell_name);
			ptrs[0]=arg0;
			char arg1[DEF_DATASIZE]={0};
			snprintf(arg1,DEF_DATASIZE-1,"-i");
			ptrs[1]=arg1;
			ptrs[2]=NULL;
			login_tty(slave_fd);
			execvp(ptrs[0], ptrs);
			exit(-1);
		default:
			printf("We just spawned shell process of pid number %d!!!!!\n",pid_shell);
			close(slave_fd);
			break;
	}
	
	pthread_create(&commandPrompt,NULL,command_prompt_thread,NULL);
	pthread_setname_np(commandPrompt,"commandPrompt_remote_shell_server");

	signal(SIGINT,sigint_handler_client);
	signal(SIGPIPE,sigpipe_handler_client);

	acessVarMtx32(&varMtx,&out_alive,1,0);

        pthread_cond_signal(&outCond);

	pthread_create(&outputWritter,NULL,writeOutput,NULL);
	pthread_setname_np(outputWritter,"outputWritter_remote_shell_server");


	cleanup_crew_client();


}

static void accept_connections(char* shell_name){

	while(server_alive){
		
		int iResult;
                struct timeval tv;
                fd_set rfds;
                FD_ZERO(&rfds);
                FD_SET(server_socket,&rfds);
                tv.tv_sec=srv_con_pair[0];
                tv.tv_usec=srv_con_pair[1];
                iResult=select(server_socket+1,&rfds,(fd_set*)0,(fd_set*)0,&tv);
	        if(iResult>0){
			struct sockaddr_in addr_con={0};
		        if(client_socket<0){
				printf("Esperando conexão de cliente\n");
				client_socket=accept(server_socket,(struct sockaddr *)&addr_con,&socklenvar[0]);
			}
			if(client_socket<0){
		                perror("Erro no accept (cliente)");
		                continue;
		        }
		        setNonBlocking(&client_socket);
		        print_addr_aux("Coneccao (client) de: ",&addr_con);
			pid_client=fork();
			switch(pid_client){
				case -1:
					close(client_socket);
					raise(SIGINT);
					exit(-1);
				case 0:
					close(server_socket);
					do_connection(shell_name);
					exit(-1);
				default:
					printf("We just spawned connection process of pid number %d!!!!!\n",pid_client);
					close(client_socket);
					client_socket=-1;
					break;
			}
		}
		else if(!iResult){
			print_addr_aux("No client connected!!!!\n",&server_address);
		}
		else if(errno!=EINTR){
			perror("Select error in server connection accepting\n");
			raise(SIGINT);
		}
	}
	printf("A fechar a loja!!!\n");
	close(server_socket);
	printf("Loja fechada\n");

}
int main(int argc, char ** argv){
	
	if(argc!=4){

		printf("arg1: address\narg2: porta do server\narg3: shell to use\n");
		exit(-1);
	}
	logstream=stderr;
	logging=1;
	initServer(argv[1],atoi(argv[2]));
	sa.sa_handler=sigact_sigint_handler_server;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags=SA_RESTART|SA_NOCLDWAIT;

        sigaction(SIGCHLD, &sa, NULL);

	signal(SIGINT,sigint_handler_server);
	signal(SIGPIPE,sigpipe_handler_server);
	accept_connections(argv[3]);
        printf("ending server.\n");
	return 0;
}
