#ifndef SOCKETOPS_H
#define SOCKETOPS_H


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

