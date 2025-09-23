#ifndef AVL_H
#define AVL_H
#include "comparator.h"
#include "stackList.h"
enum AVLNodeType {E,L,R};
typedef enum AVLNodeType AVLNodeType;


typedef struct AVLNode{

void* mem;
struct AVLNode* left,*right;
AVLNodeType type;
}AVLNode;

typedef struct pathStepAVL{

AVLNode* parent;
int isLeft;


}pathStepAVL;


typedef struct AVLTree{

u_int64_t currSize;
AVLNode* root;
comparator* comp;
stackList* path;

}AVLTree;

AVLNode* initAVLNode(void*initmem,AVLNode* left, AVLNode* right,AVLNodeType type);

AVLNode* initNakedAVLNode(void*initmem,AVLNodeType type);

AVLree* initAVLree(comparator*comp);


void addToAVLree(AVLTree* tree,void* elem);

void removeFromAVLree(AVLTree* tree,void* elem);

void destroyAVLree(AVLTree* tree);

void* findInAVLree(AVLTree* tree, void* data);

void* getAVLNodeMem(AVLNode* node);

void setAVLNodeMem(AVLNode* node, void* mem);

void setLeft(AVLNode* node, AVLNode**newLeft);

void setRight(AVLNode* node, AVLNode**newRight);

int isLeaf(AVLNode* node);

void destroyAVLNode(AVLNode* node);


void printIntAVLreeBreadth(AVLTree*tree);

void printIntAVLreeDepth(AVLTree*tree);

void printIntAVLreeInfix(AVLTree*tree);

void printAVLreePretty(AVLTree*tree);

AVLTree* makeIntAVLTree(int arr[],int arrSize);

#endif
