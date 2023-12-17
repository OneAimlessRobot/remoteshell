
#include "../Includes/preprocessor.h"
#include "../Includes/doublelist.h"

#include "../Includes/stackList.h"

stackList* initDLStack(u_int64_t elemSize){


stackList* stck= malloc(sizeof(stackList));

stck->list=initDList(elemSize);

return stck;


}

void destroyDLStack(stackList* stck){

destroyDList(stck->list);
free(stck);
stck=NULL;

}


void* popDLStack(stackList *stck){
if(stck){
if(!isEmptyDLStack(stck)){
void* mem=malloc(stck->list->elemSize);
memcpy(mem,stck->list->head->mem,stck->list->elemSize);
remElemFromList(stck->list,0);
return mem;
}

}
return NULL;


}
void pushDLStack(stackList*stck, void* mem){

addElemToList(stck->list,mem,0);



}

u_int64_t sizeDLStack(stackList*stck){

	return stck->list->currSize;

}

int isEmptyDLStack(stackList*stck){

return !stck->list->head;

}
