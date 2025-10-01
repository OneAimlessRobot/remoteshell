#include "../xtrafun/Includes/preprocessor.h"
#include "../xtrafun/Includes/fileshit.h"


static char wbuf[DEF_DATASIZE*4*1000]={0};

typedef int (*format_func)(const char*,...);

static pthread_mutex_t ncurses_mtx= PTHREAD_MUTEX_INITIALIZER;

static int enable_ncurses=1;

void mtx_protected_print(const char* str,...){
	pthread_mutex_lock(&ncurses_mtx);
	va_list args;
	va_start(args, str);
	vsnprintf(wbuf, sizeof(wbuf), str, args);
	va_end(args);
	write(STDOUT_FILENO,wbuf,strlen(wbuf));
	pthread_mutex_unlock(&ncurses_mtx);


}
static void endwin_wrapper(void){

        pthread_mutex_lock(&ncurses_mtx);
        if(enable_ncurses&&!isendwin()){
                printf("Chamamos endwin!!!!\n");
		endwin();
        }
        pthread_mutex_unlock(&ncurses_mtx);

}
static void send_exit_cmd_to_server(void){

	char buff[sizeof(raw_line)]={0};
	snprintf(buff,sizeof(buff)-1,"exit\n");
	sendsome(client_socket,buff,DEF_DATASIZE,clnt_data_pair);

}

static void sigint_handler(int signal){

	mtx_protected_print("sigint was called in client!!\n");
	acessVarMtx32(&varMtx,&all_alive,0,0*signal);
        pthread_cond_signal(&exitCond);
}

static void sigpipe_handler(int signal){

        mtx_protected_print("sigpipe was called in client!!\n");
        raise(SIGINT+signal*0);
}


static void initClient(char* addr,int port){

	client_socket= socket(AF_INET,SOCK_STREAM,0);
	if(client_socket==-1){
		raise(SIGINT);
	}
	setNonBlocking(&client_socket);
	signal(SIGINT,sigint_handler);
	signal(SIGPIPE,sigpipe_handler);
	init_addr(&server_address,addr,port);


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
		if((errno==EINPROGRESS)||(errno==EAGAIN)){

			fprintf(stdout,"Tentando de novo!\nErro normal: %s\n Erro Socket: %s\nNumero socket: %d\n",strerror(errno),strerror(sockerr),*socket);
			continue;

		}
		fd_set wfds;
		FD_ZERO(&wfds);
		FD_SET((*socket),&wfds);

                struct timeval t;
		t.tv_sec=clnt_con_pair[0];
		t.tv_usec=clnt_con_pair[1];
		int iResult=select((*socket)+1,0,&wfds,0,&t);

		if(iResult>0&&!success&&((*socket)!=-1)){
			fprintf(stdout,"breaking out of connection loop!!!\n");
        		break;

		}
		fprintf(stderr,"Não foi possivel:\nErro normal:%s\n Erro Socket%s\nNumero socket: %d\n",strerror(errno),strerror(sockerr),*socket);
        }
        if(!numOfTries){
        mtx_protected_print("Não foi possivel conectar. Numero limite de tentativas (%d) atingido!!!\n",MAXNUMBEROFTRIES);
        raise(SIGINT);
        }
	fprintf(stdout,"Sucessfully connected!!!\n");
}
static void* getOutput(void* args){
	
	pthread_mutex_lock(&outMtx);
        while(!acessVarMtx32(&varMtx,&out_alive,0,-1)&&acessVarMtx32(&varMtx,&all_alive,0,-1)){

                pthread_cond_wait(&outCond,&outMtx);

        }
        pthread_mutex_unlock(&outMtx);
	if(enable_ncurses){
		initscr();
		keypad(stdscr, TRUE);
		echo();
		typeahead(0);
	}
	mtx_protected_print("Client's output printing message channel thread alive!\n");
	memset(outbuff,0,DEF_DATASIZE);
	acessVarMtx32(&varMtx,&cmd_alive,1,0);
	int numread=-1;
	pthread_cond_signal(&cmdCond);
	while(acessVarMtx32(&varMtx,&out_alive,0,-1)&&acessVarMtx32(&varMtx,&all_alive,0,-1)){
		numread=recvsome(client_socket,outbuff,DEF_DATASIZE,clnt_data_pair);
		if(numread<=0){
			if((numread==-2)||!numread){
				continue;
			}
			else if(numread){
				raise(SIGINT);
			}
		}
		mtx_protected_print("%s",outbuff);
		memset(outbuff,0,DEF_DATASIZE);
	}
	acessVarMtx32(&varMtx,&out_alive,0,0);
	if(enable_ncurses){
		endwin_wrapper();
	}
	mtx_protected_print("Client's output printing message channel thread exiting!\n");
	return args;

}

static void* command_line_thread(void* args){
	pthread_mutex_lock(&cmdMtx);
	while(!acessVarMtx32(&varMtx,&cmd_alive,0,-1)&&acessVarMtx32(&varMtx,&all_alive,0,-1)){
		pthread_cond_wait(&cmdCond,&cmdMtx);

	}
	pthread_mutex_unlock(&cmdMtx);;
	printf("Client's command sending channel thread alive!\n");
	int numread=0;
	int numsent=0;
	memset(raw_line,0,DEF_DATASIZE);
	while(acessVarMtx32(&varMtx,&cmd_alive,0,-1)&&acessVarMtx32(&varMtx,&all_alive,0,-1)){
		memset(raw_line,0,DEF_DATASIZE);
		numread=readsome(0,raw_line,DEF_DATASIZE,clnt_data_pair);
		if(numread>0){
			numsent=sendsome(client_socket,raw_line,DEF_DATASIZE,clnt_data_pair);
			if(numsent<0){
				break;
			}
		}
	}
	printf("Client's command sending channel thread exiting!\n");
	raise(SIGINT);
	return args;
}

void cleanup_crew(void){

        pthread_mutex_lock(&exitMtx);
        while(acessVarMtx32(&varMtx,&all_alive,0,-1)){

                pthread_cond_wait(&exitCond,&exitMtx);

        }
        pthread_mutex_unlock(&exitMtx);
        send_exit_cmd_to_server();
	mtx_protected_print("Cleanup crew called in client\n");
	mtx_protected_print("Cleanup crew called in client. About to join threads which are online\n");
        mtx_protected_print("Reaping client cmdline thread!!\n");
	pthread_join(commandPrompt,NULL);

        mtx_protected_print("Reaping client output writter thread!!\n");
	pthread_join(outputPrinter,NULL);

        mtx_protected_print("Cleanup crew called in client. Closing file descriptors and sockets\n");
        fflush(stdout);
	close(client_socket);

	mtx_protected_print("Finished cleanup in client.\n");
}

int main(int argc, char ** argv){

	if(argc!=4){

		mtx_protected_print("Utilizacao correta: arg1: ip de server a connectar.\narg2: porta de server\narg3: enable_ncurses or not (0=off, 1=on)\n");
		exit(-1);
	}
	enable_ncurses=clamp(atoi(argv[3]),0,1);

	initClient(argv[1],atoi(argv[2]));
	tryConnect(&client_socket);

	pthread_create(&outputPrinter,NULL,getOutput,NULL);
	pthread_setname_np(outputPrinter,"outputPrinter_remote_shell_client");

	pthread_create(&commandPrompt,NULL,command_line_thread,NULL);
        pthread_setname_np(commandPrompt,"commandPrompt_remote_shell_client");

	clear_screen_with_printf();
	if(enable_ncurses){
		clear();
	}
	acessVarMtx32(&varMtx,&out_alive,1,0);
	pthread_cond_signal(&outCond);

	cleanup_crew();
	printf("ending client.\n");
	return 0;
}

