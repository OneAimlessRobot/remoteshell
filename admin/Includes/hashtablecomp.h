#ifndef HASHTABLECOMP_H
#define HASHTABLECOMP_H
#define GROWFACTOR 2
#define HTCSTARTSIZE 3
#define MIN_LOAD_FACTOR 0.75
typedef struct hashtablecomp{

DListWComp **spine;
u_int64_t currSize,currSpineSize,sizeOfElem;
comparator*comp;
hasher*hfunc;
}hashtablecomp;


hashtablecomp* initHashTableComp(u_int64_t sizeofElem,comparator*comp,hasher*hfunc);

void addToHTComp(hashtablecomp** table,void* mem);

void removeFromHTComp(hashtablecomp* table,void * mem);

void* getHTElemComp(hashtablecomp* table,void* mem);

void destroyHashTableComp(hashtablecomp* table);

void printHashTableComp(hashtablecomp* table);




#endif

