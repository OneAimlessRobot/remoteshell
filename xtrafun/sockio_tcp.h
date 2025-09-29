#ifndef SOCKIO_TCP_H_H
#define SOCKIO_TCP_H_H
#include "sockio.h"
int readall(int sock,char* buff,int size,int_pair times);

int sendall(int sock,char* buff,int size,int_pair times);

int sendsome(int sd,char buff[],u_int64_t size,int_pair times);

int sendallfd(int sock,int fd,int_pair times);

int readsome(int sd,char buff[],u_int64_t size,int_pair times);

int readalltofd(int sock,int fd,int down_size,int_pair times);

#endif
