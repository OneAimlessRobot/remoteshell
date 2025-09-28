#include "../Includes/preprocessor.h"
#include "../Includes/socketops.h"


void safety_close(/*int fd_to_close,*/char* prompt_to_show,int safety_fd_write){

        printf("We are trying to close the fd of the following description safely:\nWe are in the server\n%s\n",prompt_to_show);
        write(safety_fd_write,"x",1);
        printf("Okay! We wrote the closing byte into the safety socket of the following description:\nWe are in the server\n%s\n",prompt_to_show);
	//close(fd_to_close);

}


int64_t timedSend(int fd,int safety_fd,char buff[],u_int64_t size,int secwait,int usecwait){
                int iResult;
                struct timeval tv;
                fd_set wrfds;
                FD_ZERO(&wrfds);
                FD_SET(fd,&wrfds);
                fd_set rfds;
                char drain_buff[LINESIZE]={0};
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


int64_t timedRead(int fd,int safety_fd,char buff[],u_int64_t size,int secwait,int usecwait){
                int iResult;
                struct timeval tv;
                fd_set rfds;
                char drain_buff[LINESIZE]={0};
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
