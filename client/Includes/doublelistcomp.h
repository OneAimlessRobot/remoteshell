#ifndef DOUBLELISTCOMP_H
#define DOUBLELISTCOMP_H

typedef struct DListWComp{

DList head,trail;
u_int64_t currSize,elemSize;
comparator*comp;
}DListWComp;


DListWComp* initDListComp(u_int64_t elemSize,comparator*comp);

void addElemToList2(DListWComp* list,void* data,u_int64_t index);

void addElemToListComp2(DListWComp* list,void* data);

void remElemFromListComp2(DListWComp* list,void* data);

void remElemFromList2(DListWComp* list,u_int64_t index);


void* findElemListCompComp(DListWComp* list,void* data);


void* getElemAtIndexComp(DListWComp*list,u_int64_t index);

DListWComp* makeIntListComp(int arr[],int size,comparator*comp);

void printIntListComp(DListWComp* list);

void printIntListBackComp(DListWComp* list);

void destroyDListComp(DListWComp* list);
#endif
