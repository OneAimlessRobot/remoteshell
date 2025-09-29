#ifndef SOCKETOPS_H
#define SOCKETOPS_H
#define MAXNUMBEROFTRIES 10


#define CLIENT_MAXTIMEOUTCONS 2
#define CLIENT_MAXTIMEOUTUCONS 0


#define CLIENT_MAXTIMEOUTSECS (60*5)
#define CLIENT_MAXTIMEOUTUSECS 0

#define CLIENT_MAXTIMEOUTCMD 1
#define CLIENT_MAXTIMEOUTUCMD 0

#define SERVER_MAXTIMEOUTCONS 10
#define SERVER_MAXTIMEOUTUCONS 0


#define SERVER_MAXTIMEOUTSECS (60*5)
#define SERVER_MAXTIMEOUTUSECS 0

#define SERVER_MAXTIMEOUTCMD 1
#define SERVER_MAXTIMEOUTUCMD 0

#define DATASIZE 1024


void safety_close(/*int fd_to_close,*/char* prompt_to_show,int safety_fd_write);

int64_t timedSendSafe(int fd,int safety_fd,char buff[],u_int64_t size,int secwait,int usecwait);

int64_t timedReadSafe(int fd,int safety_fd,char buff[],u_int64_t size,int secwait,int usecwait);

int64_t timedSend(int fd,char buff[],u_int64_t size,int secwait,int usecwait);

int64_t timedRead(int fd,char buff[],u_int64_t size,int secwait,int usecwait);

void create_safety_pipe(int safety_pipe[2],char* pipe_desc,char* module_desc,int flags);

void set_sock_reuseaddr(int*socket,int on_off);

void set_sock_sendtimeout(int*socket,int timeout_s,int timeout_us);

void set_sock_recvtimeout(int*socket,int timeout_s,int timeout_us);

void setSocketRecvBuffSize(int*sd,int size);

void setSocketSendBuffSize(int* sd,int size);

int getSocketRecvBuffSize(int*sd);

int getSocketSendBuffSize(int*sd);

void setLinger(int*socket,int onoff,int time);

void setNonBlocking(int*socket);

#endif

