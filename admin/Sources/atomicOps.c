#include "../Includes/preprocessor.h"

/*
0- add a value
1- remove a value
2- get element from index(value has to be a pointer to an index on the list)
3- destroy the list
4- get a pointer to a (non copied) chunk of memory where the size of memory where the size of the list is stored
5- get a pointer to a (non copied) chunk of memory where the the list actually is stored

if the list is NULL or the value, this wont do anything


it allways returns the final value of the variable by default
(whether or not one of the specific options above is chosen (returns original value))
*/

void* acessListMtx(pthread_mutex_t * mtx,DListW* list,void* value,u_int64_t index,int op){
	
	void* result=NULL;
	if(list){
	pthread_mutex_lock(mtx);
	switch(op){
	break;
	case 0:
	if(value){
	addElemToListComp(list,value);
	}
	break;
	case 1:
	if(list->currSize&&value){
	remElemFromListComp(list,value);
	}
	break;
	case 2:
	if(list->currSize&&index>=0&&index<list->currSize){
	result=getElemAtIndex(list,index);
	}
	break;
	case 3:
	destroyDList(list);
	break;
	case 4:
	result=&list->currSize;
	break;
	case 5:
	result=(void*)list;
	break;
	default:
	break;
	}
	pthread_mutex_unlock(mtx);
	}
	return result;


}
/*
0- push a value
1- pop a value
2- destroy the log stack
3- get a pointer to a (non copied) chunk of memory where the size of memory where the size of the list is stored
4- get a pointer to a (non copied) chunk of memory where the the stack actually is stored

if the stack is NULL or the value, this wont do anything


it allways returns the final value of the variable by default
(whether or not one of the specific options above is chosen (returns original value))
*/

void* acessStackMtx(pthread_mutex_t * mtx,stackList* stck,void* value,int op){
	
	void* result=NULL;
	if(stck){
	pthread_mutex_lock(mtx);
	switch(op){
	break;
	case 0:
	if(value){
	pushDLStack(stck,value);
	}
	break;
	case 1:
	if(stck->list->currSize){
	result=popDLStack(stck);
	}
	break;
	case 2:
	destroyDLStack(stck);
	break;
	case 3:
	result=&stck->list->currSize;
	break;
	case 4:
	result=(void*)stck;
	break;
	default:
	break;
	}
	pthread_mutex_unlock(mtx);
	}
	return result;


}

/*
0- change its value
1- increment it
2- decrement it
3- increment it (by a value)


it allways returns the final value of the variable by default
(whether or not one of the specific options above is chosen (returns original value))
*/
u_int64_t acessVarMtx(pthread_mutex_t * mtx,u_int64_t* var,u_int64_t value,int op){
	
	pthread_mutex_lock(mtx);
	u_int64_t result=0;
	switch(op){
	break;
	case 0:
	(*var)=value;
	break;
	case 1:
	(*var)++;
	break;
	case 2:
	(*var)--;
	break;
	case 3:
	(*var)+=value;
	break;
	default:
	break;
	}
	result=(*var);
	pthread_mutex_unlock(mtx);
	return result;



}

void printScrMtx(pthread_mutex_t * mtx,char* str,int y,int x){

	pthread_mutex_lock(mtx);
	mvprintw(y,x,"%s",str);
	pthread_mutex_unlock(mtx);
	

}



void* acessItMtx(pthread_mutex_t * mtx,dliterator* it){

void* result=NULL;
pthread_mutex_lock(mtx);

		result= nextIt(it);
pthread_mutex_unlock(mtx);
	return result;

}

