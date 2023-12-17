#ifndef DOUBLELIST_H
#define DOUBLELIST_H
typedef struct DListW{

DList head,trail;
u_int64_t currSize,elemSize;

}DListW;


DListW* initDList(u_int64_t elemSize);

void addElemToList(DListW* list,void* data,u_int64_t index);

void addElemToListComp(DListW* list,void* data);

void remElemFromListComp(DListW* list,void* data);

void remElemFromList(DListW* list,u_int64_t index);


void* findElemListComp(DListW* list,void* data);



void* getElemAtIndex(DListW*list,u_int64_t index);


DListW* makeIntList(int arr[],int size);

void printIntList(DListW* list);

void printIntListBack(DListW* list);

void destroyDList(DListW* list);
#endif
