#include "../Includes/preprocessor.h"
#include "../Includes/auxFuncs.h"
#include "../Includes/sockio.h"
#include "../Includes/sockio_tcp.h"
#include "../Includes/openssl_stuff.h"
#include "../Includes/fileshit.h"
#include <unistd.h>
#include <limits.h>
uint8_t SERVER_SSL_initted_in_process=0;

uint8_t SSL_on_in_process=0;

uint8_t CLIENT_SSL_initted_in_process=0;

int verify_callback(int ok, X509_STORE_CTX *ctx) {
    if (!ok) {
        int err = X509_STORE_CTX_get_error(ctx);
        printf("Verify error: %s\n",
               X509_verify_cert_error_string(err));
    }
    return ok;
}

void InitializeSSL(void){
	if(SSL_on_in_process){
		if(logging){
			fprintf(logstream,"SSL já ativo! Ignorando!\n");
		}
		return;
	}
	else{
		if(logging){
			fprintf(logstream,"SSL ativo! SSL_on_in_process: 0 -> 1 !\n");
		}
		SSL_on_in_process=1;
	}
    printf("OpenSSL version: %s\n", OPENSSL_VERSION_TEXT);
    printf("OpenSSL version text func: %s\n", OpenSSL_version(OPENSSL_VERSION));
    SSL_load_error_strings();
    SSL_library_init();
    OpenSSL_add_all_algorithms();
}

void DestroySSL(void){
	if(!SSL_on_in_process){
		if(logging){
			fprintf(logstream,"SSL já desativado! Ignorando!\n");
		}
		return;
	}
	else{
		if(logging){
			fprintf(logstream,"SSL desativado! SSL_on_in_process: 1 -> 0 !\n");
		}
		SSL_on_in_process=0;
	}

    ERR_free_strings();
    EVP_cleanup();
}

void ShutdownSSL(SSL** cSSL){
	SSL_shutdown(*cSSL);
	SSL_free(*cSSL);
	*cSSL=NULL;
}
void convert_server_con_to_ssl(SSL** cSSL, int sd,int_pair times){

	(*cSSL)=SSL_new(global_ctx);
	SSL_set_fd(*cSSL, sd);
	//Here is the SSL Accept portion.  Now all reads and writes must use SSL
	int ssl_ret=0,ssl_err=0;
	struct timeval tv;
	tv.tv_sec=times[0];
	tv.tv_usec=times[1];
	while(1) {
		ssl_ret  = SSL_accept(*cSSL);

		if (ssl_ret == 1) {
			// Handshake complete
			break;
		}

		ssl_err = SSL_get_error(*cSSL, ssl_ret);

		if (ssl_err == SSL_ERROR_WANT_READ) {
			fd_set rfds;
			FD_ZERO(&rfds);
			FD_SET(sd, &rfds);
			select(sd + 1, &rfds, (fd_set*)0, (fd_set*)0, &tv);
		}
		else if (ssl_err == SSL_ERROR_WANT_WRITE) {
			fd_set wfds;
			FD_ZERO(&wfds);
			FD_SET(sd, &wfds);
			select(sd + 1, (fd_set*)0, &wfds, (fd_set*)0, &tv);
		}
		else {
			// Real failure
			if(logging){
				fprintf(logstream, "SSL handshake failed:\n");
				ERR_print_errors_fp(logstream);
			}
			ShutdownSSL(cSSL);
			break;
		}
	}
}
void convert_client_con_to_ssl(SSL** cSSL, int sd,int_pair times){

	(*cSSL)=SSL_new(global_ctx);
	SSL_set_fd(*cSSL, sd);
	//Here is the SSL Accept portion.  Now all reads and writes must use SSL
	int ssl_ret=0,ssl_err=0;
	struct timeval tv;
	tv.tv_sec=times[0];
	tv.tv_usec=times[1];
	while(1) {
		ssl_ret  = SSL_connect(*cSSL);

		if (ssl_ret == 1) {
			// Handshake complete
			break;
		}

		ssl_err = SSL_get_error(*cSSL, ssl_ret);

		if (ssl_err == SSL_ERROR_WANT_READ) {
			fd_set rfds;
			FD_ZERO(&rfds);
			FD_SET(sd, &rfds);
			select(sd + 1, &rfds, (fd_set*)0, (fd_set*)0, &tv);
		}
		else if (ssl_err == SSL_ERROR_WANT_WRITE) {
			fd_set wfds;
			FD_ZERO(&wfds);
			FD_SET(sd, &wfds);
			select(sd + 1, (fd_set*)0, &wfds, (fd_set*)0, &tv);
		}
		else {
		// Real failure
			if(logging){
				fprintf(logstream, "SSL handshake failed:\n");
				ERR_print_errors_fp(logstream);
			}
			ShutdownSSL(cSSL);
			break;
		}
		if(logging){
			fprintf(logstream,"Waiting...\n");
		}
	}
}
void init_openssl_libs_server_side(void){
	if(logging){
		if(will_use_tls){

			fprintf(logstream,"Server's SSL Ativado? inicializando Server's SSL!...\n");
		}
		else{

			fprintf(logstream,"Server's SSL não ativado. Returning...\n");
			return;
		}
	}

	if(will_use_tls){
		if(SERVER_SSL_initted_in_process){
			if(logging){
				fprintf(logstream,"SERVER SSL já ativo! Ignorando!\n");
			}
			return;
		}
		else{
			if(logging){
				fprintf(logstream,"SERVER SSL ativo! SERVER_SSL_initted_in_process: 0 -> 1 !\n");
			}
			SERVER_SSL_initted_in_process=1;
		}
		InitializeSSL();
		global_ctx = SSL_CTX_new(TLS_server_method());
		SSL_CTX_set_verify(global_ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, verify_callback);

		/* Load CA certificate to verify the server */
		if (!SSL_CTX_load_verify_locations(global_ctx, auth_cert_file_path, NULL)) {
		    ERR_print_errors_fp(stderr);
		}

		/* Load client certificate */
		if (!SSL_CTX_use_certificate_file(global_ctx, host_cert_file_path, SSL_FILETYPE_PEM)) {
		    ERR_print_errors_fp(stderr);
		}

		/* Load client private key */
		if (!SSL_CTX_use_PrivateKey_file(global_ctx, host_pkey_file_path, SSL_FILETYPE_PEM)) {
		    ERR_print_errors_fp(stderr);
		}

		/* Make sure key matches cert */
		if (!SSL_CTX_check_private_key(global_ctx)) {
		    fprintf(stderr, "Client key does not match certificate\n");
		}
	}

}

void init_openssl_libs_client_side(void){

	if(logging){
		if(will_use_tls){

			fprintf(logstream,"Client's SSL Ativado? inicializando Client's SSL!...\n");
		}
		else{

			fprintf(logstream,"Client's SSL não ativado. Returning...\n");
			return;
		}
	}

	if(will_use_tls){
		if(CLIENT_SSL_initted_in_process){
			if(logging){
				fprintf(logstream,"CLIENT SSL já ativo! Ignorando!\n");
			}
			return;
		}
		else{
			if(logging){
				fprintf(logstream,"CLIENT SSL ativo! CLIENT_SSL_initted_in_process: 0 -> 1 !\n");
			}
			CLIENT_SSL_initted_in_process=1;
		}
		InitializeSSL();
		global_ctx = SSL_CTX_new(TLS_client_method());
		SSL_CTX_set_verify(global_ctx, SSL_VERIFY_PEER, verify_callback);

		char cwd[PATH_MAX];
		getcwd(cwd, sizeof(cwd));
		printf("Current working dir: %s\n", cwd);

		/* Load CA certificate to verify the server */
		if (!SSL_CTX_load_verify_locations(global_ctx, auth_cert_file_path, NULL)) {
		    ERR_print_errors_fp(stderr);
		}

		/* Load client certificate */
		if (!SSL_CTX_use_certificate_file(global_ctx, host_cert_file_path, SSL_FILETYPE_PEM)) {
		    ERR_print_errors_fp(stderr);
		}

		/* Load client private key */
		if (!SSL_CTX_use_PrivateKey_file(global_ctx, host_pkey_file_path, SSL_FILETYPE_PEM)) {
		    ERR_print_errors_fp(stderr);
		}

		/* Make sure key matches cert */
		if (!SSL_CTX_check_private_key(global_ctx)) {
		    fprintf(stderr, "Client key does not match certificate\n");
		}
	}


}

void end_openssl_libs_client_side(void){



	if(logging){

		fprintf(logstream,"Closing client's global ssl context?\n");
	}
	if(global_ctx&&will_use_tls){
		if(!CLIENT_SSL_initted_in_process){
			if(logging){	
				fprintf(logstream,"CLIENT SSL já desativado! Ignorando!\n");
			}
			return;
		}
		else{
			if(logging){	
				fprintf(logstream,"CLIENT SSL desativado! CLIENT_SSL_initted_in_process: 1 -> 0 !\n");
			}
			CLIENT_SSL_initted_in_process=0;
		}
		if(logging){

			fprintf(logstream,"Yes!\n");
		}
		DestroySSL();
		SSL_CTX_free(global_ctx);
		global_ctx=NULL;
		if(logging){

			fprintf(logstream,"Client's global ssl context closed!!\n");
		}
	}
	else if(logging){

		fprintf(logstream,"Nooooo... (Client's global ctx is null so there is nothing to close)\n");
	}
}

void end_openssl_libs_server_side(void){

	if(logging){

		fprintf(logstream,"Closing server's global ssl context?\n");
	}
	if(global_ctx&&will_use_tls){
		if(!SERVER_SSL_initted_in_process){
			if(logging){	
				fprintf(logstream,"SERVER SSL já desativado! Ignorando!\n");
			}
			return;
		}
		else{
			if(logging){	
				fprintf(logstream,"SERVER SSL desativado! SERVER_SSL_initted_in_process: 1 -> 0 !\n");
			}
			SERVER_SSL_initted_in_process=0;
		}
		if(logging){

			fprintf(logstream,"Yes!\n");
		}
		DestroySSL();
		SSL_CTX_free(global_ctx);
		global_ctx=NULL;
		if(logging){

			fprintf(logstream,"Server's global ssl context closed!!\n");
		}
	}
	else if(logging){

		fprintf(logstream,"Nooooo... (Server's global ctx is null so there is nothing to close)\n");
	}


}
