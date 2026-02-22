#include "../Includes/preprocessor.h"
#include "../Includes/auxFuncs.h"
#include "../Includes/sockio.h"
#include "../Includes/sockio_tcp.h"
#include "../Includes/fileshit.h"



int sendsome_ssl(SSL* ssl, const char* buf, size_t len, int_pair times) {

	int sd= SSL_get_fd(ssl);
	size_t send_total = 0;
	int iResult=0;
	while (send_total < len) {
	struct timeval tv;
	tv.tv_sec=times[0];
	tv.tv_usec=times[1];
	fd_set wrfds;
	FD_ZERO(&wrfds);
	FD_SET(sd, &wrfds);
	iResult=select(sd + 1, (fd_set*)0, &wrfds, (fd_set*)0, &tv);
	if(iResult>0){
		int ret = SSL_write(ssl, buf + send_total, len - send_total);
	        if (ret > 0) {
	            send_total += ret;
	            continue;
	        }
			else if (ret == 0) {
				return send_total;
			}
	        int ssl_err = SSL_get_error(ssl, ret);
		if (ssl_err == SSL_ERROR_WANT_READ) {
			fd_set rfds;
			FD_ZERO(&rfds);
			FD_SET(sd, &rfds);
			select(sd + 1, &rfds, (fd_set*)0, (fd_set*)0, &tv);
			if(logging){

				fprintf(logstream, "Waiting\n%s\n",strerror(errno));
			}
			continue;
		}
		else if (ssl_err == SSL_ERROR_WANT_WRITE) {
			fd_set wfds;
			FD_ZERO(&wfds);
			FD_SET(sd, &wfds);
			select(sd + 1, (fd_set*)0, &wfds, (fd_set*)0, &tv);
			if(logging){

				fprintf(logstream, "Waiting\n%s\n",strerror(errno));
			}
			continue;
		}
		else{
		    ERR_print_errors_fp(stderr);
		    return -1;
		}
	}
	else if(!iResult){
		return -2;
	}
	else{
		if(logging){

	        	fprintf(logstream, "SELECT ERROR!!!!! SSL SEND\n%s\n",strerror(errno));
		}
		return -1;
	}

}
return send_total;


}

int readsome_ssl(SSL* ssl, char* buf, size_t len, int_pair times) {
	int sd= SSL_get_fd(ssl);
	size_t read_total = 0;
	int iResult=0;
	while (read_total < len) {
	struct timeval tv;
	tv.tv_sec=times[0];
	tv.tv_usec=times[1];
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(sd, &rfds);
	iResult=select(sd + 1, &rfds, (fd_set*)0, (fd_set*)0, &tv);
	if(iResult>0){
		int ret = SSL_read(ssl, buf + read_total, len - read_total);
		if (ret > 0) {
			read_total += ret;
			continue;
		}
		else if (ret == 0) {
			return read_total;
		}
		int ssl_err = SSL_get_error(ssl, ret);
		if (ssl_err == SSL_ERROR_WANT_READ) {
			fd_set rfds;
			FD_ZERO(&rfds);
			FD_SET(sd, &rfds);
			select(sd + 1, &rfds, (fd_set*)0, (fd_set*)0, &tv);
			if(logging){

				fprintf(logstream, "Waiting\n%s\n",strerror(errno));
			}
			continue;
		}
		else if (ssl_err == SSL_ERROR_WANT_WRITE) {
			fd_set wfds;
			FD_ZERO(&wfds);
			FD_SET(sd, &wfds);
			select(sd + 1, (fd_set*)0, &wfds, (fd_set*)0, &tv);
			if(logging){

				fprintf(logstream, "Waiting\n%s\n",strerror(errno));
			}
			continue;
		}
		else{
		    ERR_print_errors_fp(stderr);
		    return -1;
		}
	}
	else if(!iResult){
		return -2;
	}
	else{
		if(logging){

	        	fprintf(logstream, "SELECT ERROR!!!!! SSL READ\n%s\n",strerror(errno));
		}
		return -1;
	}

}
return read_total;

}


int sendsome(int sd,char buff[],u_int64_t size,int_pair times){
                int iResult;
                struct timeval tv;
                fd_set wfds;
                FD_ZERO(&wfds);
                FD_SET(sd,&wfds);
                tv.tv_sec=times[0];
                tv.tv_usec=times[1];
                iResult=select(sd+1,(fd_set*)0,&wfds,(fd_set*)0,&tv);
                if(iResult>0){

                return send(sd,buff,size,0);
                }
		else if(!iResult){
               	return -2;
		}
		else{
		if(logging){

		fprintf(logstream, "SELECT ERROR!!!!! SEND\n%s\n",strerror(errno));
		}
		return -1;
		}
}
int writesome(int fd,char buff[],u_int64_t size,int_pair times){
                int iResult;
                struct timeval tv;
                fd_set wfds;
                FD_ZERO(&wfds);
                FD_SET(fd,&wfds);
                tv.tv_sec=times[0];
                tv.tv_usec=times[1];
                iResult=select(fd+1,(fd_set*)0,&wfds,(fd_set*)0,&tv);
                if(iResult>0){

                return write(fd,buff,size);
                }
		else if(!iResult){
               	return -2;
		}
		else{
		if(logging){

		fprintf(logstream, "SELECT ERROR!!!!! SEND\n%s\n",strerror(errno));
		}
		return -1;
		}
}

int sendallfd(int sock,int fd,int_pair times){
logstream= stderr;

char buff[DEF_DATASIZE];
memset(buff,0,DEF_DATASIZE);
int numread;
int all_that_was_sent=0;
int sent=0;
while ((numread = read(fd,buff,DEF_DATASIZE)) > 0) {
    
        errno=0;
	sent = sendsome(sock, buff,  numread,times);
	memset(buff,0,DEF_DATASIZE);
	if(sent==-2){
		
		if(logging){
		fprintf(logstream,"Timeout no sending!!!!: %s\nsocket %d\n",strerror(errno),sock);
                }
		lseek(fd,-(numread-sent),SEEK_CUR);
		continue;
	}
	if(sent<0){
	
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                if(logging){
		fprintf(logstream,"Block no sending!!!!: %s\nsocket %d\n",strerror(errno),sock);
                }
		break;

        }
	else if(errno==EPIPE){

		if(logging){
		fprintf(logstream,"Pipe partido!!! A socket e %d\n",sock);
		}
		raise(SIGINT);
		return -1;
	}
        else if(errno == ECONNRESET){
		if(logging){
                fprintf(logstream,"Conexão largada!!\nSIGPIPE!!!!!: %s\n",strerror(errno));
                }
		raise(SIGINT);
		return -1;
	}
	else {
		if(logging){
                fprintf(logstream,"Outro erro qualquer!!!!!: %d %s\n",errno,strerror(errno));
                }
		break;
	}
        }
		all_that_was_sent+=sent;
	}
	if(logging){
	fprintf(logstream,"send de %d bytes feito!!!!!\n",all_that_was_sent);
	}
	return 0;
}


int recvsome(int sd,char buff[],u_int64_t size,int_pair times){
		int iResult;
                struct timeval tv;
                fd_set rfds;
                FD_ZERO(&rfds);
                FD_SET(sd,&rfds);
                tv.tv_sec=times[0];
                tv.tv_usec=times[1];
                iResult=select(sd+1,&rfds,(fd_set*)0,(fd_set*)0,&tv);
                if(iResult>0){

                return recv(sd,buff,size,0);

                }
		else if(!iResult){
               	return -2;
		}
		else{
		if(logging){

		fprintf(logstream, "SELECT ERROR!!!!! READ\n%s\n",strerror(errno));
		}
		return -1;
		}
}

int readsome(int fd,char buff[],u_int64_t size,int_pair times){
		int iResult;
                struct timeval tv;
                fd_set rfds;
                FD_ZERO(&rfds);
                FD_SET(fd,&rfds);
                tv.tv_sec=times[0];
                tv.tv_usec=times[1];
                iResult=select(fd+1,&rfds,(fd_set*)0,(fd_set*)0,&tv);
                if(iResult>0){

                return read(fd,buff,size);

                }
		else if(!iResult){
               	return -2;
		}
		else{
		if(logging){

		fprintf(logstream, "SELECT ERROR!!!!! READ\n%s\n",strerror(errno));
		}
		return -1;
		}
}


int sendall(int sock,char buff[],int size,int_pair times){
        int64_t len=0;
	int64_t total=0;
	for(len=sendsome(sock,buff+total,size-total,times);(len>0)&&(total!=size);total+=len){
	
			len=sendsome(sock,buff+total,size-total,times);
	}
	
	if(!(total-size)){
		if(logging){
		fprintf(logstream,"sendall bem sucedido!! A socket e %d\n",sock);
		
		}
	}
	else if(errno==EPIPE){

		if(logging){
		fprintf(logstream,"Pipe partido!!! A socket e %d\n",sock);
		}
		return -2;
	}
	else if(errno==ENOTCONN){
		if(logging){
		fprintf(logstream,"sendall saiu com erro!!!!!:\nAvisando server para desconectar!\n%s\n",strerror(errno));
		}
		
		return -2;
	}
	else if(len!=-2){
		if(logging){
		fprintf(logstream,"sendall saiu com erro!!!!!:\n%s\n",strerror(errno));
		}
	}
	
        return total;

}

int readall(int sock,char buff[],int size,int_pair times){
        int64_t len=0;
	int64_t total=0;
	for(len=readsome(sock,buff+total,size-total,times);(len>0)&&(total!=size);total+=len){
	
			len=readsome(sock,buff+total,size-total,times);
	}
	
	if(!(total-size)){
		if(logging){
		fprintf(logstream,"readall bem sucedido!! A socket e %d\n",sock);
		
		}
	}
	else if(errno==EPIPE){

		if(logging){
		fprintf(logstream,"Pipe partido!!! A socket e %d\n",sock);
		}
		return -2;
	}
	else if(errno==ENOTCONN){
		if(logging){
		fprintf(logstream,"readall saiu com erro!!!!!:\nAvisando server para desconectar!\n%s\n",strerror(errno));
		}
		
		return -2;
	}
	else if(len!=-2){
		if(logging){
		fprintf(logstream,"readall saiu com erro!!!!!:\n%s\n",strerror(errno));
		}
	}
	
        return total;

}

int readalltofd(int sock,int fd,int size,int_pair times){
        int32_t len=1;
	int32_t written=1;
	int32_t total=0;
	char buff[DEF_DATASIZE];
	memset(buff,0,DEF_DATASIZE);
	for(;(len==-2||len>0)&&(total!=size);){
                len=readsome(sock,buff,DEF_DATASIZE,times);
                written=write(fd,buff,len);
                total+= (written<0)? 0:written;
                memset(buff,0,DEF_DATASIZE);
        }

	if(!(total-size)){
		if(logging){
		fprintf(logstream,"readall bem sucedido!! A socket e %d\n",sock);

		}
	}
	if(len<0){
	if (errno == EAGAIN || errno == EWOULDBLOCK) {
        	if(logging){
		fprintf(logstream,"readall bem sucedido!! A socket e %d\n",sock);
		}
	}
	else if(errno==EPIPE){

		if(logging){
		fprintf(logstream,"Pipe partido!!! A socket e %d\n",sock);
		}
		return -2;
	}
	else if(errno==ENOTCONN){
		if(logging){
		fprintf(logstream,"readall saiu com erro!!!!!:\nAvisando server para desconectar!\n%s\n",strerror(errno));
		}
		
		return -2;
	}
	else if(len!=-2){
		if(logging){
		fprintf(logstream,"readall saiu com erro!!!!!:\n%s\n",strerror(errno));
		}
	}
	
	}
	if(logging){
		fprintf(logstream,"readalltofd bem sucedido. A socket e %d\nLemos %d de %d bytes\n",sock,total,size);

	}
	memset(buff,0,DEF_DATASIZE);
	
        return 0;

}

