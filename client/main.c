#include "../xtrafun/Includes/preprocessor.h"
#include "../xtrafun/Includes/fileshit.h"

#define TERMBUFFSIZE 1024

#define TERMIOS_BUFFER_THRESHOLD_BYTES 0
#define TERMIOS_INPUT_DELAY_TENTHS 0


static struct termios orig;
static struct sigaction sa_client_shell;

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
    raw.c_cc[VMIN]  = TERMIOS_BUFFER_THRESHOLD_BYTES;
    raw.c_cc[VTIME] = TERMIOS_INPUT_DELAY_TENTHS;

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

	all_alive=0*signal;
	out_alive=0*signal;
	cmd_alive=0*signal;
}
static void cleanup(void){

	all_alive=0;
	out_alive=0;
	cmd_alive=0;
	pthread_cond_signal(&exitCond);
}



static void initClient(char* addr,int port){

	client_socket= socket(AF_INET,SOCK_STREAM,0);
	if(client_socket==-1){
		exit(-1);
	}
	setNonBlocking(&client_socket);
	sa_client_shell.sa_handler = sigint_handler;
        sigemptyset(&sa_client_shell.sa_mask);
        sa_client_shell.sa_flags = SA_RESTART;
        sigaction(SIGINT, &sa_client_shell, NULL);
        sigaction(SIGPIPE, &sa_client_shell, NULL);
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
        	cleanup();
        }
	fprintf(stdout,"Sucessfully connected!!!\n");
}


void cleanup_crew(void){

        pthread_mutex_lock(&exitMtx);
        while(all_alive){

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
        while(!out_alive&&all_alive){

                pthread_cond_wait(&outCond,&outMtx);

        }
        pthread_mutex_unlock(&outMtx);
	mtx_protected_print("Client's output printing message channel thread alive!\n");
	memset(outbuff,0,DEF_DATASIZE);
	cmd_alive=1;
	pthread_cond_signal(&cmdCond);
	int numread=-1;
	while(out_alive&&all_alive){
		while ((numread=recvsome(client_socket, outbuff, min(TERMBUFFSIZE,sizeof(outbuff)),clnt_data_pair)) >0) {
		        writesome(STDOUT_FILENO, outbuff, numread,clnt_data_pair);
	 		memset(outbuff,0,min(TERMBUFFSIZE,sizeof(outbuff)));
		}

	}
	out_alive=0;
	cleanup();
	mtx_protected_print("Client's output printing message channel thread exiting!\n");
	return args;

}

static void* command_line_thread(void* args){
	pthread_mutex_lock(&cmdMtx);
	while(!cmd_alive&&all_alive){
		pthread_cond_wait(&cmdCond,&cmdMtx);

	}
	pthread_mutex_unlock(&cmdMtx);;
	printf("Client's command sending channel thread alive!\n");
	enable_raw();
	int numread=0;
	int numsent=0;
	memset(raw_line,0,DEF_DATASIZE);
	while(cmd_alive&&all_alive){
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
	out_alive=1;
	pthread_cond_signal(&outCond);

	cleanup_crew();
	printf("ending client.\n");
	return 0;
}

