#include "../Includes/preprocessor.h"
#include "../Includes/fileshit.h"

FILE* logstream=stderr;

u_int64_t logging=1;

char curr_dir[PATHSIZE]={0};

socklen_t socklenvar[2]= {sizeof(struct sockaddr),sizeof(struct sockaddr_in)};
