#ifndef SERVERCMDS_H
#define SERVERCMDS_H

extern cmdstruct servercmds[];


void setPingSize(int64_t argc,int* toExit, void** argv);

void setDataSize(int64_t argc,int* toExit, void** argv);

void sair(int64_t argc,int* toExit, void** argv);

void showservercmds(int64_t argc,int* toExit, void** argv);



#endif
