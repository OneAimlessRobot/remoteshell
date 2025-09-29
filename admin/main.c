#include "../xtrafun/Includes/preprocessor.h"

static int32_t all_alive=1;
static int32_t out_alive=0;
static int32_t cmd_alive=0;
static pthread_mutex_t varMtx= PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t exitCond= PTHREAD_COND_INITIALIZER;
static pthread_mutex_t exitMtx= PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cmdCond= PTHREAD_COND_INITIALIZER;
static pthread_mutex_t cmdMtx= PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t outCond= PTHREAD_COND_INITIALIZER;
static pthread_mutex_t outMtx= PTHREAD_MUTEX_INITIALIZER;

static pthread_t outputWritter;
static pthread_t commandPrompt;

struct sockaddr_in server_address;


int master_fd=-1;
int slave_fd=-1;
int pid=0;
int32_t server_socket,client_socket,output_socket;
char raw_line[DATASIZE]={0};
char  outbuff[DATASIZE]={0};

static void sigint_handler(int signal){
	printf("sigint was called in server %d!!\n",signal);
	acessVarMtx32(&varMtx,&all_alive,0,0);
	pthread_cond_signal(&exitCond);
}

static void sigpipe_handler(int signal){

	printf("sigpipe was called in server!!\n");
	raise(SIGINT+signal*0);
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
	fcntl(server_socket,F_SETFL,O_NONBLOCK);
	init_addr(&server_address,address,port);
	if(bind(server_socket,(struct sockaddr*) &server_address,sizeof(server_address))){

		perror("Nao foi possivel dar bind!!!\n");
		close(server_socket);
		exit(-1);
	}
	print_addr_aux("server inicializado no address: ",&server_address);
        listen(server_socket,3);


}

void setupConnections(void){


	acceptConnection(&client_socket);

	acceptConnection(&output_socket);
	long flags=0;
	flags= fcntl(client_socket,F_GETFL);
	flags |= O_NONBLOCK;
        fcntl(client_socket,F_SETFL,flags);

	flags= fcntl(output_socket,F_GETFL);
	flags |= O_NONBLOCK;
        fcntl(output_socket,F_SETFL,flags);

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
		int numsent=1;
		while(acessVarMtx32(&varMtx,&out_alive,0,-1)&&acessVarMtx32(&varMtx,&all_alive,0,-1)){
		numread=timedRead(master_fd,outbuff,DATASIZE,MAXTIMEOUTCMD,MAXTIMEOUTCMD);
		if(numread<0){
			break;
		}
		numread=timedSend(output_socket,outbuff,DATASIZE,MAXTIMEOUTSECS,MAXTIMEOUTUSECS);
		if(numsent<0){
			break;
		}
		memset(outbuff,0,DATASIZE);
		}

	}
	acessVarMtx32(&varMtx,&out_alive,0,0);
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
	while(acessVarMtx32(&varMtx,&cmd_alive,0,-1)&&acessVarMtx32(&varMtx,&all_alive,0,-1)){
	memset(raw_line,0,sizeof(raw_line));
	int numread=0;
	int numwritten=0;
	while(acessVarMtx32(&varMtx,&cmd_alive,0,-1)&&acessVarMtx32(&varMtx,&all_alive,0,-1)){
	numread=timedRead(client_socket,raw_line,DATASIZE,MAXTIMEOUTCMD,MAXTIMEOUTUCMD);
	if(numread<0){

		break;
	}
	numwritten=write(master_fd,raw_line,strlen(raw_line));
	if(numwritten<0){

		break;
	}
	if(!strncmp(raw_line, "exit",strlen("exit"))&&((strlen(raw_line)-1)==strlen("exit"))){

		printf("The server got orders to exit!\n");
		acessVarMtx32(&varMtx,&all_alive,0,0);
		break;

	}
	memset(raw_line,0,sizeof(raw_line));
	}
	}
	acessVarMtx32(&varMtx,&cmd_alive,0,0);
	printf("Server's command receiving channel thread about to exit!\n");
	raise(SIGINT);
	return args;

}

void cleanup_crew(void){

	pthread_mutex_lock(&exitMtx);
	while(acessVarMtx32(&varMtx,&all_alive,0,-1)){

		pthread_cond_wait(&exitCond,&exitMtx);

	}
	pthread_mutex_unlock(&exitMtx);
	printf("Cleanup crew called in server\n");
	printf("The server got orders to exit!\n");
	kill(pid,SIGTERM);
	print_sock_addr(client_socket);
	waitpid(0,NULL,0);

	printf("Cleanup crew called in server. Closing file descriptors and sockets\n");


	printf("Cleanup crew called in server. About to join threads which are online\n");
	
	printf("reaping server output writter thread!!\n");
	pthread_join(commandPrompt,NULL);
	printf("reaping server output writter thread!!\n");
	pthread_join(outputWritter,NULL);

	close(master_fd);
	close(server_socket);
	close(client_socket);
	close(output_socket);

	printf("Finished cleanup in server.\n");

}


int main(int argc, char ** argv){
	
	if(argc!=4){

		printf("arg1: address\narg2: porta do server\narg3: shell to use\n");
		exit(-1);
	}
	initServer(argv[1],atoi(argv[2]));
	setupConnections();

	printf("Shell name: %s\n",argv[3]);

	char* ptrs[3];
	char pty_name[128]={0};
	openpty(&master_fd,&slave_fd,pty_name,NULL,NULL);
	
	printf("Pty name: %s\n",pty_name);
	long flags= 0;
	flags=fcntl(master_fd,F_GETFL);
	//flags |= O_NONBLOCK;
        fcntl(master_fd,F_SETFL,flags);
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
			char arg0[DATASIZE]={0};
			snprintf(arg0,DATASIZE-1,"/bin/%s",argv[3]);
			ptrs[0]=arg0;
			char arg1[DATASIZE]={0};
			snprintf(arg1,DATASIZE-1,"-i");
			ptrs[1]=arg1;
			ptrs[2]=NULL;
			login_tty(slave_fd);
			execvp(ptrs[0], ptrs);
			exit(-1);
		default:
			close(slave_fd);
			break;
	}
	
	pthread_create(&commandPrompt,NULL,command_prompt_thread,NULL);
	pthread_setname_np(commandPrompt,"commandPrompt_remote_shell_server");

	signal(SIGINT,sigint_handler);
	signal(SIGPIPE,sigpipe_handler);

	acessVarMtx32(&varMtx,&out_alive,1,0);

        pthread_cond_signal(&outCond);

	pthread_create(&outputWritter,NULL,writeOutput,NULL);
	pthread_setname_np(outputWritter,"outputWritter_remote_shell_server");


	cleanup_crew();
        printf("ending server.\n");
	return 0;
}
