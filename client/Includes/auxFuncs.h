#ifndef AUXFUNCS_H
#define AUXFUNCS_H



char* randStr(int size);
char** randStrArr(int sizeOfStrs,int size);

void freeStrArr(char** arr,int size);
int* getRandIntArr(int min,int max,int size);

double genRanddouble(double min, double max);
int genRandInt(int min, int max);
void swap(void** var1, void** var2);
int64_t min(int64_t arg1, int64_t arg2);
#endif
