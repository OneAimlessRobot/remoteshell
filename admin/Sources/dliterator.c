#include "../Includes/preprocessor.h"
#include "../Includes/nodes.h"
#include "../Includes/comparator.h"
#include "../Includes/doublelist.h"
#include "../Includes/dliterator.h"



void* nextIt(dliterator* it){


	void* result=it->nextNode->mem;
	it->nextNode=it->nextNode->next;
	return result;

}
dliterator* initIt(DListW* list){
	
	dliterator* result= malloc(sizeof(dliterator));
	result->list= list;
	result->nextNode=list->head;

	return result;


}
int hasNextIt(dliterator*it){


	return it->nextNode!=NULL;

}
