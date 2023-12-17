#ifndef HASHER_H
#define HASHER_H


typedef struct hasher{

int64_t (*func)(char*);

}hasher;

#endif
