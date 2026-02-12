#ifndef FILESHIT_H
#define FILESHIT_H

#define MAXCLIENTS 10000

extern FILE* logstream;

extern u_int64_t logging;

extern char curr_dir[PATHSIZE];

extern socklen_t socklenvar[2];

extern int_pair srv_con_pair,
		srv_data_pair,
		clnt_con_pair,
		clnt_data_pair;


extern atomic_int all_alive,
	out_alive,
	cmd_alive;
extern pthread_mutex_t varMtx,
		exitMtx,
		cmdMtx,
		outMtx;
extern pthread_cond_t exitCond,
			cmdCond,
			outCond;


extern int32_t server_socket,
		client_socket;


extern pthread_t commandPrompt,
                outputWritter,
                outputPrinter;

extern char outbuff[DEF_DATASIZE*10],
        raw_line[DEF_DATASIZE];

extern struct sockaddr_in server_address;


#endif
