#include "../xtrafun/Includes/preprocessor.h"
#include "../xtrafun/Includes/fileshit.h"


static void sigint_handler(int signal){

        printf("sigint was called in client!!\n");
        acessVarMtx32(&varMtx,&all_alive,0,0*signal);
        pthread_cond_signal(&exitCond);
}

static void sigpipe_handler(int signal){

        printf("sigpipe was called in client!!\n");
        raise(SIGINT+signal*0);
}


static void initClient(int port, char* addr){

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
        printf("Não foi possivel conectar. Numero limite de tentativas (%d) atingido!!!\n",MAXNUMBEROFTRIES);
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
	printf("Client's output printing message channel thread alive!\n");
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
		dprintf(1,"%s",outbuff);
		memset(outbuff,0,DEF_DATASIZE);
	}
	acessVarMtx32(&varMtx,&out_alive,0,0);
	printf("Client's output printing message channel thread exiting!\n");
	return args;

}

static void* command_line_thread(void* args){
	pthread_mutex_lock(&cmdMtx);
	while(!acessVarMtx32(&varMtx,&cmd_alive,0,-1)&&acessVarMtx32(&varMtx,&all_alive,0,-1)){
		pthread_cond_wait(&cmdCond,&cmdMtx);

	}
	pthread_mutex_unlock(&cmdMtx);
	printf("Client's command sending channel thread alive!\n");
	int numread=0;
	int numsent=0;
	memset(raw_line,0,DEF_DATASIZE);
	while(acessVarMtx32(&varMtx,&cmd_alive,0,-1)&&acessVarMtx32(&varMtx,&all_alive,0,-1)){
		memset(raw_line,0,DEF_DATASIZE);
		numread=readsome(0,raw_line,DEF_DATASIZE,clnt_data_pair);
		if(numread<=0){
			printf("Client launching sigint!!!\nLast numread: %d\n",numread);
			break;
		}
		numsent=sendsome(client_socket,raw_line,DEF_DATASIZE,clnt_data_pair);
		if(numsent<0){
			printf("Client launching sigint!!!\nLast numsent: %d\n",numsent);
			break;
		}
		if(!strncmp(raw_line, "exit",strlen("exit"))&&((strlen(raw_line)-1)==strlen("exit"))){
			break;
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
        printf("Cleanup crew called in client\n");
	printf("Cleanup crew called in client. About to join threads which are online\n");
        printf("Reaping client cmdline thread!!\n");
	pthread_join(commandPrompt,NULL);

        printf("Reaping client output writter thread!!\n");
	pthread_join(outputPrinter,NULL);

        printf("Cleanup crew called in client. Closing file descriptors and sockets\n");
        fflush(stdout);
	close(client_socket);

	printf("Finished cleanup in client.\n");
}

int main(int argc, char ** argv){

	if(argc!=3){

		printf("Utilizacao correta: arg1: ip de server a connectar.\narg2: porta de server\n");
		exit(-1);
	}

	initClient(atoi(argv[1]),argv[2]);
	tryConnect(&client_socket);

	pthread_create(&outputPrinter,NULL,getOutput,NULL);
	pthread_setname_np(outputPrinter,"outputPrinter_remote_shell_client");

	pthread_create(&commandPrompt,NULL,command_line_thread,NULL);
        pthread_setname_np(commandPrompt,"commandPrompt_remote_shell_client");

        acessVarMtx32(&varMtx,&out_alive,1,0);
	pthread_cond_signal(&outCond);

	cleanup_crew();
	printf("ending client.\n");
	return 0;
}

