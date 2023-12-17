#include "../Includes/preprocessor.h"
#include "../Includes/doublelistcomp.h"
#include "../Includes/comparator.h"
#include "../Includes/hasher.h"
#include "../Includes/hashtablecomp.h"

static u_int64_t getTablePosForElem(hashtablecomp * table, void* mem){
	
	return table->hfunc->func((char*)mem)%table->currSpineSize;


}

static  double getLoadFactor(hashtablecomp* table){

	return (double)table->currSpineSize/(double)table->currSize;


}

static int isFull(hashtablecomp* table){

	return getLoadFactor(table)<MIN_LOAD_FACTOR;

}

hashtablecomp* initHashTableComp(u_int64_t sizeOfElem,comparator*comp,hasher*hfunc){
	
	hashtablecomp* result= malloc(sizeof(hashtablecomp));
	result->spine= malloc(sizeof(DListWComp*)*HTCSTARTSIZE);
	for(u_int64_t i=0;i<HTCSTARTSIZE;i++){
		
		result->spine[i]= initDListComp(sizeOfElem,comp);


	}
	result->sizeOfElem=sizeOfElem;
	result->currSize=0;
	result->comp=comp;
	result->hfunc=hfunc;
	result->currSpineSize=HTCSTARTSIZE;
	
	return result;


}

static hashtablecomp* initHashTableInitSizeComp(u_int64_t sizeOfElem,u_int64_t size,comparator*comp,hasher*hfunc){
	
	hashtablecomp* result= malloc(sizeof(hashtablecomp));
	result->spine= malloc(sizeof(DListWComp*)*size);
	for(u_int64_t i=0;i<size;i++){
		
		result->spine[i]= initDListComp(sizeOfElem,comp);


	}
	result->sizeOfElem=sizeOfElem;
	result->currSize=0;
	result->comp=comp;
	result->hfunc=hfunc;
	result->currSpineSize=size;

	return result;


}

static hashtablecomp* rehash(hashtablecomp* table){
	u_int64_t newTableSize= (table)->currSpineSize*GROWFACTOR;
	hashtablecomp* newTable= initHashTableInitSizeComp((table)->sizeOfElem,newTableSize,table->comp,table->hfunc);	
	for(u_int64_t i=0;i<(table)->currSpineSize;i++){

		if((table)->spine[i]->head){
			DList node=(table)->spine[i]->head;
			for(u_int64_t j=0;j<(table)->spine[i]->currSize;j++){
			
			 addToHTComp(&newTable,node->mem);

			node=node->next;

			}

		}

	}
	destroyHashTableComp(table);
	return newTable;
}

void printHashTableComp(hashtablecomp* table){

for(u_int64_t i=0;i<table->currSpineSize;i++){

		if(table->spine[i]->head){
			DList node=table->spine[i]->head;
			for(u_int64_t j=0;j<table->spine[i]->currSize;j++){
			
			printf("%s ",(char*)(node->mem));
			
			node=node->next;

			}
			printf("\n");
		}

	}
}

//pre: mem is sizeOfElem in bytes in size
void addToHTComp(hashtablecomp** table,void* mem){

	if(isFull(*table)){
		

		*table=rehash(*table);
	}
	
	u_int64_t pos=getTablePosForElem(*table,mem);
	DListWComp* list= (*table)->spine[pos];
	addElemToListComp2(list,mem);
	(*table)->currSize++;



}
void removeFromHTComp(hashtablecomp* table,void * mem){


	if(!table->currSize){


		return;
	}
	u_int64_t pos=getTablePosForElem(table,mem);
	DListWComp* list= table->spine[pos];
	remElemFromListComp2(list,mem);
	table->currSize--;




}
void destroyHashTableComp(hashtablecomp* table){

	for(u_int64_t i=0;i<table->currSpineSize;i++){
	
		destroyDListComp(table->spine[i]);
		table->spine[i]=NULL;
	}
	free(table->spine);
	free(table);
	table=NULL;


}

void* getHTElemComp(hashtablecomp* table,void* mem){

u_int64_t pos=getTablePosForElem(table,mem);
	
	return findElemListCompComp(table->spine[pos],mem);


}
