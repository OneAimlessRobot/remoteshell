#include "../Includes/preprocessor.h"
#include "../Includes/comparator.h"
#include "../Includes/nodes.h"
#include "../Includes/doublelistcomp.h"
#include "../Includes/auxFuncs.h"


static DNode* initNakedDNode(DListWComp*list,void*initmem){

DNode* newNode= malloc(sizeof(DNode));

//newNode->mem=malloc(newNode->memSize);
//memcpy(newNode->mem,initmem, memSize);

newNode->mem= malloc(list->elemSize);
memcpy(newNode->mem,initmem,list->elemSize);
newNode->prev=NULL;
newNode->next=NULL;

return newNode;


}

DListWComp* initDListComp(u_int64_t elemSize,comparator*comp){

DListWComp* result= malloc(sizeof(DListWComp));

result->head=result->trail=NULL;
result->currSize=0;
result->elemSize=elemSize;
result->comp=comp;
return result;

}
//pre: node->mem ==NULL and node->mem is free
DList destroyHEADDNode(DNode* node){
if(!node){
return node;

}

free(node->mem);
node->mem=NULL;
if(node->next){

node=node->next;

node->prev->next=NULL;
free(node->prev);

node->prev=NULL;

}
else{
free(node);
node=NULL;
}

return node;
}
static DList clearHEADDNode(DNode* node){
if(!node){
return node;

}
free(node->mem);
node->mem=NULL;

return node;
}


DListWComp* makeIntListComp(int arr[],int size,comparator*comp){
DListWComp* list= malloc(sizeof(DListWComp));
list->elemSize=sizeof(int);
DList dummyHead=initNakedDNode(list,arr+0);
list->head=dummyHead;
for(int i=1;i<size;i++){

DList dummy=initNakedDNode(list,arr+i);
dummy->prev=dummyHead;
dummy->next=NULL;
dummyHead->next= dummy;
dummyHead=dummy;
list->trail=dummy;
}
list->comp=comp;
list->currSize=size;
return list;

}

static DList findNode(DListWComp*list,void* data){
	
	if(!list->head){

		return NULL;
	}
	DList curr= list->head;

	while(curr&&list->comp->func(data,curr->mem)<0){

	curr=curr->next;

	}
	if(!curr){
		return NULL;
	}
	if(list->comp->func(data,curr->mem)>=0){
		
		return curr;

	}
	return NULL;


}


void* findElemListCompComp(DListWComp* list,void* data){

	
                 if(list->currSize){
                          DList node=findNode(list,data);
                         if(node){
                         return node->mem;
                         }
                         else{
                         return NULL;
                         }
                 }
		return NULL;


}

static void addBeforeNode(DList addedNode, DList nodeAfter){

                       addedNode->prev=nodeAfter->prev;
                        addedNode->next=nodeAfter;
                        nodeAfter->prev->next=addedNode;
                        nodeAfter->prev=addedNode;


}

void printIntListComp(DListWComp* list){

for(DList head=list->head;head;head=head->next){

	printf("%i\n",*(int*)(head->mem));

}


}
void printIntListBackComp(DListWComp* list){


DList head=list->head;
for(;head->next;head=head->next){

}

do{

	printf("%i\n",*(int*)(head->mem));
	head=head->prev;
}
while(head);


}

void destroyDListComp(DListWComp* list){
if(list==NULL){

	return;
}

DList head=list->head;
do{
head=destroyHEADDNode(head);


}while(head);
free(list);
list=NULL;

}
static DList getNodeFromIndex(DListWComp* list, u_int64_t index){


	        DList j = NULL;
                        u_int64_t i;
                        if(index>list->currSize/2) {

                                i=list->currSize-1;
                                j=list->trail;
                                for(;i>index;i--,j=j->prev);

                        }
                        else if(index<=(list->currSize)/2) {

                                i=0;
                                j=list->head;
                                for(;i<index;i++,j=j->next);

                        }
        	return j;



}
void* getElemAtIndexComp(DListWComp*list,u_int64_t index){
	
		if(list->currSize){
                         return getNodeFromIndex(list,index)->mem;
		}
		return NULL;

}

static void addMiddleOfList(DListWComp* list,DList node,u_int64_t index){
		
		if(node) {
                        DList j = NULL;
                        u_int64_t i;
                        if(index>list->currSize/2) {

                                i=list->currSize-1;
                                j=list->trail;
                                for(;i>index;i--,j=j->prev);

                        }
                        else if(index<=(list->currSize)/2) {

                                i=0;
                                j=list->head;
                                for(;i<index;i++,j=j->next);

                        }
                        node->prev=j->prev;
                        node->next=j;
                        j->prev->next=node;
                        j->prev=node;

                }


}
static void addEndOfList(DListWComp* list,DList node){
		
		if(node) {


                        node->prev=list->trail;
                        list->trail->next=node;
                        list->trail=node;

		}


}
static void addStartOfList(DListWComp* list,DList node){

	if(node){
                        node->next=list->head;
                        list->head->prev=node;
                        list->head=node;

	}


}

void addElemToList2(DListWComp* list,void* data,u_int64_t index){

DList node= initNakedDNode(list,data);

                if(!list->head) {

                        list->head=list->trail=node;
                        list->currSize++;
                        return;

                }
                else if(index <=0) {

                        addStartOfList(list,node);
                }
                else if(index >=list->currSize-1) {

                        addEndOfList(list,node);
                }
                else {

                        addMiddleOfList(list,node,index);

                }
		list->currSize++;



}
static void remFirstNodeFromList(DListWComp*list){

	DList tmp=list->head->next;
	tmp->prev=NULL;
	list->head->next=NULL;
	list->head->prev=NULL;
	destroyHEADDNode(list->head);
	list->head=tmp;
	

}
static void remLastNodeFromList(DListWComp*list){

	DList tmp=list->trail->prev;
	tmp->next=NULL;
	list->trail->prev=NULL;
	list->trail->next=NULL;
	destroyHEADDNode(list->trail);
	list->trail=tmp;

}
static void remMiddleNodeFromListComp(DListWComp*list,void* data){


	DList node=findNode(list, data);
	if(node){
		DList next=node->next;
		DList prev= node->prev;
		prev->next=next;
		next->prev=prev;
		node->next=NULL;
		node->prev=NULL;
		destroyHEADDNode(node);
	}
		
}

static void remMiddleNodeFromList(DListWComp*list,u_int64_t index){

	DList node=getNodeFromIndex(list, index);
	if(node){
		DList next=node->next;
		DList prev= node->prev;
		prev->next=next;
		next->prev=prev;
		node->next=NULL;
		node->prev=NULL;
		destroyHEADDNode(node);
	}
		

}
void remElemFromListComp2(DListWComp* list,void* data){


                if(!list->head) {

                        return;

                }
		DList removedNode= findNode(list,data);
                if(!removedNode){

			return;

		}
		if(list->currSize==1){
		if(!list->comp->func(list->head->mem,removedNode->mem)){
		list->head=destroyHEADDNode(list->head);
		list->currSize--;
		return;
		}
		}
		
		if(!removedNode||removedNode==list->trail) {

			remLastNodeFromList(list);
                }
                else if(removedNode==list->head) {

                	remFirstNodeFromList(list);
                }
                else {

			remMiddleNodeFromListComp(list,data);

                }
		list->currSize--;



}
void remElemFromList2(DListWComp* list,u_int64_t index){
	if(list->currSize){
	if(list->currSize==1){

		list->head=destroyHEADDNode(list->head);
	}
	else{
	if(index<=0){

		remFirstNodeFromList(list);



	}
	else if(index>=list->currSize-1){


		
		remLastNodeFromList(list);


	}
	else{

		remMiddleNodeFromList(list,index);

	}
	}

	list->currSize--;
	}



}

void addElemToListComp2(DListWComp* list,void* data){


DList node= initNakedDNode(list,data);

                if(!list->head) {

                        list->head=list->trail=node;
                        list->currSize++;
                        return;

                }

		DList nodeAfter=findNode(list,data);
                
		if(!nodeAfter){
			
			addEndOfList(list,node);
                

		}
		else if(nodeAfter==list->head) {

                        addStartOfList(list,node);
                }
                else{
			addBeforeNode(node,nodeAfter);

                }
		list->currSize++;




}
