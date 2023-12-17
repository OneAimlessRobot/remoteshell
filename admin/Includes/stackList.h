
#ifndef STACKLIST_H
#define STACKLIST_H
typedef struct stackList{


DListW *list;


}stackList;

stackList* initDLStack(u_int64_t elemSize);

void destroyDLStack(stackList* stck);

void* popDLStack(stackList *stck);


void pushDLStack(stackList*stck, void* mem);


u_int64_t sizeDLStack(stackList*stck);

int isEmptyDLStack(stackList*stck);


#endif
