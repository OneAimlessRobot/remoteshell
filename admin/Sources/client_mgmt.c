#include "../../xtrafun/Includes/preprocessor.h"
#include "../../xtrafun/Includes/openssl_stuff.h"
#include "../../xtrafun/Includes/fileshit.h"
#include "../Includes/admin_pty_setting.h"
#include "../Includes/client_mgmt.h"

int master_fd=-1;
int slave_fd=-1;
int pid_shell=-1;

struct sigaction sa_client;

static void switch_all_off(void){

	all_alive=0;
	out_alive=0;
	cmd_alive=0;
	printf("sigint was called in server subprocess!!\n");
	pthread_cond_signal(&exitCond);

}


static void sigint_handler_client(int signal){
	all_alive=0*signal;
	out_alive=0*signal;
	cmd_alive=0*signal;
}



static void* writeOutput(void* args){

	pthread_mutex_lock(&outMtx);
	while(!out_alive&&all_alive){

		pthread_cond_wait(&outCond,&outMtx);

	}
	pthread_mutex_unlock(&outMtx);
	printf("Server's output message channel thread online!!!\n");
	int numread=-1;
	int numsent=-1;
	cmd_alive=1;
	pthread_cond_signal(&cmdCond);
	memset(outbuff,0,DEF_DATASIZE);
	while(out_alive&&all_alive){
		memset(outbuff,0,DEF_DATASIZE);
		numread=readsome(master_fd,outbuff,DEF_DATASIZE,srv_data_pair);
		if(numread<=0){
			break;
		}
		numsent=will_use_tls?sendsome_ssl(server_ssl,outbuff,DEF_DATASIZE,srv_data_pair):sendsome(client_socket,outbuff,DEF_DATASIZE,srv_data_pair);
		if(numsent<0){
	               if((numsent==-2)||!numsent){
                                continue;
                        }
                        else if(numsent){
                        	perror("Error or interruption in output sending! Exiting...\n");
			        break;
			}
		}
	}
	printf("Server's output message channel thread out!!!\n");
        switch_all_off();
	return args;



}
static void* command_prompt_thread(void* args){
	
	pthread_mutex_lock(&cmdMtx);
	while(!cmd_alive&&all_alive){

		pthread_cond_wait(&cmdCond,&cmdMtx);

	}
	pthread_mutex_unlock(&cmdMtx);

	printf("Server's command receiving channel thread about to start!\n");
	memset(raw_line,0,sizeof(raw_line));
	int numread=0;
	int numwritten=0;
	while(cmd_alive&&all_alive){
	numread=will_use_tls?readsome_ssl(server_ssl,raw_line,DEF_DATASIZE,srv_data_pair):recvsome(client_socket,raw_line,DEF_DATASIZE,srv_data_pair);
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
	switch_all_off();
	return args;

}

static void cleanup_crew_client(void){

	pthread_mutex_lock(&exitMtx);
	while(all_alive){

		pthread_cond_wait(&exitCond,&exitMtx);

	}
	pthread_mutex_unlock(&exitMtx);
	printf("Cleanup crew called in server subprocees\n");
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
	if(!cmd_alive){
		printf("reaping server comamnd reader thread...\n");
		pthread_join(commandPrompt,NULL);
		printf("reaped server comamnd reader thread!!\n");
	}
	if(!out_alive){
		printf("reaping server output writter thread...\n");
		pthread_join(outputWritter,NULL);
		printf("reaped server output writter thread!!\n");
	}
	if(will_use_tls){
		ShutdownSSL(&server_ssl);
		end_openssl_libs_server_side();
	}
	close(master_fd);
	close(client_socket);
	printf("Finished cleanup in server.\n");

}

void do_connection(char* shell_name){
	setNonBlocking(&client_socket);
	if(will_use_tls){
		init_openssl_libs_server_side();
		convert_server_con_to_ssl(&server_ssl,client_socket,srv_con_pair);
	}
	printf("Shell name: %s\n",shell_name);

	char* ptrs[3];
	char pty_name[128]={0};
	openpty(&master_fd,&slave_fd,pty_name,NULL,NULL);
	set_pty_size(master_fd);
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
			perror("Could not start shell in admin sub process!\n");
			exit(-1);
		default:
			printf("We just spawned shell process of pid number %d!!!!!\n",pid_shell);
			close(slave_fd);
			break;
	}
	
	pthread_create(&commandPrompt,NULL,command_prompt_thread,NULL);
	pthread_setname_np(commandPrompt,"commandPrompt_remote_shell_server");
	
	sa_client.sa_handler = sigint_handler_client;
        sigemptyset(&sa_client.sa_mask);
        sa_client.sa_flags = SA_RESTART;
        sigaction(SIGINT, &sa_client, NULL);
        sigaction(SIGPIPE, &sa_client, NULL);

	out_alive=1;

        pthread_cond_signal(&outCond);

	pthread_create(&outputWritter,NULL,writeOutput,NULL);
	pthread_setname_np(outputWritter,"outputWritter_remote_shell_server");


	cleanup_crew_client();

	exit(-1);
}

