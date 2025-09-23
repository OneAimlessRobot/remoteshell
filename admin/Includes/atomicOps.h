#ifndef ATOMICOPS_H
#define ATOMICOPS_H
void* acessListMtx(pthread_mutex_t * mtx,DListW* list,void* value,u_int64_t index,int op);

void* acessItMtx(pthread_mutex_t * mtx,dliterator* it);

void* acessStackMtx(pthread_mutex_t * mtx,stackList* stck,void* value,int op);

u_int64_t acessVarMtx(pthread_mutex_t * mtx,u_int64_t* var,u_int64_t value,int op);

void printScrMtx(pthread_mutex_t * mtx,char* str,int y,int x);


#endif
