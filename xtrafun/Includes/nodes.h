#ifndef NODES_H
#define NODES_H
typedef struct DNode{

void* mem;
struct DNode* prev,*next;



}DNode,*DList;

typedef struct BSTNode{

void* mem;
struct BSTNode* left,*right;

}BSTNode;

typedef struct pathStep{

BSTNode* parent;
int isLeft;


}pathStep;



#endif
