#include "../xtrafun/Includes/preprocessor.h"

static const char* ping= "gimmemore!";
static char* module_name= "server";
static int enable_tty_mode=0;
//static int32_t all_ready=0;
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
/*static pthread_cond_t readyCond= PTHREAD_COND_INITIALIZER;
static pthread_mutex_t readyMtx= PTHREAD_MUTEX_INITIALIZER;
*/
static const u_int64_t android_comp_mode_on=0;
static const u_int32_t one_for_detach_zero_for_join=0;

static pthread_t outputWritter;
static pthread_t errWritter;
static pthread_t commandPrompt;
int outpipe[2];
int errpipe[2];
int inpipe[2];

struct sockaddr_in server_address;



//on example_pipe[2]...
//write to example_pipe[1]
//read from example_pipe[0]


int err_safety_pipe[2];
int out_safety_pipe[2];
int cmd_safety_pipe[2];
int master_fd_safety_pipe[2];

int master_fd=-1;
int slave_fd=-1;
int pid=0;
int32_t server_socket,client_socket,output_socket;
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


}

void setupConnections(void){


	acceptConnection(&output_socket);
	long flags=0;
	if(!android_comp_mode_on){
		flags= fcntl(client_socket,F_GETFL);
		//flags |= O_NONBLOCK;
	        fcntl(client_socket,F_SETFD,flags);

		flags= fcntl(output_socket,F_GETFL);
		//flags |= O_NONBLOCK;
	        fcntl(output_socket,F_SETFD,flags);
	}
 	
	if(!enable_tty_mode){
		acceptConnection(&err_socket);
		if(!android_comp_mode_on){
			flags= fcntl(err_socket,F_GETFL);
			//flags |= O_NONBLOCK;
	        	fcntl(err_socket,F_SETFD,flags);
		}
	}

}
static void init_safety_pipes(void){

if(!enable_tty_mode){
        create_safety_pipe(err_safety_pipe,"Error  safety pipe",module_name,O_NONBLOCK);
}

create_safety_pipe(out_safety_pipe,"Output printing safety pipe",module_name,O_NONBLOCK);
create_safety_pipe(cmd_safety_pipe,"command receiving safety pipe",module_name,O_NONBLOCK);
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
		while(acessVarMtx32(&varMtx,&out_alive,0,-1)&&acessVarMtx32(&varMtx,&all_alive,0,-1)&&((numread=read(enable_tty_mode?master_fd:outpipe[0],outbuff,DATASIZE))>=0)){
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
	while(acessVarMtx32(&varMtx,&cmd_alive,0,-1)&&acessVarMtx32(&varMtx,&all_alive,0,-1)&&(read(client_socket,raw_line,LINESIZE)>0)){

	
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
	waitpid(pid,NULL,0);

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

	printf("Cleanup crew called in server. About to join threads which are online\n");
	if(!acessVarMtx32(&varMtx,&cmd_alive,0,-1)||!acessVarMtx32(&varMtx,&all_alive,0,-1)){
		printf("reaping server cmdline thread!!\n");
		if(!one_for_detach_zero_for_join){
			safety_close("Command receiving socket",cmd_safety_pipe[1]);
			pthread_join(commandPrompt,NULL);
			close(client_socket);
		}
		else{
			pthread_detach(commandPrompt);
		}
	}
	if(!acessVarMtx32(&varMtx,&out_alive,0,-1)||!acessVarMtx32(&varMtx,&all_alive,0,-1)){
		printf("reaping server output writter thread!!\n");
		if(!one_for_detach_zero_for_join){
			safety_close("Output message sending socket",out_safety_pipe[1]);
			pthread_join(outputWritter,NULL);
			close(output_socket);
		}
		else{
			pthread_detach(outputWritter);
		}
	}
	if(!enable_tty_mode){

		if(!acessVarMtx32(&varMtx,&err_alive,0,-1)||!acessVarMtx32(&varMtx,&all_alive,0,-1)){
			printf("reaping server error printer thread!!\n");
			if(!one_for_detach_zero_for_join){
				safety_close("Error message sending socket",err_safety_pipe[1]);
				pthread_join(errWritter,NULL);
				close(err_socket);
			}
			else{
				pthread_detach(errWritter);
			}
		}
	}
	printf("Finished cleanup in server.\n");

}


int main(int argc, char ** argv){
	
	if(argc!=5){

		printf("arg1: address\narg2: porta do server\narg4: shell to use\narg5: 0/1 = (dis)/(en)able tty mode (experimental)\n");
		exit(-1);
	}
	enable_tty_mode=atoi(argv[4]);
	if((enable_tty_mode<0)||(enable_tty_mode>1)){

		printf("arg4 so pode ser 0 ou 1!\n");
		exit(-1);
	}
	init_safety_pipes();
	initServer(argv[1],atoi(argv[2]));
	acceptConnection(&client_socket);
	char sizes[DATASIZE]={0};
	snprintf(sizes,10,"%d",enable_tty_mode);
	send(client_socket,sizes,DATASIZE,0);
	char buff[DATASIZE]={0};
	timedRead(client_socket,cmd_safety_pipe[1],buff,DATASIZE,MAXTIMEOUTCONS,MAXTIMEOUTUCONS);
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
	pthread_setname_np(commandPrompt,"commandPrompt_remote_shell_server");

	signal(SIGINT,sigint_handler);
	signal(SIGPIPE,sigpipe_handler);

	acessVarMtx32(&varMtx,&err_alive,!enable_tty_mode,0);
        acessVarMtx32(&varMtx,&out_alive,1,0);

        if(!enable_tty_mode){
                pthread_cond_signal(&errCond);
        }
        pthread_cond_signal(&outCond);

	cleanup_crew();
        printf("ending server.\n");
	return 0;
}
