#include "../xtrafun/Includes/preprocessor.h"
#include "../xtrafun/Includes/fileshit.h"


static struct termios orig;

static void enable_raw() {
   
    struct termios raw;

    // get current terminal settings
    if (tcgetattr(STDIN_FILENO, &orig) == -1) {
        perror("tcgetattr");
        exit(1);
    }

    raw = orig;

    // Input modes: no break, CR to NL, no parity check, no strip char,
    // no start/stop output control.
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

    // Output modes: disable post processing
    raw.c_oflag &= ~(OPOST);

    // Control modes: set 8-bit chars
    raw.c_cflag |= (CS8);

    // Local modes: echoing off, canonical off, no extended functions,
    // no signal chars (Ctrl-C, Ctrl-Z, etc.)
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN|ISIG);

    // Control chars: return each byte, no timeout
    raw.c_cc[VMIN]  = 1;
    raw.c_cc[VTIME] = 3;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        perror("tcsetattr");
        exit(1);
    }
}

static void disable_raw() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig);
}

static char wbuf[DEF_DATASIZE*4*1000]={0};

static pthread_mutex_t ncurses_mtx= PTHREAD_MUTEX_INITIALIZER;

void mtx_protected_print(const char* str,...){
	pthread_mutex_lock(&ncurses_mtx);
	va_list args;
	va_start(args, str);
	vsnprintf(wbuf, sizeof(wbuf), str, args);
	va_end(args);
	printf("%s",wbuf);
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
	clear_screen_with_printf();
	disable_raw();
	mtx_protected_print("Finished cleanup in client.\n");
}

static void* getOutput(void* args){
	
	pthread_mutex_lock(&outMtx);
        while(!acessVarMtx32(&varMtx,&out_alive,0,-1)&&acessVarMtx32(&varMtx,&all_alive,0,-1)){

                pthread_cond_wait(&outCond,&outMtx);

        }
        pthread_mutex_unlock(&outMtx);
	mtx_protected_print("Client's output printing message channel thread alive!\n");
	memset(outbuff,0,DEF_DATASIZE);
	acessVarMtx32(&varMtx,&cmd_alive,1,0);
	pthread_cond_signal(&cmdCond);
	while(acessVarMtx32(&varMtx,&out_alive,0,-1)&&acessVarMtx32(&varMtx,&all_alive,0,-1)){
		char c;
		while (recvsome(client_socket, &c, 1,clnt_data_pair) == 1) {
		        writesome(STDOUT_FILENO, &c, 1,clnt_data_pair);
	 	}

	}
	acessVarMtx32(&varMtx,&out_alive,0,0);
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
	enable_raw();
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
			if(raw_line[0]==3){
				break;
			}
		}
	}
	printf("Client's command sending channel thread exiting!\n");
	raise(SIGINT);
	return args;
}
int main(int argc, char ** argv){

	if(argc!=3){

		mtx_protected_print("Utilizacao correta: arg1: ip de server a connectar.\narg2: porta de server\n");
		exit(-1);
	}

	initClient(argv[1],atoi(argv[2]));
	tryConnect(&client_socket);

	pthread_create(&outputPrinter,NULL,getOutput,NULL);
	pthread_setname_np(outputPrinter,"outputPrinter_remote_shell_client");

	pthread_create(&commandPrompt,NULL,command_line_thread,NULL);
        pthread_setname_np(commandPrompt,"commandPrompt_remote_shell_client");
	clear_screen_with_printf();
	acessVarMtx32(&varMtx,&out_alive,1,0);
	pthread_cond_signal(&outCond);

	cleanup_crew();
	printf("ending client.\n");
	return 0;
}

