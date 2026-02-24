#include "../Includes/preprocessor.h"
#include "../Includes/auxFuncs.h"
#include "../Includes/sockio.h"
#include "../Includes/sockio_tcp.h"
#include "../Includes/fileshit.h"


int sendsome_ssl(SSL* ssl, const char* buf, size_t len, int_pair times) {

	int sd= SSL_get_fd(ssl);
	size_t send_total = 0;
	while (send_total < len) {
	struct timeval tv;
	tv.tv_sec=times[0];
	tv.tv_usec=times[1];
	fd_set wrfds;
	FD_ZERO(&wrfds);
	FD_SET(sd, &wrfds);
	int iResult=select(sd + 1, (fd_set*)0, &wrfds, (fd_set*)0, &tv);
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
		if (ssl_err == SSL_ERROR_WANT_WRITE) {
			struct timeval mtv;
			mtv.tv_sec=times[0];
			mtv.tv_usec=times[1];
			fd_set mwfds;
			FD_ZERO(&mwfds);
			FD_SET(sd, &mwfds);
			select(sd + 1, (fd_set*)0, &mwfds, (fd_set*)0, &mtv);
			continue;
		}
		else if (ssl_err == SSL_ERROR_ZERO_RETURN) {
			return send_total;
		}
		else{
		    ERR_print_errors_fp(stderr);
		    exit_emergency_func();
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
	while (read_total < len) {
	struct timeval tv;
	tv.tv_sec=times[0];
	tv.tv_usec=times[1];
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(sd, &rfds);
	int iResult=select(sd + 1, &rfds, (fd_set*)0, (fd_set*)0, &tv);
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
			struct timeval mtv;
			mtv.tv_sec=times[0];
			mtv.tv_usec=times[1];
			fd_set mrfds;
			FD_ZERO(&mrfds);
			FD_SET(sd, &mrfds);
			select(sd + 1, &mrfds, (fd_set*)0, (fd_set*)0, &mtv);
			continue;
		}
		else if (ssl_err == SSL_ERROR_WANT_WRITE) {
			struct timeval mtv;
			mtv.tv_sec=times[0];
			mtv.tv_usec=times[1];
			fd_set wfds;
			FD_ZERO(&wfds);
			FD_SET(sd, &wfds);
			select(sd + 1, (fd_set*)0, &wfds, (fd_set*)0, &mtv);
			continue;
		}
		else if (ssl_err == SSL_ERROR_ZERO_RETURN) {
			return read_total;
		}
		else{
		    ERR_print_errors_fp(stderr);
		    exit_emergency_func();
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
		exit_emergency_func();
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
		exit_emergency_func();
		return -1;
		}
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
		exit_emergency_func();
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
		exit_emergency_func();
		return -1;
		}
}

