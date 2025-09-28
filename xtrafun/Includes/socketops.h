#ifndef SOCKETOPS_H
#define SOCKETOPS_H
#define MAXNUMBEROFTRIES 10


#define MAXTIMEOUTCONS (60*5)
#define MAXTIMEOUTUCONS 0


#define MAXTIMEOUTSECS (60*5)
#define MAXTIMEOUTUSECS 0

#define MAXTIMEOUTCMD 1
#define MAXTIMEOUTUCMD 0

#define LINESIZE 1024

#define DATASIZE 1024


void safety_close(/*int fd_to_close,*/char* prompt_to_show,int safety_fd_write);


int64_t timedSend(int fd,int safety_fd,char buff[],u_int64_t size,int secwait,int usecwait);

int64_t timedRead(int fd,int safety_fd,char buff[],u_int64_t size,int secwait,int usecwait);

void create_safety_pipe(int safety_pipe[2],char* pipe_desc,char* module_desc,int flags);
#endif
