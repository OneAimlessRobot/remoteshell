#include "../Includes/preprocessor.h"
#include "../Includes/fileshit.h"

FILE* logstream=NULL;

u_int64_t logging=0;

char curr_dir[PATHSIZE]={0};

socklen_t socklenvar[2]= {sizeof(struct sockaddr),sizeof(struct sockaddr_in)};
