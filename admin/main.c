#include "../xtrafun/Includes/preprocessor.h"
#include "../xtrafun/Includes/fileshit.h"
#include "./Includes/client_mgmt.h"

static int pid_client=-1;

struct sigaction sa;

static int32_t server_alive=1;


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
	server_alive+=0*signal;
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
