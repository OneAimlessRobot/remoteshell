#ifndef SOCKIO_TCP_H_H
#define SOCKIO_TCP_H_H
#include "sockio.h"
int sendsome(int sd,char buff[],u_int64_t size,int_pair times);

int writesome(int fd,char buff[],u_int64_t size,int_pair times);

int sendsome_ssl(SSL* ssl, const char* buf, size_t len, int_pair times);

int readsome_ssl(SSL* ssl, char* buf, size_t len, int_pair times);

int recvsome(int sd,char buff[],u_int64_t size,int_pair times);

int readsome(int fd,char buff[],u_int64_t size,int_pair times);


#endif
