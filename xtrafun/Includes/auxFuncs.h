#ifndef AUXFUNCS_H
#define AUXFUNCS_H


int64_t max(int64_t arg1, int64_t arg2);

int64_t clamp(int64_t value,int64_t arg1, int64_t arg2);

char* randStr(int size);
char** randStrArr(int sizeOfStrs,int size);

void freeStrArr(char** arr,int size);
int* getRandIntArr(int min,int max,int size);

double genRanddouble(double min, double max);
int genRandInt(int min, int max);
void swap(void** var1, void** var2);
int64_t min(int64_t arg1, int64_t arg2);

void print_addr_aux(char* prompt,struct sockaddr_in* addr);
void print_sock_addr(int socket);
void init_addr(struct sockaddr_in* addr, char* hostname_str,uint16_t port);
void destroy_win(WINDOW *local_win);
WINDOW *create_newwin(int height, int width, int starty, int startx);

extern socklen_t socklenvar[2];
#endif
