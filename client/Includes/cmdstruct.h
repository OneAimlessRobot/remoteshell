#ifndef CMDSTRUCT_H
#define CMDSTRUCT_H
#define CMDMAXLENGTH 127
#define CMDDESCLENGTH 1024
typedef struct cmdstruct{

char cmdname[CMDMAXLENGTH+1];
void (*cmd)(int64_t,int*,void**);
char helpdesc[CMDDESCLENGTH+1];

}cmdstruct;

int compareCmds(void* cmd1,void*cmd2);

int64_t hashCmd(char* cmd);

cmdstruct* spawnCmdStruct(char* name, void(*cmd)(int64_t,int*, void**),char* helpdesc);

void destroyCmdStruct(cmdstruct* cmd);

extern hasher cmdhasher;
extern comparator cmdcomparator;

#endif
