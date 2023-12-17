#ifndef DLITERATOR_H
#define DLITERATOR_H


typedef struct dliterator{

DListW* list;
DList nextNode;

}dliterator;


void* nextIt(dliterator* it);
dliterator* initIt(DListW* list);
int hasNextIt(dliterator*it);

#endif
