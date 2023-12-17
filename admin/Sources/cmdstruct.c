#include "../Includes/preprocessor.h"
#include "../Includes/comparator.h"
#include "../Includes/hasher.h"
#include "../Includes/cmdstruct.h"



comparator cmdcomparator = {compareCmds};
hasher cmdhasher = {hashCmd};


int compareCmds(void* cmd1,void*cmd2){

	cmdstruct* var1= (cmdstruct*)cmd1;
	cmdstruct* var2= (cmdstruct*)cmd2;

	return strcmp(var1->cmdname,var2->cmdname);
	

}

int64_t hashCmd(char* cmd){

 char* mem= ((cmdstruct*)(cmd))->cmdname;

u_int64_t hash= 5381;

	u_int64_t count=0;

for(;count<CMDMAXLENGTH+1;count++){

hash= ((hash << 5) + hash) +mem[count];
}
return hash;


}

cmdstruct* spawnCmdStruct(char* name, void(*cmd)(int64_t,int*, void**),char* helpdesc){

cmdstruct* result= malloc(sizeof(cmdstruct));

memset(result->cmdname,0,CMDMAXLENGTH+1);
memcpy(result->cmdname,name,min(strlen(name),CMDMAXLENGTH));


memset(result->helpdesc,0,CMDDESCLENGTH+1);
memcpy(result->helpdesc,helpdesc,min(strlen(helpdesc),CMDDESCLENGTH));
result->cmd=cmd;

return result;


}

void destroyCmdStruct(cmdstruct* cmd){

	free(cmd);

}
