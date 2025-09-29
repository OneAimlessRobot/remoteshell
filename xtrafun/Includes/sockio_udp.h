#ifndef SOCKIO_UDP_H_H
#define SOCKIO_UDP_H_H

#include "sockio.h"

int sendsome_udp(int sd,char buff[],u_int64_t size,int_pair times, struct sockaddr_in *udp_addr_dst);

int readall_udp(int sock,char* buff,int_pair times,struct sockaddr_in *udp_addr_src);

int sendallfd_udp(int sock,int fd,int_pair times,struct sockaddr_in *udp_addr_dst);

int readsome_udp(int sd,char buff[],u_int64_t size,int_pair times,struct sockaddr_in *udp_addr_src);

int readalltofd_udp(int sock,int fd,int_pair times,struct sockaddr_in *udp_addr_src);

#endif
