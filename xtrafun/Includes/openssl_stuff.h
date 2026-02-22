#ifndef OPENSSL_STUFF_H
#define OPENSSL_STUFF_H

#define SSL_TIMEOUT_WAIT_SEC 1
#define SSL_TIMEOUT_WAIT_USEC 1000

extern uint8_t SERVER_SSL_initted_in_process;

extern uint8_t SSL_on_in_process;

extern uint8_t CLIENT_SSL_initted_in_process;

void InitializeSSL(void);

void DestroySSL(void);

void ShutdownSSL(SSL** cSSL);

void convert_server_con_to_ssl(SSL** cSSL, int sd,int_pair times);
void convert_client_con_to_ssl(SSL** cSSL, int sd,int_pair times);

void init_openssl_libs_client_side(void);

void init_openssl_libs_server_side(void);

void end_openssl_libs_client_side(void);

void end_openssl_libs_server_side(void);

#endif
