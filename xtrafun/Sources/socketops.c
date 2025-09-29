#include "../Includes/preprocessor.h"
#include "../Includes/socketops.h"


void safety_close(/*int fd_to_close,*/char* prompt_to_show,int safety_fd_write){

        printf("We are trying to close the fd of the following description safely:\nWe are in the server\n%s\n",prompt_to_show);
        write(safety_fd_write,"x",1);
        printf("Okay! We wrote the closing byte into the safety socket of the following description:\nWe are in the server\n%s\n",prompt_to_show);
	//close(fd_to_close);

}


int64_t timedSendSafe(int fd,int safety_fd,char buff[],u_int64_t size,int secwait,int usecwait){
                int iResult;
                struct timeval tv;
                fd_set wrfds;
                FD_ZERO(&wrfds);
                FD_SET(fd,&wrfds);
                fd_set rfds;
                char drain_buff[DATASIZE]={0};
                FD_ZERO(&rfds);
                FD_SET(safety_fd,&rfds);
                tv.tv_sec=secwait;
                tv.tv_usec=usecwait;
                //printf("just read from pipe or not??\nSafety fd: %d\n\n",safety_fd);
                iResult=select(MAX(safety_fd,fd)+1,&rfds,&wrfds,(fd_set*)0,&tv);
                if(iResult>0){
                while(read(safety_fd,drain_buff,1)>0){

                        printf("reading from safety fd in the server's timedSend function!!! %s\n",drain_buff);
                        close(safety_fd);
                        return -1;
                }
                //printf("just read from pipe or not\n");
                return write(fd,buff,size);
                }
                else if(iResult){
                return -1;
                }
                else{
                return 0;
                }

}


int64_t timedReadSafe(int fd,int safety_fd,char buff[],u_int64_t size,int secwait,int usecwait){
                int iResult;
                struct timeval tv;
                fd_set rfds;
                char drain_buff[DATASIZE]={0};
                FD_ZERO(&rfds);
                FD_SET(fd,&rfds);
                FD_SET(safety_fd,&rfds);
                tv.tv_sec=secwait;
                tv.tv_usec=usecwait;
                //printf("just read from pipe or not??\nSafety fd: %d\n\n",safety_fd);
                iResult=select(MAX(safety_fd,fd)+1,&rfds,(fd_set*)0,(fd_set*)0,&tv);
                if(iResult>0){
                while(read(safety_fd,drain_buff,1)>0){

                        printf("reading from safety fd in the server's timedRead function!!! %s\n",drain_buff);
                        close(safety_fd);
                        return -1;
                }
                //printf("just read from pipe or not\n");
                return read(fd,buff,size);
                }
                else if(iResult){
                return -1;
                }
                else{
                return 0;
                }


}

int64_t timedSend(int fd,char buff[],u_int64_t size,int secwait,int usecwait){
                int iResult;
                struct timeval tv;
                fd_set wrfds;
                FD_ZERO(&wrfds);
                FD_SET(fd,&wrfds);
                tv.tv_sec=secwait;
                tv.tv_usec=usecwait;
                //printf("just read from pipe or not??\nSafety fd: %d\n\n",safety_fd);
                iResult=select(fd+1,(fd_set*)0,&wrfds,(fd_set*)0,&tv);
                if(iResult>0){
                //printf("just read from pipe or not\n");
                return write(fd,buff,size);
                }
                else if(iResult){
                return -1;
                }
                else{
                return 0;
                }

}


int64_t timedRead(int fd,char buff[],u_int64_t size,int secwait,int usecwait){
                int iResult;
                struct timeval tv;
                fd_set rfds;
                FD_ZERO(&rfds);
                FD_SET(fd,&rfds);
                tv.tv_sec=secwait;
                tv.tv_usec=usecwait;
                //printf("just read from pipe or not??\nSafety fd: %d\n\n",safety_fd);
                iResult=select(fd+1,&rfds,(fd_set*)0,(fd_set*)0,&tv);
                if(iResult>0){
        	//printf("just read from pipe or not\n");
        	return read(fd,buff,size);
                }
                else if(iResult){
                return -1;
                }
                else{
                return 0;
                }


}

void create_safety_pipe(int safety_pipe[2],char* pipe_desc,char* module_desc,int flags){
if(pipe2(safety_pipe,flags)){

        dprintf(2,"Safety pipe named: %s on module named: %s failed to open! Aborting\nError: %s\n",pipe_desc,module_desc,strerror(errno));
        exit(-1);
}

if((safety_pipe[0]=dup(safety_pipe[0]))<0){

        dprintf(2,"Safety pipe named: %s on module named: %s failed to be duped on read end!!\nAborting\nError: %s\n",pipe_desc,module_desc,strerror(errno));
        exit(-1);


}
if((safety_pipe[1]=dup(safety_pipe[1]))<0){

        dprintf(2,"Safety pipe named: %s on module named: %s failed to be duped on read end!!\nAborting\nError: %s\n",pipe_desc,module_desc,strerror(errno));
        exit(-1);


}
printf("just created Safety pipe named: %s on module named: %s\n",pipe_desc,module_desc);

}




void set_sock_reuseaddr(int *socket,int on_off){
	int ptr=on_off;
	socklen_t sizeofbuff=sizeof(ptr);
	if(setsockopt(*socket,SOL_SOCKET,SO_REUSEADDR,(char*)&ptr,sizeofbuff)){
                perror("Erro a meter SO_REUSEADDR  na socket (setsockopt)\n");
                close(*socket);
        exit(-1);
        }
}

void set_sock_sendtimeout(int*socket,int timeout_s,int timeout_us){
	struct timeval tv={0};
	tv.tv_sec=timeout_s;
	tv.tv_usec=timeout_us;
	socklen_t sizeofbuff=sizeof(struct timeval);
	if(setsockopt(*socket,SOL_SOCKET,SO_SNDTIMEO,(char*)&tv,sizeofbuff)){
                perror("Erro a meter SO_SNDTIMEO  na socket (setsockopt)\n");
                close(*socket);
        exit(-1);
        }
}

void set_sock_recvtimeout(int*socket,int timeout_s,int timeout_us){
	struct timeval tv={0};
	tv.tv_sec=timeout_s;
	tv.tv_usec=timeout_us;
	socklen_t sizeofbuff=sizeof(struct timeval);
	if(setsockopt(*socket,SOL_SOCKET,SO_RCVTIMEO,(char*)&tv,sizeofbuff)){
                perror("Erro a meter SO_RCVTIMEO  na socket (setsockopt)\n");
                close(*socket);
        exit(-1);
        }
}

void setSocketRecvBuffSize(int*sd,int size){

setsockopt(*sd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));

}

void setSocketSendBuffSize(int*sd,int size){

setsockopt(*sd, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size)); // Send buffer 1K

}
int getSocketRecvBuffSize(int*sd){

int sizevaluecontainer=0;
socklen_t sizeofbuff=sizeof(sizevaluecontainer);
getsockopt(*sd, SOL_SOCKET, SO_RCVBUF, &sizevaluecontainer, &sizeofbuff);
return sizevaluecontainer;
}

int getSocketSendBuffSize(int*sd){

int sizevaluecontainer=0;
socklen_t sizeofbuff=sizeof(sizevaluecontainer);
getsockopt(*sd, SOL_SOCKET, SO_SNDBUF, &sizevaluecontainer, &sizeofbuff);
return sizevaluecontainer;
}

void setLinger(int*socket,int onoff,int time){

struct linger so_linger;
so_linger.l_onoff = onoff; // Enable linger option
so_linger.l_linger = time; // Linger time, set to 0

if (setsockopt(*socket, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger)) < 0) {
    perror("setsockopt");
    // Handle error
}

}
void setNonBlocking(int*socket) {
    int flags = fcntl(*socket, F_GETFL, 0);
    if (flags == -1) {
        fprintf(stderr,"erro a atribuir flags a uma socket de cliente: %s\n",strerror(errno));
        return;
    }

    if (fcntl(*socket, F_SETFL, flags | O_NONBLOCK) == -1) {
        fprintf(stderr,"erro a atribuir flags a uma socket de cliente: %s\n",strerror(errno));
    }
}

