
#include "../Includes/preprocessor.h"
#include "../Includes/doublelist.h"

#include "../Includes/queueList.h"


queueList* initDLQueue(u_int64_t elemSize){


queueList* q= malloc(sizeof(queueList));

q->list=initDList(elemSize);

return q;


}

void destroyDLQueue(queueList* q){

destroyDList(q->list);
free(q);
q=NULL;

}


void* dequeueDLQueue(queueList *q){
if(q){
if(!isEmptyDLQueue(q)){
void* mem=malloc(q->list->elemSize);
memcpy(mem,q->list->head->mem,q->list->elemSize);
remElemFromList(q->list,0);
return mem;
}

}
return NULL;


}
void enqueueDLQueue(queueList*q, void* mem){

addElemToList(q->list,mem,q->list->currSize);



}

u_int64_t sizeDLQueue(queueList*q){

	return q->list->currSize;

}

int isEmptyDLQueue(queueList*q){

return !q->list->head;

}
