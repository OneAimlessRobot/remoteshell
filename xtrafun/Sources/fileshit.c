#include "../Includes/preprocessor.h"
#include "../Includes/fileshit.h"

FILE* logstream=NULL;

u_int64_t logging=1;

char curr_dir[PATHSIZE]={0};

char auth_cert_file_path[PATHSIZE]={0};

char host_cert_file_path[PATHSIZE]={0};

char host_pkey_file_path[PATHSIZE]={0};

socklen_t socklenvar[2]= {sizeof(struct sockaddr),sizeof(struct sockaddr_in)};

int_pair srv_con_pair = {SERVER_TIMEOUT_CON_SEC,SERVER_TIMEOUT_CON_USEC};


int_pair srv_data_pair = {SERVER_TIMEOUT_DATA_SEC,SERVER_TIMEOUT_DATA_USEC};

int_pair clnt_con_pair = {CLIENT_TIMEOUT_CON_SEC,CLIENT_TIMEOUT_CON_USEC};


int_pair clnt_data_pair = {CLIENT_TIMEOUT_DATA_SEC,CLIENT_TIMEOUT_DATA_USEC};


int32_t client_socket=-1;

int32_t server_socket=-1;

u_int8_t will_use_tls;

SSL* client_ssl=NULL;

SSL* server_ssl=NULL;

SSL_CTX* global_ctx=NULL;


atomic_int all_alive=1;
atomic_int out_alive=0;
atomic_int cmd_alive=0;
pthread_mutex_t varMtx= PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t exitCond= PTHREAD_COND_INITIALIZER;
pthread_mutex_t exitMtx= PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cmdCond= PTHREAD_COND_INITIALIZER;
pthread_mutex_t cmdMtx= PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t outCond= PTHREAD_COND_INITIALIZER;
pthread_mutex_t outMtx= PTHREAD_MUTEX_INITIALIZER;

pthread_t commandPrompt;
pthread_t outputPrinter;
pthread_t outputWritter;
char outbuff[DEF_DATASIZE*10]={0};
char raw_line[DEF_DATASIZE]={0};
struct sockaddr_in server_address;
