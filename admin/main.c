#include "Includes/preprocessor.h"
socklen_t socklenvar[2]= {sizeof(struct sockaddr),sizeof(struct sockaddr_in)};

static const char* ping= "gimmemore!";

static u_int64_t alive=1;
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
static pthread_t connectionChecker;
static pthread_t outputWritter;
static pthread_t errWritter;
int outpipe[2];
int errpipe[2];
int inpipe[2];

/*int master_fd=-1;
int slave_fd=-1;*/

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

        printf("sigint was called in server!!\n");
	acessVarMtx(&varMtx,&alive,0,0);
        pthread_join(connectionChecker,NULL);
	pthread_join(outputWritter,NULL);
	pthread_join(errWritter,NULL);
	close(server_socket);
	close(client_socket);
        close(lifeline_socket);
        close(output_socket);
	close(err_socket);
	//close(master_fd);
	close(outpipe[0]);
	close(outpipe[1]);
	close(errpipe[0]);
	close(errpipe[1]);
	close(inpipe[0]);
	close(inpipe[1]);
	fflush(stdout);
	fflush(stderr);
	close(0);
	close(1);
	close(2);
	exit(-1+signal*0);
}

static void sigpipe_handler(int signal){

	raise(SIGINT+signal*0);
}

static void initpipes(void){


if(pipe(outpipe)){
perror("Erro a criar pipe de stdout!!!\n");
raise(SIGINT);
}
dup2(outpipe[1],1);

long flags=fcntl(outpipe[0],F_GETFL);
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
dup2(inpipe[1],0);
flags=fcntl(inpipe[0],F_GETFL);
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


if(pipe(errpipe)){

perror("Erro a criar pipe de stderr!!!\n");
raise(SIGINT);
}
dup2(errpipe[1],2);
flags=fcntl(errpipe[0],F_GETFL);
flags |=O_NONBLOCK;
fcntl(errpipe[0],F_SETFL,flags);


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
	}
	printf("Coneccao de %s!!!!!!\n",inet_ntoa(server_address.sin_addr));
	
}
static void initServer(char* address,int port){


	
	server_socket= socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(server_socket==-1){
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
	fcntl(server_socket,F_SETFL,O_NONBLOCK);
	
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
                return -1;
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
                return -1;
}
static void* areYouStillThere(void* args){
	
        int pingLength=strlen(ping);
	char buff[strlen(ping)];
        memset(buff,0,pingLength);
	
	receiveClientInput(lifeline_socket,buff,pingLength,MAXTIMEOUTPING,MAXTIMEOUTUPING);
	while(acessVarMtx(&varMtx,&alive,0,-1)){

		send(lifeline_socket,ping,strlen(ping),0);
		int status=receiveClientInput(lifeline_socket,buff,strlen(ping),MAXTIMEOUTPING,MAXTIMEOUTUPING);
                if(status<0){
                        perror("client did not respond!!!\n");
			raise(SIGINT);
                }
		memset(buff,0,pingLength);



	}
	return args;

}

static void* writeOutput(void* args){
	while(acessVarMtx(&varMtx,&alive,0,-1)){
		char buff[strlen(ping)];
		int numread=1;
		while((numread=timedRead(outpipe[0],outbuff,DATASIZE,MAXTIMEOUTSECS,MAXTIMEOUTUSECS))>0){
		//while((numread=timedRead(master_fd,outbuff,DATASIZE,MAXTIMEOUTSECS,MAXTIMEOUTUSECS))>0){
		send(output_socket,outbuff,DATASIZE,0);
		receiveClientInput(output_socket,buff,sizeof(buff),MAXTIMEOUTSECS,MAXTIMEOUTUSECS);
		memset(outbuff,0,DATASIZE);
		}
		
		

	}
	return args;



}

static void* writeErr(void* args){
	while(acessVarMtx(&varMtx,&alive,0,-1)){
		char buff[strlen(ping)];
		int numread=1;
		
		while((numread=timedRead(errpipe[0],errbuff,DATASIZE,MAXTIMEOUTSECS,MAXTIMEOUTUSECS))>0){
		write(err_socket,errbuff,DATASIZE);
		receiveClientInput(err_socket,buff,sizeof(buff),MAXTIMEOUTSECS,MAXTIMEOUTUSECS);
		memset(errbuff,0,DATASIZE);
		}
		

	}
	return args;



}
void setupConnections(void){


	acceptConnection(&client_socket);
 
	acceptConnection(&lifeline_socket);
	
	acceptConnection(&output_socket);
 	
	acceptConnection(&err_socket);
	
	long flags= fcntl(client_socket,F_GETFL);
	flags |= O_NONBLOCK|O_ASYNC;
        fcntl(client_socket,F_SETFD,flags);
	
	flags= fcntl(lifeline_socket,F_GETFL);
	flags |= O_NONBLOCK;
        fcntl(lifeline_socket,F_SETFD,flags);
	
	flags= fcntl(output_socket,F_GETFL);
	flags |= O_NONBLOCK;
        fcntl(output_socket,F_SETFD,flags);
	
	
	flags= fcntl(err_socket,F_GETFL);
	flags |= O_NONBLOCK;
        fcntl(err_socket,F_SETFD,flags);
	
	char sizes[10]={0};
	snprintf(sizes,10,"%d",DATASIZE);
	send(client_socket,sizes,10,0);
	char buff[strlen(ping)];
	memset(buff,0,strlen(ping));
	receiveClientInput(client_socket,buff,strlen(ping),MAXTIMEOUTCONS,MAXTIMEOUTUCONS);
	

}
int main(int argc, char ** argv){
	
	if(argc!=3){

		printf("arg1: address\narg1: porta do server\n");
		exit(-1);
	}
	

	initServer(argv[1],atoi(argv[2]));
	
	setupConnections();


	initpipes();
	
	pthread_create(&connectionChecker,NULL,areYouStillThere,NULL);
	
	pthread_create(&outputWritter,NULL,writeOutput,NULL);

	pthread_create(&errWritter,NULL,writeErr,NULL);

	int pid= fork();
	switch(pid){
		case -1:
			exit(-1);
		case 0:
			dup2(inpipe[0], STDIN_FILENO);
		        close(inpipe[0]);
		        close(inpipe[1]);
			char *ptrs[3];
			ptrs[0] = "/bin/bash";
			ptrs[1] = "-i";
			ptrs[2] = NULL;
			execvp(ptrs[0], ptrs);
			exit(-1);
		default:
			close(inpipe[0]);
			break;
	}
	
	//dup2(inpipe[0], STDIN_FILENO);
		        //close(inpipe[0]);
		        //close(inpipe[1]);
	/*char* ptrs[3];
	char pty_name[128]={0};
	openpty(&master_fd,&slave_fd,pty_name,NULL,NULL);
	printf("Pty name: %s\n",pty_name);
	long flags= fcntl(master_fd,F_GETFL);
	flags |= O_NONBLOCK|O_ASYNC;
        fcntl(master_fd,F_SETFD,flags);

	int pid=fork();
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
			ptrs[0] = "/bin/bash";
			ptrs[1] = "-i";
			ptrs[2] = NULL;
			login_tty(slave_fd);
			execvp(ptrs[0], ptrs);
			exit(-1);
		default:
			close(slave_fd);
			break;
	}
	*/
	while(acessVarMtx(&varMtx,&alive,0,-1)){
	memset(line,0,sizeof(line));
	while(receiveClientInput(client_socket,line,LINESIZE,MAXTIMEOUTCMD,MAXTIMEOUTUCMD)>0){

	char buff[strlen(ping)+1];
	memset(buff,0,strlen(ping)+1);
	memcpy(buff,ping,strlen(ping));
	line[strlen(line)]='\n';
	write(inpipe[1],line,LINESIZE);
	//write(master_fd,line,LINESIZE);
	if(!strncmp(line, "exit",strlen(line)-1)&&((strlen(line)-1)==strlen("exit"))){

		printf("we got orders to exit!\n");
		print_sock_addr(client_socket);
		waitpid(-1,NULL,0);
		raise(SIGINT);

	}
	fflush(stdout);
	fflush(stderr);
	send(client_socket,buff,sizeof(buff),0);
	}
	}
	raise(SIGINT);
	return 0;
}

