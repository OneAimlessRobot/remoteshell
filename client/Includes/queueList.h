#ifndef QUEUELIST_H
#define QUEUELIST_H


typedef struct queueList{


DListW* list;


}queueList;

queueList* initDLQueue(u_int64_t elemSize);

void destroyDLQueue(queueList* stck);

void* dequeueDLQueue(queueList *stck);


void enqueueDLQueue(queueList*stck, void* mem);


u_int64_t sizeDLQueue(queueList*stck);

int isEmptyDLQueue(queueList*stck);


#endif
