#include "../xtrafun/Includes/preprocessor.h"
#include "./Includes/admin_cert_file_paths.h"
#include "../../xtrafun/Includes/openssl_stuff.h"
#include "../xtrafun/Includes/fileshit.h"
#include "./Includes/admin_pty_setting.h"
#include "./Includes/client_mgmt.h"



static int pid_client=-1;

struct sigaction sa_chld, sa;

static atomic_int server_alive=1;


static void sigint_handler_server(int signal){

	server_alive=0*signal;
}
static void sigact_sigint_handler_server(int signal){

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
	initalize_admin_cert_file_paths();
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
	
	if(argc!=7){

		printf("arg1: address\narg2: porta do server\narg3: shell to use\narg4: will use tls? (1=\"yes\", 0=\"no\")\narg5 and 6: width and height that all users will have upon logging in\n - (defaults are %d and %d respectively according to defines in this build) - \n",DEFAULT_TERM_HEIGHT,DEFAULT_TERM_HEIGHT);
		exit(-1);
	}

	height_for_pty=atoi(argv[5])>0?atoi(argv[5]):DEFAULT_TERM_HEIGHT;
	printf("setting the height of every spawned pty to: %u\n",height_for_pty);
	width_for_pty=atoi(argv[5])>0?atoi(argv[5]):DEFAULT_TERM_WIDTH;
	printf("setting the width of every spawned pty to: %u\n",width_for_pty);
	logstream=stderr;
	logging=1;
	will_use_tls=atoi(argv[4]);
	initServer(argv[1],atoi(argv[2]));
	sa_chld.sa_handler=sigact_sigint_handler_server;
        sigemptyset(&sa_chld.sa_mask);
        sa_chld.sa_flags=SA_RESTART|SA_NOCLDWAIT;

        sigaction(SIGCHLD, &sa_chld, NULL);

	sa.sa_handler = sigint_handler_server;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;
        sigaction(SIGINT, &sa, NULL);
        sigaction(SIGPIPE, &sa, NULL);

	accept_connections(argv[3]);
        printf("ending server.\n");
	return 0;
}
